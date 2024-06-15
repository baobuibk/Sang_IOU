/*
 * Pressure.c
 *
 * Created: 6/14/2024 1:01:26 PM
 *  Author: HTSANG
 */ 

#include "Pressure.h"

/* Private define ------------------------------------------------------------*/
/* Private function ----------------------------------------------------------*/
void pressure_update(void);

/* Private typedef -----------------------------------------------------------*/
typedef struct _BMP390_TaskContextTypedef_
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} BMP390_TaskContextTypedef;

/* Private variables ---------------------------------------------------------*/
static BMP390_TaskContextTypedef           s_pressure_task_context =
{
	SCH_INVALID_TASK_HANDLE,                // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		3000,								// taskPeriodInMS;
		pressure_update						// taskFunction;
	}
};


void pressure_update(void)
{
	BMP390_temp_press_update(&TempPress_data);
}

void Pressure_init(void)
{
	BMP390_init(&TempPress_data);
}

void Pressure_create_task(void)
{
	SCH_TASK_CreateTask(&s_pressure_task_context.taskHandle, &s_pressure_task_context.taskProperty);
}