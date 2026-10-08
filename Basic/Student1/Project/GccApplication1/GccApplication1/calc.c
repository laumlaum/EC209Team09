/*
 * calc.c
 *
 * Created: 8/10/2026 4:46:51 pm
 *  Author: cche632
 */ 

#include "calc.h"
#include <stdint.h>

#define OFFSET  2080.0f        
#define GVS     (1.0f)         
#define GVO     (1.0f)         
#define GIS     (1.0f)         
#define GIO     (1.0f)         

float convert_voltage(uint16_t raw){
	float pin_mv = ((int32_t)raw * 5000L) / 1024;
	return (pin_mv - OFFSET) / GVS * GVO;      // mV
}

float convert_current(uint16_t raw){
	float pin_mv = ((int32_t)raw * 5000L) / 1024;
	return (pin_mv - OFFSET) / GIS * GIO;      // mA
}

