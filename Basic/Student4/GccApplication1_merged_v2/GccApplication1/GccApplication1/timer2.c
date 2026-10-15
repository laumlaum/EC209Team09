/*
 * timer2.c  -  4 ms tick for the display ("7-seg timer ISR - runs in parallel")
 *
 * CTC mode, prescaler 64: 1 tick = 64 / 2 MHz = 32 us
 * OCR2A = 124 -> 125 x 32 us = 4.000 ms per interrupt
 * 4 digits x 4 ms = 16 ms per full scan = 62.5 Hz (no visible flicker).
 *
 * ISR_NOBLOCK: interrupts are re-enabled as soon as this ISR starts, so the ADC
 * ISR can interrupt it. Without this, shifting out a digit can delay the ADC ISR
 * past its deadline and mix up the voltage and current samples.
 */

#include "timer2.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t one_second_flag = 0;
static uint8_t ticks = 0;

void timer2_init(void)
{
	TCCR2A = (1 << WGM21);                  /* CTC, TOP = OCR2A */
	TCCR2B = (1 << CS22);                   /* clk / 64  (Timer2's /64 is CS22 alone) */
	OCR2A  = 124;                           /* 4 ms */
	TIMSK2 = (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK)
{
	display_next_digit();

	ticks++;
	if (ticks >= 250) {                     /* 250 x 4 ms = 1 s */
		ticks = 0;
		one_second_flag = 1;
	}
}
