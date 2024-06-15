/*
 * ir_led.c
 *
 * Created: 6/13/2024 4:03:38 PM
 *  Author: HTSANG
 */ 

#include "ir_led.h"
#include "scheduler.h"
#include "PWM.h"

/* Private define ------------------------------------------------------------*/
#define IR_Led_CurrentDutyCyclePercentTypedef_t		uint8_t

// /* Private function ----------------------------------------------------------*/
// void IR_led_update(void);

// /* Private typedef -----------------------------------------------------------*/
// typedef struct IR_Led_TaskContextTypedef
// {
// 	SCH_TASK_HANDLE               taskHandle;
// 	SCH_TaskPropertyTypedef       taskProperty;
// } IR_Led_TaskContextTypedef;

/* Private variables ---------------------------------------------------------*/
static IR_Led_CurrentDutyCyclePercentTypedef_t		IR_led_DutyCycle_status = 0;
// static IR_Led_TaskContextTypedef			s_IR_led_task_context =
// {
// 	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
// 	{
// 		SCH_TASK_SYNC,                      // taskType;
// 		SCH_TASK_PRIO_0,                    // taskPriority;
// 		1000,                               // taskPeriodInMS;
// 		IR_led_update                // taskFunction;
// 	}
// };
// void IR_led_create_task(void)
// {
// 	SCH_TASK_CreateTask(&s_IR_led_task_context.taskHandle, &s_IR_led_task_context.taskProperty);
// }

void IR_led_set_DutyCyclesPercent(uint8_t _DutyCyclesPercent)
{
	pwm1_set_DutyCyclePercent(_DutyCyclesPercent);
	IR_led_DutyCycle_status = _DutyCyclesPercent;
}

// void IR_led_update(void)
// {
// 	return;
// }

uint8_t IR_led_get_Current_DutyCyclesPercent(void)
{
	return (uint8_t)IR_led_DutyCycle_status;
}