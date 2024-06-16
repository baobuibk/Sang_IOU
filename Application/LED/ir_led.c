/*
 * ir_led.c
 *
 * Created: 6/13/2024 4:03:38 PM
 *  Author: HTSANG
 */ 

#include "ir_led.h"
#include "IOU_board.h"
#include "scheduler.h"
#include "PWM.h"

/* Private define ------------------------------------------------------------*/
#define IR_Led_CurrentDutyCyclePercentTypedef_t		uint8_t

/* Private variables ---------------------------------------------------------*/
static IR_Led_CurrentDutyCyclePercentTypedef_t		IR_led_DutyCycle_status = 0;

void IR_led_init(void)
{
	#ifdef	IOU_HW_V1_0_0
		pwm1_init(IR_LED_PIN, 1000, 0);
	#endif
	
	#ifdef	IOU_HW_V1_2_0
		pwm2_init(IR_LED_PIN, 1000, 0);
	#endif
}

void IR_led_set_DutyCyclesPercent(uint8_t _DutyCyclesPercent)
{
	if (_DutyCyclesPercent > 100) _DutyCyclesPercent = 100;
	#ifdef	IOU_HW_V1_0_0
		pwm1_set_DutyCyclePercent(_DutyCyclesPercent);
	#endif
		
	#ifdef	IOU_HW_V1_2_0
		pwm2_set_DutyCyclePercent(_DutyCyclesPercent);
	#endif
	
	IR_led_DutyCycle_status = _DutyCyclesPercent;
}

uint8_t IR_led_get_Current_DutyCyclesPercent(void)
{
	return (uint8_t)IR_led_DutyCycle_status;
}