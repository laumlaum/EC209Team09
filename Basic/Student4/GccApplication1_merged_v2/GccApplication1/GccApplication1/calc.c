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
#define GIS  0.5f           /* V across shunt per A of load  */
#define GIO  2.485f         /* current amp gain              */

#define VREF_MV  5000.0f    /* measure AVCC on the board and put it here */

/* Scale factors, worked out by the compiler (no division at run time) */
#define MV_PER_COUNT  ((VREF_MV / 1024.0f) / (GVS * GVO))    /* supply mV per ADC count */
#define MA_PER_COUNT  ((VREF_MV / 1024.0f) / (GIS * GIO))    /* load mA per ADC count   */

/* The maths is done in ADC counts with the offset (mean) removed, and scaled
 * to mV / mA once at the end - same result as converting every sample, but
 * far fewer float operations (~25 ms per window instead of ~200 ms).        */

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


// /* Amplitude of the fundamental in ADC counts (single-bin DFT).
//  * Needs a whole number of cycles in the window, which you already have. */
// static float fundamental_counts(const volatile uint16_t *raw, uint8_t n){
// 	float mean = mean_raw(raw, n);
// 	float w  = 2.0f * PI_F * SIGNAL_HZ * SAMPLE_PERIOD_S;  // phase step per sample
// 	float dc = cosf(w), ds = sinf(w);
// 	float c = 1.0f, s = 0.0f;
// 	float re = 0.0f, im = 0.0f;
// 
// 	for(uint8_t k = 0; k < n; k++){
// 		float x = (float)raw[k] - mean;
// 		re += x * c;
// 		im += x * s;
// 		float c_new = c * dc - s * ds;   // rotate (c,s) by w
// 		s = s * dc + c * ds;
// 		c = c_new;
// 	}
// 	return 2.0f * sqrtf(re * re + im * im) / n;
// }

float get_ipeak(const volatile uint16_t *i_raw, uint8_t n){
	return rms_counts(i_raw, n) * MA_PER_COUNT * 1.41421356;      // mA peak 
}

float calculate_power(const volatile uint16_t *voltage, const volatile uint16_t *current, uint8_t n){
	// Averaging two samples TRIG_PERIOD_S either side shrinks a sine by
	// cos(2 pi f Ts): this puts it back. (cos 36 deg = 0.809 at 200 us, 500 Hz)
	float gain = 1.0f / cosf(2.0f * PI_F * SIGNAL_HZ * TRIG_PERIOD_S);

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
	return (sum / (2.0f * n)) * (MV_PER_COUNT * MA_PER_COUNT / 1.0e6f) * gain;
}
