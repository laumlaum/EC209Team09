/*
 * GccApplication2.c
 *
 * Created: 17/09/2026 5:33:04 PM
 * Author : cche632
 *
 * main(): state machine from our partitioning diagram
 *   Init    - set up ADC, timers, UART, display
 *   Wait    - until the ADC ISR sets flag (one 40 ms window captured)
 *   Process - calc_vrms / get_ipeak / calculate_power, add to this second's totals
 *   Re-arm  - adc_restart() straight away, so the next window is captured
 *             while we wait for the report
 *   Report  - once per second (one_second_flag from Timer2): average the
 *             windows, add to energy, send UART, show the next display page
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

static void print_line(char *name, float value, char *unit){
	usart_transmit_string(name);
	print_int((int32_t)(value + 0.5f));      // rounded
	usart_transmit_string(unit);
}

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
#if DUMP_RAW
			usart_transmit_string("BEGIN\r\n");
			usart_transmit_string("k,adc0,adc1\r\n");
			for (uint8_t i = 0; i < SAMPLES; i++) {
				print_int(i);        usart_transmit_string(",");
				print_int(adc0[i]);  usart_transmit_string(",");
				print_int(adc1[i]);  usart_transmit_string("\r\n");
			}
			usart_transmit_string("END\r\n");
#endif
			vrms_sum += calc_vrms(adc0, SAMPLES);                 // mV
			ipk_sum  += get_ipeak(adc1, SAMPLES);                 // mA
			p_sum    += calculate_power(adc0, adc1, SAMPLES);     // W
			windows++;

			adc_restart();                // Re-arm: start the next window
		}

		/* ---------------- Report (once per second) ---------------- */
		if(one_second_flag && windows > 0){
			one_second_flag = 0;

			float vrms = vrms_sum / windows;    // mV
			float ipk  = ipk_sum  / windows;    // mA
			float p    = p_sum    / windows;    // W
			vrms_sum = 0; ipk_sum = 0; p_sum = 0; windows = 0;

			energy_j += p;                      // W x 1 s = J

			print_line("V_rms: ", vrms, " mV\r\n");
			print_line("Ipk: ", ipk, " mA\r\n");
			print_line("P: ", p * 1000.0f, " mW\r\n");
			print_line("E: ", energy_j, " J\r\n");

			switch(page){                       // display takes V, A, W, J
			case 0:  display_show_voltage(vrms / 1000.0f); break;
			case 1:  display_show_current(ipk / 1000.0f);  break;
			case 2:  display_show_power(p);                break;
			default: display_show_voltage(vrms / 1000.0f);        break;
			}
			if(++page_ticks >= PAGE_SECONDS){
				page_ticks = 0;
				page = (page +1) & 0x03;
			}
			/*page = (page + 1) & 0x03;*/

// 			if(period_ready){
// 				uint32_t ticks;
// 				cli();
// 				ticks = elapsed_ticks;
// 				period_ready = 0;
// 				sei();
// // 				if (ticks > 0)
// // 				{
// // 					float frequency = 250000.0f / (float)ticks;
// //
// // 					usart_transmit_string("Period ticks: ");
// // 					print_int(ticks);
// // 					usart_transmit_string(", Frequency: ");
// // 					print_int(frequency);
// // 					usart_transmit_string(" Hz\r\n");
// // 				}

// 			}
		}
	}
}
