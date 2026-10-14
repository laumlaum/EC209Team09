/*
 * calc.c
 *
 * Created: 8/10/2026 4:46:51 pm
 *  Author: cche632
 */ 

#include "calc.h"
#include <stdint.h>
#include <math.h>

#define OFFSET  2080.0f        
#define GVS  0.0459f        
#define GVO  1.0f         
#define GIS  0.6f         
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
	float max = 0;
	for(uint8_t k = 0; k < n; k++){
		float current = convert_current(i_raw[k]);
		if(current>=max){
			max = current;
		}
	}
	return max;
}

float calculate_power(const volatile uint16_t *voltage, const volatile uint16_t *current, uint8_t n){
	float sum = 0.0f;

	// The current sample is taken halfway between
	// the previous and current voltage sample times.
	for (uint8_t k = 1; k < n; k++) {

		float v = convert_voltage(voltage[k]);

		float i_prev = convert_current(current[k - 1]);
		float i_now  = convert_current(current[k]);

		// Interpolate current to the voltage sample time
		float i_aligned = 0.5f * (i_prev + i_now);

		// Voltage in mV * current in mA = µW
		sum += v * i_aligned;
	}

	// Average power in watts
	return (sum / (float)(n - 1)) / 1000.0f;
}
