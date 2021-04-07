#include <avr/io.h>
#include <util/delay.h>

#define HIGH 1
#define LOW 0

#include "print.hpp"
#include "softuart.hpp"

namespace HW {
// Harware definitions
constexpr uint8_t MODE = 8;
constexpr uint8_t RESET = 4;
constexpr uint8_t VDD_SWICH = 5;
constexpr uint8_t RX_1 = 3;
constexpr uint8_t TX_1 = 9;
constexpr uint8_t BLANK = 12;
constexpr uint8_t BUTTON = 11;
} // namespace HW

void digitalWrite(uint8_t pin, bool bit) {
  volatile uint8_t *port;
  uint8_t bit_name = bit % 8;
  if (pin > 7) {
    port = (volatile uint8_t *)DDRB;
  } else {
    port = (volatile uint8_t *)DDRD;
  }

  if (bit) {
    *port |= (uint8_t)(1 << bit_name);
  } else {
    *port &= (uint8_t) ~(1 << bit_name);
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
static uint32_t counter_ov_cnt;
constexpr uint8_t CLK_DIV_1 = 1 << 0;

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
  sei();
}

void _delay(uint32_t ticks) {
  counter_ov_cnt = 0;
  _clear();
  _start();
  auto cnt_ticks = []() {
    return (counter_ov_cnt << 8) | static_cast<uint32_t>(TCNT2);
  };

  while (cnt_ticks() < ticks) {
  }
  _stop();
}

void ns(uint32_t duration_ns) {
  constexpr uint32_t ns_to_ticks = ((1e9 / F_CPU) * 2);
  uint32_t ticks = duration_ns / ns_to_ticks;
  _delay(ticks);
}

void us(uint32_t duration_us) { _delay_us(duration_us); }

void ms(uint16_t duration_ms) { _delay_ms(duration_ms); }

void s(uint8_t duration_s) {
  constexpr uint32_t s_to_ms = 1000;
  constexpr uint32_t max_value = static_cast<uint32_t>(-1);
  uint32_t duration = static_cast<uint32_t>(duration_s);
  uint32_t d_ms =
      (duration < max_value - s_to_ms) ? duration * s_to_ms : max_value;
  ms(d_ms);
}

uint16_t send_1byte(uint8_t key) {
  // send wakeup time
  for (uint16_t i = 0; i < 16; i++) {
    softuart_putchar(0x00);
    DELAY::ms(30);
  }

  // send header
  constexpr uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    softuart_putchar(el);
  }

  // send key
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(0x00);
  softuart_putchar(key);

  // send query
  softuart_putchar(0x70);

  // Start interrupt on falling edge (pooling mode)
  INT::start();

  DELAY::ms(4);
  return INT::noOfTicks;
}

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

void send_256_bytes() {
  uint16_t max_value = 0;
  uint16_t max_digit = 0;
  for (uint16_t k = 0; k < 256; k++) {

    // Power cycle before each attempt
    reset_target();

    uint16_t required_time = send_1byte(k);
    printf("Sending %u = %ul\n", k, required_time);

    if (max_value < required_time) {
      max_value = required_time;
      max_digit = k;
    }
  }
  printf("Max digit: %u\n", max_digit);
}

void init_hw() {
  // set output pins
  DDRD |= _BV(HW::RESET) | _BV(HW::TX_1) | _BV(HW::VDD_SWICH);
  DDRB |= _BV(0) | _BV(1) | _BV(3) | _BV(4);

  // Activate pullups
  digitalWrite(HW::RX_1, 1);
  digitalWrite(HW::BUTTON, 1);
}

int main() {
  init_hw();
  init_uart();

  while (1) {
    printf("Press button to start cracking process.\n");
    // wait unit BUTTON is pressed
    loop_until_bit_is_set(PINB, 3);

    printf("Start of process");
    DELAY::ms(200);

    send_256_bytes();
    reset_target();

    printf("DONE\n");

    while (1) {
    }
  }
}
