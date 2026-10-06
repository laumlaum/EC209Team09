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

static volatile uint16_t counter = 0;

ISR(TIMER0_COMPA_vect){
	counter++;
	if(counter >= 10) {
		counter = 0;
	}
}

void timer0_init(){
	//TODO: initialise and configure timer0 to count to 10ms
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 77;
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