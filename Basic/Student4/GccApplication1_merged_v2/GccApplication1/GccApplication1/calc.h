/*
 * calc.h
 *
 * Created: 8/10/2026 4:46:59 pm
 *  Author: cche632
 */ 


#ifndef CALC_H_
#define CALC_H_
#include <stdint.h>

/* All three take the raw ADC arrays straight from adc.c.
 * Offsets are removed automatically (mean of the window).                   */
float calc_vrms(const volatile uint16_t *v_raw, uint8_t n);                 /* mV rms  */
float get_ipeak(const volatile uint16_t *i_raw, uint8_t n);                 /* mA peak */
float calculate_power(const volatile uint16_t *voltage,
                      const volatile uint16_t *current, uint8_t n);         /* W       */


#endif /* CALC_H_ */
