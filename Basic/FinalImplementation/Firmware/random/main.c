/*
 * GccApplication2.c
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

#define MERGED_SAMPLES (2 * SAMPLES)

static uint16_t va[SAMPLES];               /* snapshot of the phase-0 V window */
static uint16_t ia[SAMPLES];               /* snapshot of the phase-0 I window */
static uint16_t v_merged[MERGED_SAMPLES];  /* Va/Vb interleaved, 100 us spacing */
static uint16_t i_merged[MERGED_SAMPLES];  /* Ia/Ib interleaved, 100 us spacing */

int main(void){
	DDRB |= (1<<PINB5);
	DDRD &= ~(1<<PIND2);         // INT0 input from the zero-crossing detector

	display_init();
	timer2_init();
	adc_init();
	timer1_init();
	uint16_t ubrr = 12;
	usart_init(ubrr);
	int0_init();
	sei();

	uint8_t first = 1;
	uint8_t phase = 0;      // 0 = Va, 1 = Ia, 2 = Vb (+100us), 3 = Ib (+100us)

	float vrms_sum = 0, ipk_sum = 0, p_sum = 0;
	uint8_t groups = 0;     // one full Va,Ia,Vb,Ib group = one vrms/ipk/p estimate

	float energy_j = 0;
	uint8_t page = 0;
	uint8_t page_ticks = 0;
	#define PAGE_SECONDS 3

	// Power is a product of two noisy quantities, so even though each
	// second already averages over ~20 Va,Ia,Vb,Ib groups, second-to-second
	// power still jitters more than Vrms or Ipeak do. p_smooth is a slow
	// exponential moving average applied only to what gets shown/printed,
	// so the 3 numbers that scroll past while the display is on the power
	// page stay close together instead of bouncing around, which is what
	// was making them hard to average by eye. energy_j still integrates the
	// raw, unsmoothed p each second, so the smoothing doesn't bias energy.
	float p_smooth = 0.0f;
	uint8_t p_smooth_init = 0;
	#define POWER_SMOOTH_ALPHA 0.25f   /* lower = smoother but slower to settle */

	adc_restart(0, 0);   // Va

	while(1){

		if(flag){
			if(first){
				first = 0;
				adc_restart(0, 0);
				continue;
			}

			switch(phase){
			case 0: {
				// Va just finished: save it before Vb overwrites adc0.
				for(uint8_t k = 0; k < SAMPLES; k++) va[k] = adc0[k];
				phase = 1;
				adc_restart(1, 0);   // Ia
				break;
			}
			case 1: {
				// Ia just finished: save it before Ib overwrites adc1.
				for(uint8_t k = 0; k < SAMPLES; k++) ia[k] = adc1[k];
				phase = 2;
				adc_restart(0, 1);   // Vb, grid nudged 100 us later
				break;
			}
			case 2: {
				// Vb just finished. adc0 holds Vb; Va is in va[]. Interleave
				// by elapsed time since each window's own zero-crossing edge:
				// Va[k] is nominally k*200us, Vb[k] is k*200us + 100us, so
				// sorted by that elapsed time Va[k] comes before Vb[k] and
				// after Va[k-1] but before Va[k+1].
				for(uint8_t k = 0; k < SAMPLES; k++){
					v_merged[2*k]     = va[k];
					v_merged[2*k + 1] = adc0[k];
				}
				phase = 3;
				adc_restart(1, 1);   // Ib, grid nudged 100 us later
				break;
			}
			default: {
				// phase == 3: Ib just finished. adc1 holds Ib; Ia is in ia[].
				for(uint8_t k = 0; k < SAMPLES; k++){
					i_merged[2*k]     = ia[k];
					i_merged[2*k + 1] = adc1[k];
				}

				float hz  = estimate_hz(v_merged, MERGED_SAMPLES);
				uint8_t n = whole_cycle_samples(hz, MERGED_SAMPLES);

				vrms_sum += calc_vrms(v_merged, n);
				ipk_sum  += get_ipeak(i_merged, n);
				p_sum    += calculate_power_interleaved(v_merged, i_merged, n);
				groups++;

				phase = 0;
				adc_restart(0, 0);   // next group's Va
				break;
			}
			}
		}

		if(one_second_flag && groups > 0){
			one_second_flag = 0;

			float vrms = vrms_sum / groups;
			float ipk  = ipk_sum  / groups;
			float p    = p_sum    / groups;
			vrms_sum = 0; ipk_sum = 0; p_sum = 0; groups = 0;

			energy_j += p;   // energy integrates the raw, unsmoothed power

			if(!p_smooth_init){
				p_smooth = p;          // start at the real value, not a ramp from 0
				p_smooth_init = 1;
			} else {
				p_smooth += POWER_SMOOTH_ALPHA * (p - p_smooth);
			}

			print_line("V_rms: ", vrms, " mV\r\n");
			print_line("Ipk: ", ipk, " mA\r\n");
			print_line("P: ", p_smooth * 1000.0f, " mW\r\n");
			print_line("E: ", energy_j, " J\r\n");

			switch(page){
			case 0:  display_show_voltage(vrms / 1000.0f); break;
			case 1:  display_show_current(ipk / 1000.0f);  break;
			case 2:  display_show_power(p_smooth);         break;
			default: display_show_voltage(vrms / 1000.0f); break;
			}
			if(++page_ticks >= PAGE_SECONDS){
				page_ticks = 0;
				page = (page + 1) & 0x03;
			}
		}
	}
}