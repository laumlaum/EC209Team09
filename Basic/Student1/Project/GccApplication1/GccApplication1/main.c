/*
 * GccApplication2.c
 *
 * Created: 17/09/2026 5:33:04 PM
 * Author : cche632
 */ 

#define F_CPU 2000000UL

#include "timer0.h"
#include "adc.h"
#include "common.h"
#include "int0.h"
#include "timer1.h"
#include "uart.h"
#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint8_t flag;
extern volatile uint16_t adc0[40], adc1[40];

int main(void){
	//TODO: set direction of LED port to OUTPUT
	DDRB = 0xFF;
	DDRD &= ~(1<<PIND2);
	
	
	/*timer0_init();*/
	adc_init();	
	timer1_init();
	uint16_t ubrr = 12;
	usart_init(ubrr);
	/*int0_init();*/
	sei();
	
	DDRB |= (1<<PINB2);
	
	
	
	
	while(1){
		
		if(flag){
			for(uint8_t i = 0; i<40; i++){
				char thousands0 = ((adc0[i]/1000)%10) + 48;
				char hundreds0 = ((adc0[i] / 100)%10) + 48;
				char tens0 = ((adc0[i] / 10) % 10) + 48;
				char ones0 = (adc0[i] % 10) + 48;
				_delay_ms(1);
				usart_transmit(thousands0);
				usart_transmit(hundreds0);
				usart_transmit(tens0);
				usart_transmit(ones0);
				usart_transmit(',');
				usart_transmit(' ');
				char thousands1 = ((adc1[i]/1000)%10) + 48;
				char hundreds1 = ((adc1[i] / 100)%10) + 48;
				char tens1 = ((adc1[i] / 10) % 10) + 48;
				char ones1 = (adc1[i] % 10) + 48;
				_delay_ms(1);
				usart_transmit(thousands1);
				usart_transmit(hundreds1);
				usart_transmit(tens1);
				usart_transmit(ones1);
				usart_transmit('\r');
				usart_transmit('\n');
			}
			_delay_ms(1000);
			
			adc_restart();
		}
		
	}
}
