#ifndef _ADC_H
#define _ADC_H
#include <stdint.h>

void adc_init(void);

/* window 0: V only. window 1: I only. Waits for a real zero-crossing edge
 * (see int0.c) before starting, so it's phase locked to the signal.
 *
 * The native trigger period is 100 us (timer1.c, ADC prescaler /8 below).
 *
 * phase 0: the sampling grid starts right at the zero-crossing edge (same
 *          as before).
 * phase 1: the grid is nudged to start 50 us (half the 100 us native tick)
 *          later, by forcing TCNT1 to 100 instead of 0 before the first
 *          trigger. Every sample after that is still 100 us apart, so this
 *          window's samples land exactly between a phase-0 window's
 *          samples. Pair a phase-0 and phase-1 window of the same channel
 *          and merge them by elapsed time since their own zero-crossing
 *          edge (see main.c) to get an effective 50 us sample spacing per
 *          channel, 4x the original single-channel-at-a-time rate.        */
void adc_restart(uint8_t window, uint8_t phase);

uint16_t adc_read(uint8_t chan);
uint16_t adc_convert_mv(uint16_t value);

#endif