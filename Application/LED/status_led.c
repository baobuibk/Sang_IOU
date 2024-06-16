/*
 * status_led.c
 *
 * Created: 5/15/2024 2:22:51 PM
 *  Author: Admin
 */ 
#include "status_led.h"
#include "scheduler.h"
#include "IOU_board.h"

/* Private define ------------------------------------------------------------*/
#define	POWERUP_PERIOD	500
#define	POWER_NORMAL_OFF_PERIOD	3000
#define	POWER_NORMAL_ON_PERIOD	500
void	status_led_update(void);
static	void	status_led_normal(void);
/* Private typedef -----------------------------------------------------------*/
typedef struct Led_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} Led_TaskContextTypedef;

typedef	struct StatusLed_CurrentStateTypedef
{
	uint8_t				led1:1;
	uint8_t				led2:1;
	IOU_StateTypedef_t *			s_pIOU_Status ;
	}StatusLed_CurrentStateTypedef_t;
	
/* Private variables ---------------------------------------------------------*/

static StatusLed_CurrentStateTypedef_t	s_led_display_status;
static Led_TaskContextTypedef           s_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		50,                                // taskPeriodInMS;
		status_led_update                // taskFunction;
	}
};

/* Private function prototypes -----------------------------------------------*/

static void status_led_led1_on(void);
static void status_led_led1_off(void);
static void status_led_led2_on(void);
static void status_led_led2_off(void);
static	void	status_led_powerup(void);


void status_led_init(void)
{
	LED1_DIR_PORT |= (1 << LED1_PIN);
	LED1_DATA_PORT &= ~((1 << LED1_PIN));
	LED2_DIR_PORT |= (1 << LED2_PIN);
	LED2_DATA_PORT &= ~((1 << LED2_PIN));
	s_led_display_status.led1 = 0;
	s_led_display_status.led2 = 0;
	s_led_display_status.s_pIOU_Status = &IOU_data.IOU_State_Data;
	status_led_led1_off();
	status_led_led2_off();
	
}

static void status_led_led1_off(void)
{
	LED1_DATA_PORT |= ((1 << LED1_PIN));	
}

static void status_led_led1_on(void)
{
	LED1_DATA_PORT &= ~((1 << LED1_PIN));
}

static void status_led_led2_off(void)
{
	LED2_DATA_PORT |= ((1 << LED2_PIN));
}

static void status_led_led2_on(void)
{
	LED2_DATA_PORT &= ~((1 << LED2_PIN));
}
void	status_led_update(void)
{
	switch (*s_led_display_status.s_pIOU_Status) {
	case IOU_POWERUP:
		status_led_powerup();
		break;
	case IOU_NORMAL:
		status_led_normal();
		break;
	default:
		break;
	}
}
static	void	status_led_powerup(void)
{
		{
			if ((s_led_display_status.led1 == 1) && (s_led_display_status.led1 == 1))	//both ON
			{
				if (SCH_TIM_HasCompleted(SCH_TIM_LED))
				{
					s_led_display_status.led1 = 0;
					s_led_display_status.led1 = 0;
					status_led_led1_off();
					status_led_led2_off();
					SCH_TIM_Start(SCH_TIM_LED,POWERUP_PERIOD);	//restart
				}
			}
			else if ((s_led_display_status.led1 == 0) && (s_led_display_status.led1 == 0))	//both OFF
			{
				if (SCH_TIM_HasCompleted(SCH_TIM_LED))
				{
					s_led_display_status.led1 = 1;
					s_led_display_status.led1 = 1;
					status_led_led1_on();
					status_led_led2_on();
					SCH_TIM_Start(SCH_TIM_LED,POWERUP_PERIOD);	//restart					
					
				}

			}
			else
			{
				s_led_display_status.led1 = 0;
				s_led_display_status.led1 = 0;
				status_led_led1_off();
				status_led_led2_off();
				SCH_TIM_Start(SCH_TIM_LED,POWERUP_PERIOD);	//restart
			}
		}
}

static	void	status_led_normal(void)
{
	if (s_led_display_status.led1 == 1)
	{
		if (SCH_TIM_HasCompleted(SCH_TIM_LED))
			{
				s_led_display_status.led1 = 0;
				status_led_led1_off();
				SCH_TIM_Start(SCH_TIM_LED,POWER_NORMAL_OFF_PERIOD);	//restart
			}		
	}
	else if (s_led_display_status.led1 == 1)
	{
		if (SCH_TIM_HasCompleted(SCH_TIM_LED))
		{
			s_led_display_status.led1 = 1;
			status_led_led1_on();
			SCH_TIM_Start(SCH_TIM_LED,POWER_NORMAL_ON_PERIOD);	//restart
		}
	}
}

void	status_led_create_task(void)
{
	SCH_TASK_CreateTask(&s_task_context.taskHandle, &s_task_context.taskProperty);
}