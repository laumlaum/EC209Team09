/*
 * display.h
 *
 * Created: 14/10/2026 1:34:56 pm
 *  Author: johnn
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

/* Segment patterns: bit 0 = a ... bit 6 = g, bit 7 = dp  (74HC595 Q0 = a ... Q7 = dp) */
#define SEG_DP     0x80
#define SEG_BLANK  0x00
#define SEG_U      0x3E    /* "U" for voltage */
#define SEG_A      0x77    /* "A" for current */
#define SEG_P      0x73    /* "P" for power   */
#define SEG_DASH   0x40    /* "-"             */

#define DP_NONE    0xFF    /* no decimal point */

void display_init(void);

/* Show a 3-digit number plus a unit character on the 4th digit, e.g.
 *   format_7seg(145, 1, SEG_U)        -> "14.5U"
 *   format_7seg(532, DP_NONE, SEG_A)  -> "532A"
 *   format_7seg(521, 0, SEG_P)        -> "5.21P"
 * value is limited to 0..999. dp_digit = which of the first 3 digits gets the
 * decimal point (0, 1, 2) or DP_NONE. Leading zeros are blanked.
 * Writes the BACK buffer; the ISR swaps it in at the start of its next full scan. */
void format_7seg(uint16_t value, uint8_t dp_digit, uint8_t unit_segments);

/* Called from the Timer0 ISR every 4 ms: shows the next of the 4 digits. */
void display_next_digit(void);

#endif