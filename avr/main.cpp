#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#define BAUD_RATE 9600
#define BAUD_PRESCALE (F_CPU / 16 / BAUD_RATE - 1)

namespace HW {
constexpr uint8_t MODE = 8;
constexpr uint8_t RESET = 4;
constexpr uint8_t VDD_SWICH = 5;
constexpr uint8_t RX_1 = 3;
constexpr uint8_t TX_1 = 9;
constexpr uint8_t BUTTON = 11;
} // namespace HW

///////////////////////////   UART-RC8   ///////////////////////////
void send_bit(bool bit_value) {
  if (bit_value) {
    PORTB |= (1 << PINB1);
  } else {
    PORTB &= ~(1 << PINB1);
  }
}

void softuart_putchar(char ch) {
  constexpr uint16_t delay = 1e6 / BAUD_RATE;
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

///////////////////////////   UART-PC   ///////////////////////////
int uart_send_byte(char byte, FILE *stream) {
  if (byte == '\n') {
    uart_send_byte('\r', stream);
  }
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = byte;
  return 0;
}

int uart_get_byte(FILE *stream) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

void uart_initialize() {
  UBRR0H = (BAUD_PRESCALE >> 8);
  UBRR0L = BAUD_PRESCALE;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static FILE uart_stream;

void stream_init() {
  uart_stream.put = uart_send_byte;
  uart_stream.get = uart_get_byte;
  uart_stream.flags = _FDEV_SETUP_RW;
  stdout = stdin = &uart_stream;
}

void init_UART() {
  uart_initialize();
  stream_init();
}

////////////////////////////   HW-IO   ////////////////////////////
void digitalWrite(uint8_t pin, bool bit) {
  uint8_t bit_name = bit % 8;
  if (pin > 7) {
    // Write to port B
    if (bit) {
      PORTB |= (uint8_t)(1 << bit_name);
    } else {
      PORTB &= (uint8_t) ~(1 << bit_name);
    }
  } else {
    // Write to port D
    if (bit) {
      PORTD |= (uint8_t)(1 << bit_name);
    } else {
      PORTD &= (uint8_t) ~(1 << bit_name);
    }
  }
}

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
  MODE::bootloader();
  digitalWrite(HW::TX_1, 1);
  digitalWrite(HW::MODE, MODE::mode_prev);
  digitalWrite(HW::RESET, RST::rst_prev);
  digitalWrite(HW::VDD_SWICH, 0);
}
void off() {
  digitalWrite(HW::TX_1, 0);
  digitalWrite(HW::MODE, 1);
  digitalWrite(HW::RESET, 1);
  digitalWrite(HW::VDD_SWICH, 1);
}
} // namespace VDD

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
  // wait until RX_1 is high
  while (PIND & (1 << 3))
    ;
  stop();
}

void stop() {
  stopTimer1();
  noOfTicks = getTimerTicks();
}

} // namespace INT

namespace DELAY {
void ns(uint32_t duration_ns) {
  constexpr uint32_t ns_to_ticks = ((1e9 / F_CPU) * 2);
  uint32_t ticks = duration_ns / ns_to_ticks;

  volatile uint32_t counted_ticks = 0;
  while (counted_ticks < ticks) {
    counted_ticks++;
  }
}

void us(uint32_t duration_us) { ns(duration_us * 1000); }

void ms(uint32_t duration_ms) { us(duration_ms * 1000); }

void s(uint8_t duration_s) {
  uint32_t duration = static_cast<uint32_t>(duration_s);
  ms(duration * 1000);
}
} // namespace DELAY

///////////////////////////   PROGRAM   ////////////////////////////
void reset_target() {
  DELAY::ms(100);
  VDD::off();
  RST::off();
  DELAY::ms(1000);
  VDD::on();
  DELAY::ms(100);
  RST::on();
  DELAY::ms(218);
  DELAY::us(125);
}

uint16_t send_1byte(uint8_t key) {
  // Send wakeup time
  for (uint16_t i = 0; i < 16; i++) {
    softuart_putchar(0x00);
    DELAY::ms(30);
  }

  // Send header
  constexpr uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    softuart_putchar(el);
  }

  // Send key
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(key);

  // Send query
  softuart_putchar(0x70);

  // Start interrupt on falling edge (pooling mode)
  INT::start();

  DELAY::ms(4);
  return INT::noOfTicks;
}

void send_256_bytes() {
  uint16_t max_value = 0;
  uint16_t max_digit = 0;
  for (uint16_t k = 0; k < 256; k++) {

    // Power cycle before each attempt
    reset_target();

    uint16_t required_time = send_1byte(k);
    printf("Sending: %u = %ul ticks \n", k, required_time);

    if (max_value < required_time) {
      max_value = required_time;
      max_digit = k;
    }
  }
  printf("Max digit: %u\n", max_digit);
}

void init_IO() {
  // Set output pins
  DDRD |= _BV(HW::RESET) | _BV(HW::TX_1) | _BV(HW::VDD_SWICH);
  DDRB |= _BV(0) | _BV(1) | _BV(3) | _BV(4);

  // Set TX_1 to high
  digitalWrite(HW::TX_1, 1);

  // Activate pullups
  digitalWrite(HW::RX_1, 1);
  digitalWrite(HW::BUTTON, 1);
}

int main() {
  // Initialize all hardware
  init_IO();
  init_UART();
  sei();

  while (1) {
    printf("Press button to start cracking process.\n");

    // Wait unit button is pressed
    while (PINB & (1 << 3)) {
    }

    printf("Start of process");
    DELAY::ms(200);

    send_256_bytes();
    reset_target();

    printf("DONE\n");
    while (1) {
    }
  }
}
