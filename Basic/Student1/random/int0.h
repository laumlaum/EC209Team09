#ifndef INT0_H_
#define INT0_H_

#include <stdbool.h>
#include <stdint.h>

void int0_init();

/* Set on every zero-crossing edge. adc.c clears it and waits for it to be set
 * again, that's the only thing it's used for here. */
extern volatile uint8_t zc_flag;

#endif