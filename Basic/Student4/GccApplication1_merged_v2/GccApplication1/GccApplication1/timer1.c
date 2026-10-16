/*
 * timer1.c
 *
 * Created: 1/10/2026 5:11:01 pm
 *  Author: cche632
 */ 
#include "timer1.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>


void timer1_init(){
	// Set up Timer 1 in CTC mode toggle OC1B on compare match. Top OCR1A
	DDRB |= (1<<PINB2);
	TCCR1A |= (1<<COM1B0);
	TCCR1B |= (1<<WGM12) | (1<<CS10);
	
	/* 400 x 0.5 us = 200 us per conversion (see common.h, TRIG_PERIOD_S).
	 * 249 (125 us) left only ~13 us between the end of the ADC ISR's
	 * flag clear and the next compare match: any other ISR running at that
	 * moment (e.g. INT0) would make it miss a trigger. */
	OCR1A = 399;
	OCR1B = 399;
		
}
