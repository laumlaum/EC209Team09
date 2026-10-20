#include "int0.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t zc_flag = 0;

/* All adc_restart() needs from an edge is to know one happened, so that's
 * all this ISR does now. The old timer0-based period measurement is gone:
 * estimate_hz() in calc.c works out the mains frequency directly from the
 * sampled V array instead, and that code depended on timer0_init() having
 * been called, which it no longer is. */
ISR(INT0_vect){
	zc_flag = 1;         // used by adc.c to phase lock the two acquisition windows
}

// Initialises int0
void int0_init(){
	EICRA |= (1<<ISC01) | (1<<ISC00);
	EIMSK |= (1<<INT0);
}