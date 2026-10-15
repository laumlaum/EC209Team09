/*
 * GccApplication2.c
 *
 * Created: 17/09/2026 5:33:04 PM
 * Author : cche632
 *
 */

#include "common.h"
#include "timer0.h"
#include "timer2.h"
#include "adc.h"
#include "int0.h"
#include "timer1.h"
#include "uart.h"
#include "calc.h"
#include "display.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint8_t flag;
extern volatile uint16_t adc0[SAMPLES], adc1[SAMPLES];
extern volatile uint8_t period_ready;
extern volatile uint32_t elapsed_ticks;


int main(void){
	/* ---------------- Init ---------------- */
	DDRB |= (1<<PINB5);          // EoC debug pin / LED (was DDRB = 0xFF: don't drive pins we don't use)
	DDRD &= ~(1<<PIND2);         // INT0 input from the zero-crossing detector

	display_init();
	//timer0_init();             // frequency measurement - Timer0 is free for this (display uses Timer2)
	timer2_init();
	adc_init();
	timer1_init();
	uint16_t ubrr = 12;
	usart_init(ubrr);
	//int0_init();
	sei();

	uint8_t first = 1;

	/* totals for the current second */
	float vrms_sum = 0, ipk_sum = 0, p_sum = 0;
	uint8_t windows = 0;

	float energy_j = 0;          // energy in joules since power-up
	uint8_t page = 0;            // display page: 0 V, 1 I, 2 P, 3 E
	uint8_t page_ticks = 0;
	#define PAGE_SECONDS 3

	while(1){

		/* ---------------- Wait -> Process -> Re-arm ---------------- */
		if(flag){
			if(first){                    // first window after power-up: throw it away
				first = 0;
				adc_restart();
				continue;
			}
			// flag == 1 means the ISR has stopped, so adc0/adc1 are safe to read
			// Use only a whole number of signal cycles (see calc.c): 98 of the
			// 100 pairs at 510 Hz, all 100 at 500 Hz.
			float   hz = estimate_hz(adc0, SAMPLES);
			uint8_t n  = whole_cycle_samples(hz, SAMPLES);

			vrms_sum += calc_vrms(adc0, n);                       // Gets Voltage RMS in mV
			ipk_sum  += get_ipeak(adc1, n);                       // gets peak current in mA
			p_sum    += calculate_power(adc0, adc1, n, hz);       // gets power in W
			windows++;

			adc_restart();                // Re-arm: start the next window
		}

		/* ---------------- Report (once per second) ---------------- */
		if(one_second_flag && windows > 0){
			one_second_flag = 0;

	
			float vrms = vrms_sum / windows;    // Gets Voltage RMS in mV
			float ipk  = ipk_sum  / windows;    // gets peak current in mA
			float p    = p_sum    / windows;    // gets power in mW
			vrms_sum = 0; ipk_sum = 0; p_sum = 0; windows = 0;

			energy_j += p;                      // W x 1 s = J

			print_line("V_rms: ", vrms, " mV\r\n");
			print_line("Ipk: ", ipk, " mA\r\n");
			print_line("P: ", p * 1000.0f, " mW\r\n");
			print_line("E: ", energy_j, " J\r\n");

			switch(page){                       // display takes V, A, W
			case 0:  display_show_voltage(vrms / 1000.0f); break;
			case 1:  display_show_current(ipk / 1000.0f);  break;
			case 2:  display_show_power(p);                break;
			default: display_show_voltage(vrms / 1000.0f);        break;
			}
			if(++page_ticks >= PAGE_SECONDS){
				page_ticks = 0;
				page = (page +1) & 0x03;
			}
		}
	}
}