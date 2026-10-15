#ifndef COMMON_H_
#define COMMON_H_

#ifndef F_CPU
#define F_CPU 2000000UL
#endif

/* ---- Sampling (must match timer1.c: OCR1A = OCR1B = 399) ----
 * One ADC conversion every 200 us, alternating V, I, V, I ...
 * so each I sample is 200 us after its V sample.                           */
#define TRIG_PERIOD_S  200.0e-6f

/* Signal frequency: 500 for the real source, 50 for 50 Hz bench tests.
 * (Only used for the interpolation correction in calc.c.)                  */
#define SIGNAL_HZ      500.0f

/* 100 pairs x 400 us = 40 ms window = 20 whole cycles at 500 Hz, 2 at 50 Hz.
 * calculate_power() wraps around the end of the window, so the window MUST be
 * a whole number of cycles - keep SAMPLES x 2 x TRIG_PERIOD_S a multiple of 20 ms. */
#define SAMPLES 100

/* 1 = also print every raw sample (BEGIN / k,adc0,adc1 / END) for Excel.
 * Slow (about 2 s per dump at 9600 baud), so 0 for normal use.             */
#define DUMP_RAW 0

#endif
