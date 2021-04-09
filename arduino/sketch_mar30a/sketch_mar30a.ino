#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#ifdef DEBUG
#define digitalWrite(x, y) (1)
#define pinMode(x, y) (1)
#define digitalRead(x) (1)
#endif

void start_PWM();
void stop_PWM();
//////////////////////////////////////////////////
////////////////// Config parameters /////////////
//////////////////////////////////////////////////

namespace HW {
// Harware definitions
constexpr uint8_t MODE = 8;
constexpr uint8_t RESET = 4;
constexpr uint8_t VDD_SWICH = 5;
constexpr uint8_t RX_1 = 3;
constexpr uint8_t TX_1 = 9;
constexpr uint8_t BLANK = 12;
constexpr uint8_t CLK = 11;
constexpr uint8_t BUTTON = 6;

} // namespace HW
int incomingByte = 0;

namespace UART {
// Baudrate for uart
constexpr uint16_t PC_BAUD = 9600;
constexpr uint16_t RNS8_BAUD = 9600;
} // namespace UART

//////////////////////////////////////////////////

namespace RST {
// Reset pin
static uint8_t rst_prev = 0;
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
static uint8_t mode_prev = 0;
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
  start_PWM();
  MODE::bootloader();
  digitalWrite(HW::TX_1, HIGH);
  digitalWrite(HW::MODE, MODE::mode_prev);
  digitalWrite(HW::RESET, RST::rst_prev);
  digitalWrite(HW::VDD_SWICH, LOW);
}
void off() {
  stop_PWM();
  digitalWrite(HW::TX_1, LOW);
  digitalWrite(HW::MODE, HIGH);
  digitalWrite(HW::RESET, HIGH);
  digitalWrite(HW::VDD_SWICH, HIGH);
}
} // namespace VDD

namespace DELAY {
template <int duration_us> void us() { _delay_us(duration_us); }

template <int duration_ms> void ms() { _delay_ms(duration_ms); }

template <int duration_ns> void ns() {
  constexpr int tick_ns = (1e9 / F_CPU);
  constexpr int ticks_low = duration_ns / tick_ns;
  constexpr int ticks_high = (duration_ns + 1) / tick_ns;
  constexpr int ring = duration_ns % tick_ns;
  constexpr int ticks = (ring < tick_ns / 2) ? ticks_low : ticks_high;

  for (int i = 0; i < ticks; i++) {
    asm volatile("nop");
  }
}

} // namespace DELAY

namespace INT {
static uint16_t noOfTicks;
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

uint16_t getTimerTicks() { return TCNT1; }

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

//////////////////////////////////////////////////
//////////////////// PWM timer 2 /////////////////
//////////////////////////////////////////////////
void start_PWM() {
  // Set counter value to 0
  TCNT2 = 0x00;
  // Count to value
  OCR2A = 255;

  // Toggle on compare match (CTC mode)
  TCCR2A = 0b01 << 6 | 0b10 << 0;
  // Output to pin, 8 prescaller (1 MHZ)
  TCCR2B = 0b010 << 0;
}

void stop_PWM() {
  TCCR2A = 0x00;
  digitalWrite(HW::CLK, 0);
}

void send_bit(bool bit_value) {
  if (bit_value) {
    PORTB |= (1 << PINB1);
  } else {
    PORTB &= ~(1 << PINB1);
  }
}

void softuart_putchar(char ch) {
  constexpr uint16_t delay = (1e6 / UART::PC_BAUD) * 16;
  for (int i = 0; i < 10; i++) {
    // Wait before sending bit
    _delay_us(delay);

    // Start bit
    if (i == 0) {
      send_bit(0);
      continue;
    }

    if (i < 9) {
      // Send 8 bits of data
      bool value = ch & 0x01;
      ch >>= 1;
      send_bit(value);
      continue;
    }

    // Stop bit
    send_bit(1);
  }
}
/////////////////////////////////////////////////////////////

uint16_t send_1byte(uint8_t key) {
  uint8_t zero = 0x00;

  for (uint16_t i = 0; i < 16; i++) {
    softuart_putchar(zero);
    DELAY::ms<30>();
  }

  // send header
  const uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    softuart_putchar(el);
  }

  // send key

  // TEST 123

  softuart_putchar(key);
  softuart_putchar(zero);

  softuart_putchar(zero);
  softuart_putchar(zero);
  softuart_putchar(zero);
  softuart_putchar(zero);
  softuart_putchar(zero);

  // send query
  softuart_putchar(0x70);

  // Start interrupt on falling edge
  INT::start();

  // Delay for max timeout (4ms)
  DELAY::ms<4>();

  return INT::noOfTicks;
}

void send_256_bytes() {
  uint16_t max_value = 0;
  uint16_t max_digit = 0;
  for (uint16_t k = 0; k < 256; k++) {

    /*reset po vsakem poslanem filu*/
    DELAY::ms<100>();
    VDD::off();
    RST::off();
    DELAY::ms<1000>();
    VDD::on();
    DELAY::ms<400>();
    RST::on();

    DELAY::ms<300>();
    // DELAY::us<310>();
    // DELAY::ns<350>();

    uint16_t required_time = send_1byte(k);

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
  DELAY::ms<300>();
  RST::off();
  DELAY::ms<300>();
  RST::on();
  DELAY::ms<300>();
  VDD::off();
  DELAY::ms<400>();
}

////////////////////////////////////////////////////////////////////
/////////////////////////// ARDUINO ////////////////////////////////
////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(UART::PC_BAUD);

  pinMode(HW::MODE, OUTPUT);
  pinMode(HW::BUTTON, INPUT);
  pinMode(HW::RESET, OUTPUT);
  pinMode(HW::VDD_SWICH, OUTPUT);
  pinMode(HW::CLK, OUTPUT);
  pinMode(HW::RX_1, INPUT_PULLUP);
  pinMode(HW::TX_1, OUTPUT);
  digitalWrite(HW::TX_1, HIGH);

  start_PWM();
}

void loop() {
  Serial.println("Press button to start cracking process.");

  while (digitalRead(HW::BUTTON) == HIGH)
    ;

  Serial.println("Start of process");
  DELAY::ms<400>();

  // bootload_target();
  send_256_bytes();
  reset_target();

  Serial.println("End of process");

  while (1)
    ;
}
