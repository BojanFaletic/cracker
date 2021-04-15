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
constexpr uint8_t UART_CLK = 10;

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
volatile static uint32_t noOfTicks = 0;
volatile static uint8_t overflow_cnt = 0;
void stop();

void startTimer1() {
  // Set counter value to 0
  TCNT1 = 0x00;
  // Reset TCCR1A options since Arduino likes to enable some of them.
  TCCR1A = 0x00;
  // Set timer to normal mode and set prescaler to 1.
  TCCR1B = 0x01;
  // Enable interrupt on overflow
  TIMSK1 = 0x01;
}

ISR(TIMER1_OVF_vect) { overflow_cnt++; }

void stopTimer1() {
  // Set prescaler to 0, disable interrupt
  TCCR1B = 0x00;
  TIMSK1 = 0x01;
}

uint16_t getTimerTicks() { return TCNT1; }

void start() {
  overflow_cnt = 0;
  noOfTicks = 0;
  startTimer1();
  while (digitalRead(HW::RX_1))
    ;
  stop();
}

void stop() {
  stopTimer1();
  noOfTicks = ((uint32_t)(overflow_cnt) << 16) + ((uint32_t)(getTimerTicks()));
}

} // namespace INT

//////////////////////////////////////////////////
//////////////////// PWM timer 2 /////////////////
//////////////////////////////////////////////////
void start_PWM() {
  // Set counter value to 0
  TCNT2 = 0x00;
  // Count to value
  OCR2A = 11;

  // Toggle on compare match (CTC mode)
  TCCR2A = 0b01 << 6 | 0b10 << 0;
  // Output to pin, 8 prescaller (1 MHZ)
  TCCR2B = 0b1 << 0;
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

void write_uart_clk(bool value) {
  if (value) {
    PORTB |= (1 << PINB2);
  } else {
    PORTB &= ~(1 << PINB2);
  }
}
template <uint16_t delay> void uart_one_bit(bool bit) {
  write_uart_clk(0);
  _delay_us(delay / 2);
  write_uart_clk(1);
  send_bit(bit);
  _delay_us(delay / 2);
}

template <uint16_t delay> void generic_uart(char ch) {
  // Start bit
  uart_one_bit<delay>(0);

  for (int i = 0; i < 8; i++) {
    // Send 8 bits of data
    bool value = ch & 0x01;
    ch >>= 1;
    uart_one_bit<delay>(value);
  }

  // Stop bit
  uart_one_bit<delay>(1);
}

void softuart_putchar(char ch) {
  constexpr uint16_t delay = (((1e6 / UART::PC_BAUD) * 2) * 12);
  generic_uart<delay>(ch);
}

void softuart_putchar_2(char ch) {
  constexpr uint16_t delay = (((1e6 / 115200) * 2) * 12);
  generic_uart<delay>(ch);
}

/////////////////////////////////////////////////////////////

uint32_t send_1byte(uint8_t key) {
  uint8_t zero = 0xFF;

  for (uint16_t i = 0; i < 17; i++) {
    softuart_putchar(0X00);
    DELAY::ms<200>();
  }

  softuart_putchar(0xB0);
  DELAY::ms<900>();
  softuart_putchar(0xB4);
  DELAY::ms<900>();
  softuart_putchar_2(0x50);
  softuart_putchar_2(0x70);
  DELAY::ms<400>();
  softuart_putchar_2(0x70);
  DELAY::ms<400>();
  //
  // send header
  const uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    // softuart_putchar(el);
    softuart_putchar_2(el);
  }

  softuart_putchar_2(key);
  softuart_putchar_2(zero);
  softuart_putchar_2(zero);
  softuart_putchar_2(zero);
  softuart_putchar_2(zero);
  softuart_putchar_2(zero);
  softuart_putchar_2(zero);

  softuart_putchar_2(0x70);

  // Start interrupt on falling edge
  INT::start();

  DELAY::ms<2000>();

  softuart_putchar_2(0x75);
  softuart_putchar_2(0x70);

  return INT::noOfTicks;
}

void send_256_bytes() {
  uint32_t max_value = 0;
  uint16_t max_digit = 0;
  for (int k = 0; k <= 255; k++) {

    // Normal working
    DELAY::ms<700>();
    VDD::on();
    RST::on();
    MODE::program();
    DELAY::ms<700>();
    RST::off();
    DELAY::ms<400>();
    RST::on();
    DELAY::ms<700>();

    // Bootloader working
    RST::off();
    DELAY::ms<700>();
    VDD::off();
    DELAY::ms<2500>();
    VDD::on();
    DELAY::ms<1500>();
    RST::on();

    // Delay to first digit
    DELAY::ms<1000>();

    uint32_t required_time = send_1byte(k);

    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" required time: ");

    double ms_time = ((double)required_time) / ((double)(F_CPU / 1000));
    Serial.print(ms_time);
    Serial.print(" ms  ");
    Serial.println(required_time);

    if (max_value < required_time) {
      max_value = required_time;
      max_digit = k;
    }
  }
  Serial.print("Max digit: ");
  Serial.println(max_digit);
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
  pinMode(HW::UART_CLK, OUTPUT);
  digitalWrite(HW::UART_CLK, HIGH);

  start_PWM();
  sei();
}

void loop() {

  Serial.println("Press button to start cracking process.");

  // Disable power
  VDD::off();

  // reset counter
  INT::startTimer1();
  INT::noOfTicks = 0;
  INT::overflow_cnt = 0;
  INT::stopTimer1();

  while (digitalRead(HW::BUTTON) == HIGH)
    ;

  while (1) {
    send_256_bytes();

    Serial.println("***********************************************");
  }
}
