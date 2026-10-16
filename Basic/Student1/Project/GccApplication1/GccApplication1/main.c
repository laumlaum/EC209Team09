/*
 * GccApplication2.c
 *
 * Created: 17/09/2026 5:33:04 PM
 * Author : cche632
 */ 

#define F_CPU 2000000UL
#define SAMPLES 80

#include "timer0.h"
#include "adc.h"
#include "common.h"
#include "int0.h"
#include "timer1.h"
#include "uart.h"
#include "calc.h"
#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint8_t flag;
extern volatile uint16_t adc0[SAMPLES], adc1[SAMPLES];
extern volatile uint8_t period_ready;
extern volatile uint32_t elapsed_ticks;

int main(void){
	//TODO: set direction of LED port to OUTPUT
	DDRB = 0xFF;
	DDRD &= ~(1<<PIND2);
	
	
	//timer0_init();
	adc_init();	
	timer1_init();
	uint16_t ubrr = 12;
	usart_init(ubrr);
	//int0_init();
	sei();
	
	DDRB |= (1<<PINB2);
	
	
	uint8_t first = 1;
	
	while(1){
		
		if(flag){
			if(first){
				first = 0;
				adc_restart();
				continue;
			}
			
			usart_transmit_string("BEGIN\r\n");
			usart_transmit_string("k,adc0,adc1\r\n");
			for (uint8_t i = 0; i < SAMPLES; i++) {
				print_int(i);        usart_transmit_string(",");
				print_int(adc0[i]);  usart_transmit_string(",");
				print_int(adc1[i]);  usart_transmit_string("\r\n");
			}
			usart_transmit_string("END\r\n");
			
			float vrms = calc_vrms(adc0,SAMPLES);
			usart_transmit_string("V_rms: ");
			print_int((int32_t)(vrms + 0.5f));      // mV, rounded
			usart_transmit_string(" mV\r\n");
			float ipk = get_ipeak(adc1,SAMPLES);
			usart_transmit_string("Ipk: ");
			print_int((int32_t)(ipk + 0.5f));
			usart_transmit_string(" mA\r\n");
		
			
			float p = calculate_power(adc0, adc1, SAMPLES);
			usart_transmit_string("P: ");
			print_int((int32_t)(p * 1000 + 0.5f));    // mW
			usart_transmit_string(" mW\r\n");
			
// 			if(period_ready){
// 				uint32_t ticks;
// 				cli();
// 				ticks = elapsed_ticks;
// 				period_ready = 0;
// 				sei();
// // 				if (ticks > 0)
// // 				{
// // 					float frequency = 250000.0f / (float)ticks;
// // 
// // 					usart_transmit_string("Period ticks: ");
// // 					print_int(ticks);
// // 					usart_transmit_string(", Frequency: ");
// // 					print_int(frequency);
// // 					usart_transmit_string(" Hz\r\n");
// // 				}

// 			}
			
			_delay_ms(1000);
			adc_restart();
		}
		
	}
}
