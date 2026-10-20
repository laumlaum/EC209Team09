#ifndef CALC_H_
#define CALC_H_
#include <stdint.h>

float calc_vrms(const volatile uint16_t *v_raw, uint8_t n);                 /* mV rms  */
float get_ipeak(const volatile uint16_t *i_raw, uint8_t n);                 /* mA peak */

/* Power from the merged, phase-locked V/I arrays built in main.c: v_raw and
 * i_raw are genuinely the same instant at every index k, so this is a direct
 * product, no interpolation, no cosine correction. Raw, physics-based watts
 * only - no empirical gain/offset calibration applied. */
float calculate_power_interleaved(const volatile uint16_t *v_raw,
                                   const volatile uint16_t *i_raw, uint8_t n); /* W */

float   estimate_hz(const volatile uint16_t *v_raw, uint8_t n);
uint8_t whole_cycle_samples(float signal_hz, uint8_t n);

#endif /* CALC_H_ */