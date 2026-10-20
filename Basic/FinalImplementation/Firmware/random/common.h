#ifndef COMMON_H_
#define COMMON_H_

#ifndef F_CPU
#define F_CPU 2000000UL
#endif

/* ---- Sampling (must match timer1.c: OCR1A = OCR1B = 199) ----
 * Native ADC trigger every 100 us, one channel per window (see adc.c). The
 * phase-0/phase-1 offset trick in adc_restart() interleaves two windows of
 * the same channel 50 us apart, so the merged V/I arrays used in calc.c end
 * up 50 us apart, not 100 us (see SAMPLE_PERIOD_S there).                   */
#define TRIG_PERIOD_S  100.0e-6f

/* Signal frequency: 500 for the real source, 50 for 50 Hz bench tests.
 * Used as a fallback in estimate_hz() if it can't find enough zero
 * crossings in a window.                                                   */
#define SIGNAL_HZ      500.0f

/* Samples per single-channel, single-phase window (see adc.c/main.c). Two
 * phases of this get merged into a 2*SAMPLES array at half the spacing. */
#define SAMPLES 100

/* 1 = also print every raw sample (BEGIN / k,adc0,adc1 / END) for Excel.
 * Slow (about 2 s per dump at 9600 baud), so 0 for normal use.             */
#define DUMP_RAW 0

#endif