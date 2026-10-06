#include "common.h"
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint8_t channel = 0;
volatile uint16_t adc0, adc1;
volatile uint8_t counter= 0;
volatile uint8_t flag = 0;

ISR(ADC_vect){
	PINB = (1<<PINB5); // Toggle pinb0 to signal end of conversion
	if(channel==0){
		adc0 = ADC;
	} else {
		adc1 = ADC;
		counter++;
	}
	channel ^= 1;
	ADMUX = (1 << REFS0) | channel;
	TIFR1 = (1 << OCF1B);
	
	if(counter >= 40){
		flag=1;
		ADCSRA &= ~(1 << ADATE);  //Stops ADC conversion
	}	
}

void adc_init(){
	ADMUX = 0b01000000;
	ADCSRA = 0b11101100;
	ADCSRB |= (1<<ADTS2) | (1<<ADTS0);
	DIDR0 = 0x00;
}

void adc_restart(){
	counter =0;
	flag =0;
	ADCSRA |= (1<<ADATE);
	TIFR1 = (1<<OCF1B);
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
