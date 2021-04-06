#include <avr/io.h>
#include <util/delay.h>

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
  init_uart();

  // soft uart
  init_tx();


  DDRB |= (1 << PB0);

  while (1) {
    PORTB ^= (1 << PB0);
    printf("Hello world\n");
    softuart_putchar('A');
    _delay_us(1000);
  }
}
