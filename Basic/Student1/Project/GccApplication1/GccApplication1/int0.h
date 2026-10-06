#ifndef INT0_H_
#define INT0_H_

#include <stdbool.h>
#include <stdint.h>

//Initialize int0 as per Part 1
void int0_init();

extern volatile uint16_t elapsed_ticks;
extern volatile uint8_t measurement_ready;

#endif