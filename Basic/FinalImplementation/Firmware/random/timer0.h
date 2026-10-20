/*
 * timer0.h
 *
 * Created: 17/09/2026 5:34:27 PM
 *  Author: cche632
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_

#include <stdbool.h>
#include <stdint.h>

extern volatile uint16_t timer0counter;

//Initialize timer0 as per Part 1
void timer0_init();


#endif
