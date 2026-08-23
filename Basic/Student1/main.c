/*
 * GccApplication1.c
 *
 * Created: 30/07/2026 5:20:53 PM
 * Author : cche632
 */
#define F_CPU 2000000UL
#define RMSVoltage 14.5
#define PeakCurrent 125
#define Power 1.60
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart.h"




int main(void)
{
	/* Replace with your application code */


	uint16_t ubrr = 12;
	usart_init(ubrr);

	while (1)
	{
		/* Scales RMS voltage in order to extract every digit*/
		uint16_t scaled = (uint16_t)(RMSVoltage * 10 + 0.5);
		
		/* Extracts each digit of RMS Voltage*/
		char RMShundreds = (scaled / 100) + 48;
		char RMStens = ((scaled / 10) % 10) + 48;
		char RMSones = (scaled % 10) + 48;
		/* Transmits data*/
		usart_transmit_string("RMS Voltage is: ");
		usart_transmit(RMShundreds);
		usart_transmit(RMStens);
		usart_transmit(46);
		usart_transmit(RMSones);
		usart_transmit('\n');
		
		/* Extracts each digit of Peak Current*/
		char hundreds = (PeakCurrent / 100) + 48;
		char tens = ((PeakCurrent / 10) % 10) + 48;
		char ones = (PeakCurrent % 10) + 48;
		/* Transmits data*/
		usart_transmit_string("Peak Current is: ");
		usart_transmit(hundreds);
		usart_transmit(tens);
		usart_transmit(ones);
		usart_transmit('\n');
		
		/* Scales Power in order to extract every digit*/
		uint16_t PowerScaled = (uint16_t)(Power * 100 + 0.5);
		/* Extracts each digit of Power*/
		char Phundreds = (PowerScaled / 100) + 48;
		char Ptens = ((PowerScaled / 10) % 10) + 48;
		char Pones = (PowerScaled % 10) + 48;
		/* Transmits data*/
		usart_transmit_string("Power is: ");
		usart_transmit(Phundreds);
		usart_transmit(46);
		usart_transmit(Ptens);
		usart_transmit(Pones);
		usart_transmit('\n');
		
		/* Delay of 1 second*/
		_delay_ms(1000);
	}
}

