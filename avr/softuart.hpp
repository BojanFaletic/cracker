#pragma once

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

// TX = 9 pin
#define SOFT_DDR_PORT DDRB
#define SOFT_PORT PORTB
#define SOFT_TX_PIN 1

#define BAUDRATE 9600

void wait_for_baud() {
  constexpr uint16_t delay = 1e6 / BAUDRATE;
  _delay_us(delay);
}

void set() { PORTB |= (1 << PINB1); }

void clear() { PORTB &= ~(1 << PINB1); }

void send_bit(bool value) {
  if (value) {
    set();
  } else {
    clear();
  }
  return;
}

void init_tx() {
  // set as high impedance
  DDRB |= (1 << PINB1);
  // input with pullup
  PORTB |= (1 << PINB1);
}

void softuart_putchar(char ch) {
  for (int i = 0; i < 10; i++) {
    // start bit
    wait_for_baud();
    if (i == 0) {
      clear();
      continue;
    }

    if (i < 9) {
      // send 8 bits of data
      bool value = ch & 0x01;
      ch >>= 1;
      send_bit(value);
      continue;
    }

    // stop bit
    set();
  }
}