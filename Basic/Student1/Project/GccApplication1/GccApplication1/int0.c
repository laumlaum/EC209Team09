#include "timer0.h"


#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t elapsed_ticks = 0;
volatile uint8_t have_previous_edge = 0;
volatile uint8_t period_ready = 0;

ISR(INT0_vect){
	
	uint32_t timestamp;
	
	timestamp = ((uint32_t)timer0counter * 256) + TCNT0;
	
	static uint32_t previous_timestamp = 0;
	static uint8_t first_edge = 1;
	
	if(first_edge){
		previous_timestamp = timestamp;
		first_edge = 0;
	} else {
		elapsed_ticks = timestamp - previous_timestamp;
		previous_timestamp = timestamp;
		period_ready = 1;
	}
	
}

void int0_init(){
	EICRA |= (1<<ISC01) | (1<<ISC00);
	EIMSK |= (1<<INT0);
}
