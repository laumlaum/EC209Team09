/*
 * timer2.h  -  4 ms tick for the 7-segment display ("7-seg timer ISR")
 *
 * Timer0 is used by int0.c (frequency measurement), so the display uses Timer2.
 */

#ifndef TIMER2_H_
#define TIMER2_H_

#include <stdint.h>

/* Timer2: 4 ms tick. Refreshes one display digit per tick and sets
 * one_second_flag every 250 ticks (1 s). */
void timer2_init(void);

/* Set to 1 by the ISR once per second. main clears it after using it.
 * (8-bit, so main can read/clear it safely without disabling interrupts.) */
extern volatile uint8_t one_second_flag;

#endif
