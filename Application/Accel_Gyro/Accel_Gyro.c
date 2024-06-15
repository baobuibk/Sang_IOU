/*
 * Accel_Gyro.c
 *
 * Created: 6/14/2024 12:27:07 AM
 *  Author: HTSANG
 */ 

#include "Accel_Gyro.h"
#include "LSM6DSOX.h"
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
		3000,								// taskPeriodInMS;
		accel_gyro_update						// taskFunction;
	}
};

static Accel_Gyro_DataTypedef Accel_Status;
static Accel_Gyro_DataTypedef Gyro_Status;

void accel_gyro_update(void)
{
	read_accel(&Accel_Status);
	read_gyro(&Gyro_Status);
}

void Accel_and_Gyro_init(void)
{
	lsm6dsox_init();
}

void Accel_and_Gyro_create_task(void)
{
	SCH_TASK_CreateTask(&s_accel_gyro_task_context.taskHandle, &s_accel_gyro_task_context.taskProperty);
}

void get_acceleration_gyroscope(void)
{
	char buffer[50];
	sprintf(buffer,"acceleration %.2f %.2f %.2f\r\n", Accel_Status.x, Accel_Status.y, Accel_Status.z);
	usart0_send_array(buffer, strlen(buffer));
	sprintf(buffer,"gyroscope %.2f %.2f %.2f\r\n", Gyro_Status.x, Gyro_Status.y, Gyro_Status.z);
	usart0_send_array(buffer, strlen(buffer));
}