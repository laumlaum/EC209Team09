/*
 * calc.h
 *
 * Created: 8/10/2026 4:46:59 pm
 *  Author: cche632
 */ 


#ifndef CALC_H_
#define CALC_H_
#include <stdint.h>

float convert_voltage(uint16_t raw);
float convert_current(uint16_t raw);

float calc_vrms(const volatile uint16_t *v_raw, uint8_t n);
float get_ipeak(const volatile uint16_t *i_raw, uint8_t n);

float calculate_power(const volatile uint16_t *voltage, const volatile uint16_t *current, uint8_t n);


#endif /* CALC_H_ */
