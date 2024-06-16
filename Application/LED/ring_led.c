/*
 * ring_led.c
 *
 * Created: 5/28/2024 5:14:19 PM
 *  Author: HTSANG
 */ 

#include "ring_led.h"
#include <string.h>
#include "SK6812.h"
#include "scheduler.h"


/* Private define ------------------------------------------------------------*/

/* Private function ----------------------------------------------------------*/
void ringled_update(void);

/* Private typedef -----------------------------------------------------------*/
typedef struct RingLed_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} RingLed_TaskContextTypedef;

/* Private variables ---------------------------------------------------------*/
static RingLed_TaskContextTypedef           s_ringled_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		30,                                // taskPeriodInMS;
		ringled_update                // taskFunction;
	}
};

void	ringled_create_task(void)
{
	SCH_TASK_CreateTask(&s_ringled_task_context.taskHandle, &s_ringled_task_context.taskProperty);
}

void	ringled_update(void)
{
	ringled_show(colorMap, LED_COUNT);
}