/*
 * timer0.h
 *
 * Created: 14/10/2026 1:34:11 pm
 *  Author: johnn
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_

#include <stdint.h>

/* Timer0: 4 ms tick. Refreshes one display digit per tick and sets
 * one_second_flag every 250 ticks (1 s). */
void timer0_init(void);

/* Set to 1 by the ISR once per second. main clears it after using it.
 * (8-bit, so main can read/clear it safely without disabling interrupts.) */
extern volatile uint8_t one_second_flag;

#endif