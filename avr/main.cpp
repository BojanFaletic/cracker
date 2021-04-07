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

void init_port() {
  // set output pins
  DDRD = _BV(HW::RESET) | _BV(HW::TX_1) | _BV(HW::VDD_SWICH);
  DDRB = _BV(0) | _BV(1) | _BV(3) | _BV(4);
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
  /*
  while (digitalRead(HW::RX_1))
    ;
    */
  stop();
}

void stop() {
  stopTimer1();
  noOfTicks = getTimerTicks();
}

} // namespace INT




int main() {
  uart_init();


  while (1) {

    printf("Hello world\n");
    softuart_putchar('A');
    _delay_ms(1000);
  }
}
