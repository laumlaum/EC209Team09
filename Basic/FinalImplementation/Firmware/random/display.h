/*
 * display.h
 *
 * Created: 14/10/2026 1:34:56 pm
 *  Author: johnn
 *
 * 4-digit display board: all 4 digits show the number, and the unit is shown
 * by one of the four LEDs above the digits (Voltage, Current, Power, Energy).
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

/* Segment patterns: bit 0 = a ... bit 6 = g, bit 7 = dp  (74HC595 Q0 = a ... Q7 = dp) */
#define SEG_DP     0x80
#define SEG_BLANK  0x00
#define SEG_DASH   0x40    /* "-" */

#define DP_NONE    0xFF    /* no decimal point */

/* Which unit LED is lit */
#define UNIT_NONE     0xFF
#define UNIT_VOLTAGE  0
#define UNIT_CURRENT  1
#define UNIT_POWER    2
#define UNIT_ENERGY   3

void display_init(void);

/* Show a 4-digit number and light one unit LED, e.g.
 *   format_7seg(1400, 1, UNIT_VOLTAGE)  -> "14.00" + Voltage LED
 *   format_7seg(697, 0, UNIT_CURRENT)   -> "0.697" + Current LED
 * value is limited to 0..9999. dp_digit = which digit (0..3) gets the decimal
 * point, or DP_NONE. Leading zeros are blanked.
 * Writes the BACK buffer; the ISR swaps it in (number and LED together) at the
 * start of its next full scan. */
void format_7seg(uint16_t value, uint8_t dp_digit, uint8_t unit);

/* Called from the Timer0 ISR every 4 ms: shows the next of the 4 digits. */
void display_next_digit(void);

/* ---- Hand-over from the calc code: pass the real values ----
 * The decimal point is placed automatically to show 4 significant digits:
 *   display_show_voltage(14.00f)  -> "14.00"   V rms
 *   display_show_current(0.697f)  -> "0.697"   A peak
 *   display_show_power(6.185f)    -> "6.185"   W
 *   display_show_energy(123.4f)   -> "123.4"   (whatever unit main passes in)
 * Negative values show as 0. Values >= 9999.5 show as 9999. */
void display_show_voltage(float vrms);
void display_show_current(float ipk);
void display_show_power(float watts);
//void display_show_energy(float energy);

#endif
