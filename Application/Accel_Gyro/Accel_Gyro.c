/*
 * Accel_Gyro.c
 *
 * Created: 6/14/2024 12:27:07 AM
 *  Author: HTSANG
 */ 

#include "Accel_Gyro.h"
#include "i2c.h"
#include "math.h"
#include "twi.h"
#include "uart.h"
#include "scheduler.h"
#include "IOU_board.h"

/* Private define ------------------------------------------------------------*/
/* Private function ----------------------------------------------------------*/
void accel_gyro_update(void);

/* Private typedef -----------------------------------------------------------*/
typedef struct LSM6DSOX_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} LSM6DSOX_TaskContextTypedef;

/* Private variables ---------------------------------------------------------*/
static LSM6DSOX_TaskContextTypedef           s_accel_gyro_task_context =
{
	SCH_INVALID_TASK_HANDLE,                // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,									// taskPeriodInMS;
		accel_gyro_update					// taskFunction;
	}
};

Accel_Gyro_DataTypedef Accel_Status;
Accel_Gyro_DataTypedef Gyro_Status;

void accel_gyro_update(void)
{
	read_accel(&Accel_Status);
	read_gyro(&Gyro_Status);
}

bool Accel_and_Gyro_init(void)
{
	if (!lsm6dsox_init())
	{
		usart0_send_string("LSM6DSOX is unavailble \r\n> ");
		return false;
	}
	return true;
}

void Accel_and_Gyro_create_task(void)
{
	SCH_TASK_CreateTask(&s_accel_gyro_task_context.taskHandle, &s_accel_gyro_task_context.taskProperty);
}

Accel_Gyro_DataTypedef get_acceleration(void)
{
	return (Accel_Gyro_DataTypedef)Accel_Status;
}

Accel_Gyro_DataTypedef get_gyroscope(void)
{
	return (Accel_Gyro_DataTypedef)Gyro_Status;
}