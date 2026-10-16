/*
 * 7-seg.c
 *
 * Created: 14/10/2026 1:32:59 pm
 * Author : johnn
 */ 

#include "common.h"
#include "display.h"
#include "timer0.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	display_init();
	timer0_init();
	sei();

	uint8_t page = 0;

	while (1) {
		if (one_second_flag) {
			one_second_flag = 0;

			switch (page) {
				case 0:  format_7seg(145, 1, SEG_U);        break;   /* 14.5U */
				case 1:  format_7seg(532, DP_NONE, SEG_A);  break;   /* 532A  */
				case 2:  format_7seg(521, 0, SEG_P);        break;   /* 5.21P */
				case 3:  format_7seg(75, 0, SEG_P);         break;   /* 0.75P */
				default: format_7seg(85, DP_NONE, SEG_A);   break;   /*  85A  */
			}
			page = (page + 1) % 5;
		}
	}
}
