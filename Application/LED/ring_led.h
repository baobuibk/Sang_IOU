/*
 * ringled.h
 *
 * Created: 5/28/2024 5:11:24 PM
 *  Author: HTSANG
 */ 


#ifndef RING_LED_H_
#define RING_LED_H_

#include "SK6812.h"

void ringled_create_task(void);
extern void ringled_set_RGBW(uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
extern rgbw_color ringled_get_RGBW(void);

#endif /* RINGLED_H_ */