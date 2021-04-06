

#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

// HW
const int MODE = 8;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 3;
const int TX_1 = 9;
const int BLANK = 12;
const int BUTTON_PIN = 11;

SoftwareSerial targetSerial(BLANK, TX_1);

constexpr uint8_t NOT(uint8_t x) { return x ^ 1; }

namespace rst {
static uint8_t rst_prev = 1;
void off() {
  digitalWrite(RESET, 1);
  rst_prev = 1;
}
void on() {
  digitalWrite(RESET, 0);
  rst_prev = 0;
}
} // namespace rst

namespace mode {
static uint8_t mode_prev = 1;
void bootloader() {
  digitalWrite(MODE, 1);
  mode_prev = 1;
}
void program() {
  digitalWrite(MODE, 0);
  mode_prev = 0;
}
} // namespace mode

namespace vdd {
void on() {
  mode::bootloader();
  digitalWrite(TX_1, 1);
  digitalWrite(MODE, mode::mode_prev);
  digitalWrite(RESET, rst::rst_prev);
  digitalWrite(VDD_SWICH, 0);
}
void off() {
  digitalWrite(TX_1, 0);
  digitalWrite(MODE, 1);
  digitalWrite(RESET, 1);
  digitalWrite(VDD_SWICH, 1);
}
} // namespace vdd

namespace DELAY {
static u32 counter_ov_cnt;
constexpr u8 CLK_DIV_1 = 1 << 0;

void _start() { TCCR2B = CLK_DIV_1; }
void _stop() { TCCR2B = 0x00; }
void _clear() {
  TCNT2 = 0x00;
  counter_ov_cnt = 0;
}
void _overflow_int() { TIMSK2 = 0x01; }

void init() {
  _stop();
  _clear();
  _overflow_int();
}

void _delay(u32 ticks) {
  counter_ov_cnt = 0;
  _clear();
  _start();
  auto cnt_ticks = []() { return counter_ov_cnt | TCNT2; };
  while (cnt_ticks() < ticks)
    ;
  _stop();
}

void ns(u32 duration_ns) {
  u32 ticks = duration_ns / F_CPU;
  _delay(ticks);
}

void us(u32 duration_us) {
  constexpr u32 us_to_ns = 1000;
  constexpr u32 max_value = static_cast<u32>(-1);
  u32 d_ns =
      (duration_us < max_value - us_to_ns) ? duration_us * us_to_ns : max_value;
  ns(d_ns);
}

void ms(u16 duration_ms) {
  constexpr u32 ms_to_us = 1000;
  constexpr u32 max_value = static_cast<u32>(-1);
  u32 duration = static_cast<u32>(duration_ms);
  u32 d_us =
      (duration < max_value - ms_to_us) ? duration * ms_to_us : max_value;
  us(d_us);
}

void s(u8 duration_s) {
  constexpr u32 s_to_ms = 1000;
  constexpr u32 max_value = static_cast<u32>(-1);
  u32 duration = static_cast<u32>(duration_s);
  u32 d_ms = (duration < max_value - s_to_ms) ? duration * s_to_ms : max_value;
  ms(d_ms);
}

} // namespace DELAY uses timer2

// timer 2 overflow
ISR(TIMER2_OVF_vect) { DELAY::counter_ov_cnt += (1 << 8); }

namespace INT {

static unsigned int noOfTicks;

void stop();

void startTimer1() {
  // Set counter value to 0
  TCNT1 = 0x00;
  // Reset TCCR1A options since Arduino likes to enable some of them.
  TCCR1A = 0x00;
  // Set timer to normal mode and set prescaler to 1.
  TCCR1B = 0x01;
}

void stopTimer1() {
  // Set timer to normal mode and set prescaler to 0.
  TCCR1B = 0x00;
}

bool isTimerActive() { return TCCR1B & 0x01; }

unsigned int getTimerTicks() {
  // Get upper and lower counter byte to ticks.
  return TCNT1;
}

void start() {
  // attachInterrupt(digitalPinToInterrupt(RX_1), INT::stop, FALLING);
  startTimer1();
  while (digitalRead(RX_1))
    ;
  stop();
}

void stop() {
  stopTimer1();
  noOfTicks = getTimerTicks();
  detachInterrupt(digitalPinToInterrupt(RX_1));
}

} // namespace INT

unsigned int send_1byte(uint8_t key) {
  uint8_t zero = 0x00;

  for (int i = 0; i < 16; i++) {
    targetSerial.write(zero);
    delay(40);
  }

  // send header
  const uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    targetSerial.write(el);
  }

  // send key

  targetSerial.write(key);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);

  // send query
  targetSerial.write(0x70);

  // Start interrupt on falling edge
  INT::start();

  // Delay for max timeout (4ms)
  delay(4);

  return INT::noOfTicks;
}

void send_256_bytes() {
  unsigned int max_value = 0;
  unsigned int max_digit = 0;
  for (unsigned int k = 0; k < 256; k++) {
    unsigned int required_time = send_1byte(k);

    // reset after sending

    delay(200);
    rst::off();
    delay(200);
    rst::on();
    delay(200);

    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" requred time: ");
    Serial.println(required_time);

    if (max_value < required_time) {
      max_value = required_time;
      max_digit = k;
    }
  }
  Serial.print("Max digit: ");
  Serial.println(max_digit);
}

void reset_target() {
  mode::program();
  delay(300);
  rst::off();
  delay(300);
  rst::on();
  delay(300);
  vdd::off();
  delay(400);
}

void bootload_target() {
  // Začetek delovanja
  vdd::on();
  delay(300);
  rst::on();
  delay(300);
  mode::program();
  delay(300);

  // We send R8C to normall working.

  rst::off();
  delay(300);
  rst::on();
  delay(500);
  vdd::off();

  delay(1000);

  // We send R8C to bootloader.
  vdd::on();
  delay(300);
  mode::bootloader();
  delay(300);
  rst::off();
  delay(300);
  rst::on();
  delay(300);
}

////////////////////////////////////////////////////////////////////
/////////////////////////// ARDUINO ////////////////////////////////
////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  targetSerial.begin(9600);
  DELAY::init();

  pinMode(MODE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(VDD_SWICH, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RX_1, INPUT_PULLUP);
}

void loop() {
  Serial.println("Press button to start cracking process.");

  while (digitalRead(BUTTON_PIN) == HIGH)
    ;

  Serial.println("Start of process");
  delay(400);

  bootload_target();
  send_256_bytes();
  reset_target();

  Serial.println("End of process");

  while (1)
    ;
}
