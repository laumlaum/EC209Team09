/*
 * calc.c
 *
 * Created: 8/10/2026 4:46:51 pm
 *  Author: cche632
 */ 

#include "calc.h"
#include <stdint.h>
#include <math.h>

#define SIGNAL_HZ      500.0f       // 50 for these tests, 500 for the real source
#define TRIG_PERIOD_S  125.0e-6f   // must match OCR1A/OCR1B in timer1.c
#define PI_F           3.14159265f
#define OFFSET  2080.0f        
#define GVS  0.0459f        
#define GVO  1.0f         
#define GIS  0.55f         
#define GIO  2.478f         

float convert_voltage(uint16_t raw){
	float pin_mv = ((float)raw * 5000L) / 1024;
	return (pin_mv - OFFSET) / (GVS * GVO);      // mV
}

float convert_current(uint16_t raw){
	float pin_mv = ((float)raw * 5000L) / 1024;
	return (pin_mv - OFFSET) / (GIS * GIO);      // mA
}

float calc_vrms(const volatile uint16_t *v_raw, uint8_t n){
	float sum = 0;
	for(uint8_t i = 0; i < n; i++){
		float v = convert_voltage(v_raw[i]);
		sum += v * v;
	}
	return sqrtf(sum/n);
}



float get_ipeak(const volatile uint16_t *i_raw, uint8_t n){
	float sum = 0;
	for(uint8_t k = 0; k < n; k++){
		float i = convert_current(i_raw[k]);
		sum += i * i;
	}
	return sqrtf(sum / n) * 1.41421356f; 
}

float calculate_power(const volatile uint16_t *voltage, const volatile uint16_t *current, uint8_t n){
	float gain = 1.0f / cosf(2.0f * PI_F * SIGNAL_HZ * TRIG_PERIOD_S);
	float sum = 0.0f;
	float i_prev = convert_current(current[n - 1]);       // I[-1] = I[N-1]

	for(uint8_t k = 0; k < n; k++){
		float v      = convert_voltage(voltage[k]);
		float v_next = convert_voltage(voltage[(k + 1 == n) ? 0 : k + 1]);   // V[N] = V[0]
		float i_now  = convert_current(current[k]);

		float i_bar = 0.5f * (i_prev + i_now);   // current at the time of V[k]
		float v_bar = 0.5f * (v + v_next);       // voltage at the time of I[k]

		sum += v * i_bar + v_bar * i_now;        // mV * mA = uW
		i_prev = i_now;
	}
	return (sum / (2.0f * n)) / 1.0e6f * gain;   // watts
}
