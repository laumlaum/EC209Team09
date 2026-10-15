/*
 * timer0.c
 *
 * Created: 14/10/2026 1:33:48 pm
 *  Author: johnn
 */ 

#include "timer0.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t one_second_flag = 0;
static uint8_t ticks = 0;

void timer0_init(void)
{
	TCCR0A = (1 << WGM01);                  /* CTC, TOP = OCR0A */
	TCCR0B = (1 << CS01) | (1 << CS00);     /* clk / 64 */
	OCR0A  = 124;                           /* 4 ms */
	TIMSK0 = (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect, ISR_NOBLOCK)
{
	display_next_digit();

	ticks++;
	if (ticks >= 250) {                     /* 250 x 4 ms = 1 s */
		ticks = 0;
		one_second_flag = 1;
	}
}