#include "common.h"
#include "adc.h"
#include "int0.h"
#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t channel = 0;
volatile uint16_t adc0[SAMPLES], adc1[SAMPLES];
volatile uint8_t counter = 0;
volatile uint8_t flag = 0;
volatile uint8_t adc_index = 0;
volatile uint8_t skip;

ISR(ADC_vect){
	// Clears the flag
	TIFR1 = (1 << OCF1B);

	uint16_t result = ADC;   // channel doesn't change mid window
	// Toggles EoC
	PINB = (1<<PINB5);

	// If adc full, leave
	if(flag) return;
	// Skips first couple samples to allow for ADC to settle
	if(skip){ skip--; return; }

	if(channel == 0) adc0[adc_index] = result; else adc1[adc_index] = result;
	adc_index++;
	counter++;

	// Checks whether adc array is full
	if(counter >= SAMPLES){
		flag = 1;
		adc_index = 0;
		ADCSRA &= ~(1 << ADATE);
	}
}

void adc_init(){
	ADMUX = 0b01000000; // Set AVcc voltage reference
	ADCSRA = 0b10101011;  // Enables ADEN, ADATE, ADIE, and sets prescaler to /8
	ADCSRB |= (1<<ADTS2) | (1<<ADTS0); // Timer 1 compare match enable
	DIDR0 = 0x00;
}

void adc_restart(uint8_t window, uint8_t phase){
	counter = 0;
	adc_index = 0;
	channel = window;      

	
	zc_flag = 0;
	EIFR = (1<<INTF0);     // clear any edge that arrived while INT0 was off
	EIMSK |= (1<<INT0);
	uint16_t timeout = 0;
	while(!zc_flag && ++timeout < 60000U){
		;
	}
	EIMSK &= ~(1<<INT0);   // off again: nothing may interrupt the acquisition below

	// Phase-shift the sampling grid. TCNT1 free-runs the rest of the time,
	// so forcing it here to either 0 or 100 is what actually sets whether
	// this window's samples land on the phase-0 grid or exactly half a tick
	// (50 us, half of the 100 us native period from timer1.c) into it. Done
	// with interrupts off: a 16-bit TCNT1 write is two 8-bit bus writes, and
	// an ISR landing in between would smear it.
	uint8_t sreg = SREG;
	cli();
	TCNT1 = phase ? 100 : 0;
	SREG = sreg;
	TIFR1 = (1 << OCF1B);  // a flag that latched during the jump is stale, clear it

	skip = 2;
	ADMUX = (1 << REFS0) | channel;
	flag = 0;
	ADCSRA |= (1 << ADATE);     // enable auto-trigger FIRST,
	TIFR1 = (1 << OCF1B);       // then clear the flag so the next compare match is a fresh edge
}

uint16_t adc_read(uint8_t chan){
	ADMUX &= 0xF0;
	ADMUX |= chan;
	ADCSRA |= (1<<ADSC);
	while(!(ADCSRA & (1<<ADIF))){

	}
	ADCSRA |= (1<<ADIF);
	uint16_t ADCResult = ADCL | (ADCH <<8);
	return ADCResult;
}

uint16_t adc_convert_mv(uint16_t value) {
	uint32_t voltage = ((uint32_t)value * 5000) / 1024;

	return (uint16_t)voltage;
}
