#include "timer0.h"


#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t elapsed_ticks = 0;
volatile uint8_t measurement_ready = 0;

ISR(INT0_vect){
	PORTB ^= (1<<PINB5);
	if(EICRA & (1<<ISC00)){
		TCNT0 = 0;
		TCCR0B |= (1<<CS02);
		EICRA &= ~(1<<ISC00);
		} else {
		TCCR0B &= ~(1<<CS02);
		elapsed_ticks = TCNT0;
		measurement_ready = 1;
		EICRA |= (1<<ISC00);
	}
	
}

void int0_init(){
	EICRA |= (1<<ISC01) | (1<<ISC00);
	EIMSK |= (1<<INT0);
}