/*
 * calc.c
 *
 * Created: 8/10/2026 4:46:51 pm
 *  Author: cche632
 */ 

#include "common.h"
#include "calc.h"
#include <stdint.h>
#include <math.h>

#define PI_F           3.14159265f
#define SAMPLE_PERIOD_S  0.0004f

/* ---- Gains (from our circuit) ----
 * Voltage: divider R4/(R3+R4) = 2.7k/58.7k = 0.0460, diff amp gain 1.00
 *          (R10 5.6k + divider Thevenin 2.58k = 8.18k ~ R11 8.2k)
 * Current: shunt R5 || R6 = 0.5 ohm, amp gain (1 + 8.2/3.3) x 8.2/11.5 = 2.485 */
#define GVS  0.0460f        /* V at divider per V of supply  */
#define GVO  1.0f           /* voltage amp gain              */
#define GIS  0.575f           /* V across shunt per A of load  */
#define GIO  2.485f         /* current amp gain              */

#define IPEAK_GAIN      1.123f
#define IPEAK_OFFSET_MA 11.6f
#define POWER_GAIN      1.067f
#define POWER_OFFSET_W  0.177f

#define VREF_MV  4978.0f    

/* Scale factors, worked out by the compiler (no division at run time) */
#define MV_PER_COUNT  ((VREF_MV / 1024.0f) / (GVS * GVO))    /* supply mV per ADC count */
#define MA_PER_COUNT  ((VREF_MV / 1024.0f) / (GIS * GIO))    /* load mA per ADC count   */


// function to calculate the mean 
static float mean_raw(const volatile uint16_t *raw, uint8_t n){
	uint32_t sum = 0;
	for(uint8_t k = 0; k < n; k++){
		sum += raw[k];
	}
	return (float)sum / n;
}

/* RMS of one channel in ADC counts, with the mean removed */
static float rms_counts(const volatile uint16_t *raw, uint8_t n){
	float mean = mean_raw(raw, n);
	float sum = 0;
	for(uint8_t k = 0; k < n; k++){
		float x = (float)raw[k] - mean;
		sum += x * x;
	}
	return sqrtf(sum / n);
}

float calc_vrms(const volatile uint16_t *v_raw, uint8_t n){
	return rms_counts(v_raw, n) * MV_PER_COUNT;                       // mV rms
}



float get_ipeak(const volatile uint16_t *i_raw, uint8_t n){
	float ipeak_ma = rms_counts(i_raw, n) * MA_PER_COUNT * 1.41421356f;   // mA peak, uncorrected
	float ipeak_cal = IPEAK_GAIN * ipeak_ma - IPEAK_OFFSET_MA;            // mA peak, corrected
	return (ipeak_cal < 0.0f) ? 0.0f : ipeak_cal;
}

// Calculate power using linear approximation
float calculate_power(const volatile uint16_t *voltage, const volatile uint16_t *current, uint8_t n, float signal_hz){
	// Undo shrink from averaging samples TRIG_PERIOD apart
	float gain = 1.0f / cosf(2.0f * PI_F * signal_hz * TRIG_PERIOD_S);

	float v_mean = mean_raw(voltage, n);
	float i_mean = mean_raw(current, n);
	float sum = 0.0f;

	// The window is a whole number of cycles, so it wraps around:
	// I[-1] = I[N-1] and V[N] = V[0].
	float i_prev = (float)current[n - 1] - i_mean;

	for(uint8_t k = 0; k < n; k++){
		float v      = (float)voltage[k] - v_mean;
		float v_next = (float)voltage[(k + 1 == n) ? 0 : k + 1] - v_mean;
		float i_now  = (float)current[k] - i_mean;

		float i_bar = 0.5f * (i_prev + i_now);   // current at the time of V[k]
		float v_bar = 0.5f * (v + v_next);       // voltage at the time of I[k]

		sum += v * i_bar + v_bar * i_now;        // counts x counts
		i_prev = i_now;
	}

	// counts^2 -> mV x mA = uW -> W
	float p = (sum / (2.0f * n)) * (MV_PER_COUNT * MA_PER_COUNT / 1.0e6f) * gain;

	p = POWER_GAIN * p - POWER_OFFSET_W;
	return (p < 0.0f) ? 0.0f : p;
}


// Frequency from the rising zero crossing of voltage 
float estimate_hz(const volatile uint16_t *v_raw, uint8_t n){
	uint32_t sum = 0;
	for(uint8_t k = 0; k < n; k++){
		sum += v_raw[k];
	}
	/* Compare x*n against the sum instead of x against the meanm, keeps integer only */
	int32_t prev = (int32_t)v_raw[0] * n - (int32_t)sum;
	float first = 0.0f, last = 0.0f;
	uint8_t crossings = 0;

	for(uint8_t k = 1; k < n; k++){
		int32_t now = (int32_t)v_raw[k] * n - (int32_t)sum;
		if(prev < 0 && now >= 0){                       // rising crossing
			float t = (float)(k - 1) + (float)prev / (float)(prev - now);
			if(crossings == 0) first = t;
			last = t;
			crossings++;
		}
		prev = now;
	}
	if(crossings < 3){
		return SIGNAL_HZ;                               // no clear signal
	}
	float hz = (float)(crossings - 1) / ((last - first) * SAMPLE_PERIOD_S);
	if(hz < 0.8f * SIGNAL_HZ || hz > 1.2f * SIGNAL_HZ){
		return SIGNAL_HZ;                               // implausible: ignore
	}
	return hz;
}

// Find largest window closest to whole number of samples
uint8_t whole_cycle_samples(float signal_hz, uint8_t n){
	float per_cycle = 1.0f / (signal_hz * SAMPLE_PERIOD_S);   // pairs per cycle (~4.9)
	uint8_t best_n = n;
	float best_err = 1.0f;

	for(uint8_t m = (uint8_t)((n + 0.5f) / per_cycle); m > 0; m--){
		float exact = m * per_cycle;
		uint8_t nn = (uint8_t)(exact + 0.5f);
		if(nn > n){
			continue;
		}
		if(nn < (uint8_t)(3 * n / 4)){
			break;                                            // too short
		}
		float err = fabsf((float)nn - exact) / per_cycle;     // leftover, in cycles
		if(err < best_err - 0.02f){	 // Switch if better
			best_err = err;
			best_n = nn;
		}
	}
	return best_n;
}
