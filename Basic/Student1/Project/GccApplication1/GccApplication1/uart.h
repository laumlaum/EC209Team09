#ifndef UART_H
#define UART_H

#include <avr/io.h>

void usart_init(uint16_t ubrr);
void usart_transmit(uint8_t data);
void usart_transmit_string(char *str);

#endif