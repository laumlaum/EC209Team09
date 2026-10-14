#ifndef _ADC_H
#define _ADC_H
#include <stdint.h>


void adc_init();
void adc_restart();
uint16_t adc_read(uint8_t chan);
uint16_t adc_convert_mv(uint16_t value);

#endif
