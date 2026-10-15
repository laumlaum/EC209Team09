/*
 * timer0.c
 *
 * Created: 17/09/2026 5:34:54 PM
 *  Author: cche632
 */ 
#include "timer0.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint16_t timer0counter = 0;

ISR(TIMER0_OVF_vect){
	timer0counter++;
}

void timer0_init(){
	//TODO: initialise and configure timer0 to count to 10ms
	timer0counter = 0;
	TCCR0A = 0;
	TCCR0B |= (1<<CS01); // Prescaler of 8
	TIMSK0 |= (1<<TOIE0); // TImer overflow interrupt enable
	TIFR0 |= (1<<TOV0);
}

uint8_t timer0_check_clear_compare(){
	if( TIFR0 & (1 << OCF0A )){ //TODO: check compare flag
		//TODO: clear compare flag.
		//Note: in datasheet this is done by writing 1 to the compare flag
		TIFR0 = (1<<OCF0A);
		return 1;
	}
	return 0;
}
