#include "int0.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t zc_flag = 0;

// Interrupt to tell us when a zero crossing has occured
ISR(INT0_vect){
	zc_flag = 1;         // used by adc.c to phase lock the two acquisition windows
}

// Initialises int0
void int0_init(){
	EICRA |= (1<<ISC01) | (1<<ISC00);
	EIMSK |= (1<<INT0);
}
