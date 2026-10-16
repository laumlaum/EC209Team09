#include "common.h"
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t channel = 0;
volatile uint16_t adc0[SAMPLES], adc1[SAMPLES];
volatile uint8_t counter= 0;
volatile uint8_t flag = 0;
volatile uint8_t adc_index = 0;
volatile uint8_t skip;

ISR(ADC_vect){
	// Clears the flag
	TIFR1 = (1 << OCF1B);                 
	                 
	// Switches sampling channel				                    
	uint8_t ch = channel;                
	channel = ch ^ 1;
	ADMUX = (1 << REFS0) | channel;  
	// Stores ADC into result
	uint16_t result = ADC;
	// Toggles EoC
	PINB = (1<<PINB5);                    

	// If adc full, leave
	if(flag) return;
	// Skips first couple samples to allow for ADC to settle
	if(skip){ skip--; return; }

	if(ch == 0){
		adc0[adc_index] = result;		  
		} else {
		adc1[adc_index] = result;
		counter++;
		adc_index++;
	}

	// Checks whether adc arrays are full
	if(counter >= SAMPLES){
		flag = 1;
		adc_index = 0;
		ADCSRA &= ~(1 << ADATE);
	}
}

void adc_init(){
	ADMUX = 0b01000000;
	ADCSRA = 0b10101100;   // ADEN, ADATE, ADIE, /16 (ADSC removed: stray first conversion)
	ADCSRB |= (1<<ADTS2) | (1<<ADTS0);
	DIDR0 = 0x00;
}

void adc_restart(){
	counter = 0;
	adc_index = 0;
	channel = 0;
	skip = 2;
	ADMUX = (1 << REFS0);
	flag = 0;
	ADCSRA |= (1 << ADATE);    // enable auto-trigger FIRST,
	TIFR1 = (1 << OCF1B);      // then clear the flag so the next compare match is a fresh edge
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
