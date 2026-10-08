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

void calculate_measurements(const volatile uint16_t voltage, const volatile uint16_t current, uint8_t n, );


#endif /* CALC_H_ */