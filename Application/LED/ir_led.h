/*
 * ir_led.h
 *
 * Created: 6/13/2024 4:03:05 PM
 *  Author: HTSANG
 */ 


#ifndef IR_LED_H_
#define IR_LED_H_

#include <stdint.h>
#include <avr/io.h>

void IR_led_init(void);
void IR_led_set_DutyCyclesPercent(uint8_t _DutyCyclesPercent);
uint8_t IR_led_get_Current_DutyCyclesPercent(void);


#endif /* IR_LED_H_ */