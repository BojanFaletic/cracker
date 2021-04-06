#pragma once

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif
#include <avr/io.h>
#include <stdio.h>

#define BAUD_RATE 9600
#define BAUD_PRESCALE (F_CPU / 16 / BAUD_RATE - 1)

#ifdef __cplusplus
extern "C" {
FILE *uart_str;
}
#endif

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

void uart_init(){
  uart_initialize();
  stream_init();
}