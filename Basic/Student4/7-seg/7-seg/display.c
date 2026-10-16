/*
 * display.c
 *
 * Created: 14/10/2026 1:34:38 pm
 *  Author: johnn
 */ 

#include "display.h"
#include <avr/io.h>

#define SH_CP  PC3
#define SH_DS  PC4
#define SH_ST  PC5

/* Lab 6 table: segment patterns for 0..9 */
static const uint8_t seg_pattern[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static uint8_t buffers[2][4];          /* [0] and [1]: one is front, one is back   */
static volatile uint8_t front = 0;     /* which buffer the ISR is showing          */
static volatile uint8_t swap_request = 0;   /* main has filled the back buffer     */
static uint8_t position = 0;           /* ISR only: digit being shown (0..3)       */

void display_init(void)
{
	/* Set the levels BEFORE making the pins outputs, so no digit flashes on at power-up */
	PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);   /* all digits off (active low) */
	PORTC &= ~((1 << SH_CP) | (1 << SH_ST));

	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRC |= (1 << SH_CP) | (1 << SH_DS) | (1 << SH_ST);

	for (uint8_t k = 0; k < 4; k++) {       /* show "----" until the first value */
		buffers[0][k] = SEG_DASH;
		buffers[1][k] = SEG_DASH;
	}
}

void format_7seg(uint16_t value, uint8_t dp_digit, uint8_t unit_segments)
{
	while (swap_request) {
		/* previous value not taken by the ISR yet (at most one 16 ms scan) */
	}

	uint8_t *back = buffers[front ^ 1];

	if (value > 999) {
		value = 999;
	}
	uint8_t d[3];
	d[0] = value / 100;
	d[1] = (value / 10) % 10;
	d[2] = value % 10;

	/* Blank leading zeros, but never the digit holding the decimal point or the
	 * last digit:  85 -> " 85",  0.75 -> "0.75" */
	uint8_t last_needed = (dp_digit <= 2) ? dp_digit : 2;
	uint8_t leading = 1;
	for (uint8_t k = 0; k < 3; k++) {
		if (leading && k < last_needed && d[k] == 0) {
			back[k] = SEG_BLANK;
		} else {
			leading = 0;
			back[k] = seg_pattern[d[k]];
		}
	}
	if (dp_digit <= 2) {
		back[dp_digit] |= SEG_DP;
	}
	back[3] = unit_segments;

	swap_request = 1;              /* hand the new value to the ISR */
}

static void shift_out(uint8_t pattern)
{
	for (uint8_t bit = 0; bit < 8; bit++) {      /* MSB (dp) first -> ends up on Q7 */
		if (pattern & 0x80) {
			PORTC |= (1 << SH_DS);
		} else {
			PORTC &= ~(1 << SH_DS);
		}
		PORTC |= (1 << SH_CP);                   /* rising edge shifts the bit in */
		PORTC &= ~(1 << SH_CP);
		pattern <<= 1;
	}
}

static void digit_on(uint8_t pos)
{
	switch (pos) {
	case 0:  PORTD &= ~(1 << PD4); break;   /* Ds1 */
	case 1:  PORTD &= ~(1 << PD5); break;   /* Ds2 */
	case 2:  PORTD &= ~(1 << PD6); break;   /* Ds3 */
	default: PORTD &= ~(1 << PD7); break;   /* Ds4 */
	}
}

void display_next_digit(void)
{
	if (position == 0 && swap_request) {    /* swap only at the start of a full scan */
		front ^= 1;
		swap_request = 0;
	}

	shift_out(buffers[front][position]);                          /* 1. load segments    */
	PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);   /* 2. all digits off   */
	PORTC |= (1 << SH_ST);                                        /* 3. latch segments   */
	PORTC &= ~(1 << SH_ST);
	digit_on(position);                                           /* 4. this digit on    */

	position = (position + 1) & 0x03;
}