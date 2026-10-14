#include "uart.h"


void usart_init(uint16_t ubrr) {
	UCSR0B = (1<< TXEN0) | (1<< RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;

}

void usart_transmit(uint8_t data){
	while (!(UCSR0A & (1<<UDRE0))) {

	}

	UDR0 = data;
}
void usart_transmit_string(char *str){
	for(int i =0; str[i] != '\0'; i++) {
		usart_transmit(str[i]);
	}
}

void print_int(int32_t x){
	char buf[11];
	uint8_t i = 0;
	if(x < 0){ usart_transmit('-'); x = -x; }
	if(x == 0){ usart_transmit('0'); return; }
	while(x){ buf[i++] = '0' + (x % 10); x /= 10; }
	while(i) usart_transmit(buf[--i]);
}
