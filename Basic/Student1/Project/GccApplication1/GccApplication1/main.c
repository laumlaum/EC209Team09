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
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void){
	//TODO: set direction of LED port to OUTPUT
	DDRB = 0xFF;
	DDRD &= ~(1<<PIND2);
	
	/*timer0_init();*/
	adc_init();	
	timer1_init();
	/*int0_init();*/
	sei();
	
	DDRB |= (1<<PINB2);
	
	while(1){
		
		
		
	}
}