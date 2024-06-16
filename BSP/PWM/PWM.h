/*
 * PWM.h
 *
 * Created: 6/13/2024 3:59:41 PM
 *  Author: HTSANG
 */ 


#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>

void pwm1_init(const uint8_t _Pin, const uint16_t _TimePeriodUs, const uint8_t _DutyCyclePercent);
void pwm2_init(const uint8_t _Pin, const uint16_t _TimePeriodUs, const uint8_t _DutyCyclePercent);
void pwm1_set_DutyCyclePercent(const uint8_t _DutyCyclePercent);
void pwm2_set_DutyCyclePercent(const uint8_t _DutyCyclePercent);

#endif /* PWM_H_ */