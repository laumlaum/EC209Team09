/*
 * display.c
 *
 * Created: 14/10/2026 1:34:38 pm
 *  Author: johnn
 */ 

#include "display.h"
#include <avr/io.h>

// Shift register pins
#define SH_CP  PC3
#define SH_DS  PC4
#define SH_ST  PC5

// Lab 6 table: segment patterns for 0..9
static const uint8_t seg_pattern[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static uint8_t buffers[2][4];          /* [0] and [1]: one is front, one is back   */
static uint8_t units[2] = { UNIT_NONE, UNIT_NONE };   /* unit LED for each buffer */
static volatile uint8_t front = 0;     /* which buffer the ISR is showing          */
static volatile uint8_t swap_request = 0;   /* main has filled the back buffer     */
static uint8_t position = 0;           /* ISR only: digit being shown (0..3)       */



void display_init(void)
{
	/* Set the levels BEFORE making the pins outputs, so no digit flashes on at power-up */
	PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);   /* all digits off (active low) */
	PORTC &= ~((1 << SH_CP) | (1 << SH_ST));

	// Configure pins as output
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRC |= (1 << SH_CP) | (1 << SH_DS) | (1 << SH_ST);

	for (uint8_t k = 0; k < 4; k++) {       /* show "----" until the first value */
		buffers[0][k] = SEG_DASH;
		buffers[1][k] = SEG_DASH;
	}
}

// Prepares and formats display
void format_7seg(uint16_t value, uint8_t dp_digit, uint8_t unit)
{
	// Wait until previous buffer update has been processed
	while (swap_request) {
		
	}

	// Swaps buffers
	uint8_t back_index = front ^ 1;
	uint8_t *back = buffers[back_index];

	// Limits value for the displays
	if (value > 9999) {
		value = 9999;
	}
	// Extracts values 
	uint8_t d[4];
	d[0] = value / 1000;
	d[1] = (value / 100) % 10;
	d[2] = (value / 10) % 10;
	d[3] = value % 10;

	// Removes leading zeros while keeping the digits to be displayed
	uint8_t last_needed = (dp_digit <= 3) ? dp_digit : 3;
	uint8_t leading = 1;
	for (uint8_t k = 0; k < 4; k++) {
		if (leading && k < last_needed && d[k] == 0) {
			back[k] = SEG_BLANK;
		} else {
			leading = 0;
			back[k] = seg_pattern[d[k]];
		}
	}
	// Adds decimal point to digit if required
	if (dp_digit <= 3) {
		back[dp_digit] |= SEG_DP;
	}
	units[back_index] = unit;

	swap_request = 1;              /* hand the new value to the ISR */
}

// Sends pattern to shift register
static void shift_out(uint8_t pattern)
{
	// Sets pattern to current bit
	for (uint8_t bit = 0; bit < 8; bit++) {    
		if (pattern & 0x80) {
			PORTC |= (1 << SH_DS);
		} else {
			PORTC &= ~(1 << SH_DS);
		}
		PORTC |= (1 << SH_CP);                   
		PORTC &= ~(1 << SH_CP);
		
		// Moves the next bit into the MSB position.
		pattern <<= 1;
	}
}

// Activates the selected digit
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
	// Swaps digit only at the start of a complete scan
	if (position == 0 && swap_request) {   
		front ^= 1;
		swap_request = 0;

	}

	// Sends pattern for current digit
	shift_out(buffers[front][position]);                          
	PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);   /* 2. all digits off   */
	PORTC |= (1 << SH_ST);                                        /* 3. latch segments   */
	PORTC &= ~(1 << SH_ST);
	digit_on(position);                                           /* 4. this digit on    */

	// Go to next digit
	position = (position + 1) & 0x03;
}

// Converts floating point into a four digit display value
static void display_show(float value, uint8_t unit)
{
	// Prevents negative values
	if (value < 0.0f) {
		value = 0.0f;
	}

	if (value < 9.9995f) {
		format_7seg((uint16_t)(value * 1000.0f + 0.5f), 0, unit);
	} else if (value < 99.995f) {
		format_7seg((uint16_t)(value * 100.0f + 0.5f), 1, unit);
	} else if (value < 999.95f) {
		format_7seg((uint16_t)(value * 10.0f + 0.5f), 2, unit);
	} else if (value < 9999.5f) {
		format_7seg((uint16_t)(value + 0.5f), DP_NONE, unit);
	} else {
		format_7seg(9999, DP_NONE, unit);
	}
}

void display_show_voltage(float vrms)  { display_show(vrms,   UNIT_VOLTAGE); }
void display_show_current(float ipk)   { display_show(ipk,    UNIT_CURRENT); }
void display_show_power(float watts)   { display_show(watts,  UNIT_POWER);   }
//void display_show_energy(float energy) { display_show(energy, UNIT_ENERGY);  }
