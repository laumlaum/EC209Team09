#include "common.h"
#include "calc.h"
#include <stdint.h>
#include <math.h>

#define SAMPLE_PERIOD_S  0.00005f  /* merged V/I arrays are 50 us apart now */

#define GVS  0.0460f
#define GVO  1.0f
#define GIS  0.49f
#define GIO  2.485f

#define VREF_MV  4978.0f
#define MV_PER_COUNT  ((VREF_MV / 1024.0f) / (GVS * GVO))
#define MA_PER_COUNT  ((VREF_MV / 1024.0f) / (GIS * GIO))

// Function to calculate the mean
static float mean_raw(const volatile uint16_t *raw, uint8_t n){
	float sum = 0.0f;
	for(uint8_t k = 0; k < n; k++){
		sum += (float)raw[k];
	}
	return sum / n;
}

// Gets the RMS value of inserted array
static float rms_counts(const volatile uint16_t *raw, uint8_t n){
	float mean = mean_raw(raw, n);
	float sum_sq = 0.0f;
	for(uint8_t k = 0; k < n; k++){
		float d = (float)raw[k] - mean;
		sum_sq += d * d;
	}
	return sqrtf(sum_sq / n);
}

// Function to calculate Voltage RMS
float calc_vrms(const volatile uint16_t *v_raw, uint8_t n){
	return rms_counts(v_raw, n) * MV_PER_COUNT;
}

// Calculates peak current 
float get_ipeak(const volatile uint16_t *i_raw, uint8_t n){
	float ipeak_ma = rms_counts(i_raw, n) * MA_PER_COUNT * 1.41421356f;
	return (ipeak_ma < 0.0f) ? 0.0f : ipeak_ma;
}

// Gets full ADC sample for voltage first then current, then uses this to calculate power
float calculate_power_interleaved(const volatile uint16_t *v_raw,
const volatile uint16_t *i_raw, uint8_t n){
	float v_mean = mean_raw(v_raw, n);
	float i_mean = mean_raw(i_raw, n);
	float sum = 0.0f;
	for(uint8_t k = 0; k < n; k++){
		float v = (float)v_raw[k] - v_mean;
		float i = (float)i_raw[k] - i_mean;
		sum += v * i;
	}
	float p = (sum / n) * (MV_PER_COUNT * MA_PER_COUNT / 1.0e6f);
	return (p < 0.0f) ? 0.0f : p;
}
// Estimates the frequency of the source 
float estimate_hz(const volatile uint16_t *v_raw, uint8_t n){
	float mean = mean_raw(v_raw, n);
	uint8_t crossings = 0;
	float first_cross = -1.0f, last_cross = -1.0f;

	for(uint8_t k = 1; k < n; k++){
		float prev = (float)v_raw[k-1] - mean;
		float cur  = (float)v_raw[k]   - mean;
		if(prev < 0.0f && cur >= 0.0f){
			float frac = -prev / (cur - prev);
			float t = ((float)(k-1) + frac) * SAMPLE_PERIOD_S;
			if(first_cross < 0.0f) first_cross = t;
			last_cross = t;
			crossings++;
		}
	}

	if(crossings < 2 || last_cross <= first_cross){
		return SIGNAL_HZ;
	}

	float cycles = (float)(crossings - 1);
	float total_t = last_cross - first_cross;
	return cycles / total_t;
}

// Calculates the best number of samples that can fit in the frequency
uint8_t whole_cycle_samples(float signal_hz, uint8_t n){
	if(signal_hz < 1.0f) return n;
	float period_s = 1.0f / signal_hz;
	float samples_per_cycle = period_s / SAMPLE_PERIOD_S;
	if(samples_per_cycle < 1.0f) return n;

	float n_cycles_f = (float)n / samples_per_cycle;
	uint8_t n_cycles = (uint8_t)n_cycles_f;
	if(n_cycles < 1) n_cycles = 1;

	uint8_t trimmed = (uint8_t)(n_cycles * samples_per_cycle + 0.5f);
	if(trimmed > n) trimmed = n;
	if(trimmed < 2) trimmed = n;
	return trimmed;
}
