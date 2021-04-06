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

#define set_pin(port, pin) port |= (1 << pin);
#define clear_pin(port, pin) port &= ~(1 << pin);

void wait_for_baud() {
  constexpr uint16_t delay = 1e6 / BAUDRATE;
  _delay_us(delay);
}

void set() {
  // enable pull up resistor
  set_pin(SOFT_PORT, SOFT_TX_PIN);
}

void clear() {
  // disable pullup resistor
  clear_pin(SOFT_PORT, SOFT_TX_PIN);
}

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
  set_pin(SOFT_DDR_PORT, SOFT_TX_PIN);
  // input with pullup
  set_pin(SOFT_PORT, SOFT_TX_PIN);
}

void softuart_putchar(char ch) {
  for (int i = 0; i < 10; i++) {
    // start bit
    wait_for_baud();
    if (i == 0) {
      clear_pin(SOFT_PORT, SOFT_TX_PIN);
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
    set_pin(SOFT_PORT, SOFT_TX_PIN);
  }
}