#include "timer1.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>


void timer1_init(){
	// Set up Timer 1 in CTC mode toggle OC1B on compare match. Top OCR1A
	DDRB |= (1<<PINB2);
	TCCR1A |= (1<<COM1B0);
	TCCR1B |= (1<<WGM12) | (1<<CS10);

	// Sets prescaler to trigger per 100us
	OCR1A = 199;
	OCR1B = 199;
}
