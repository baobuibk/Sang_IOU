/*
 * watchdog.c
 *
 * Created: 5/20/2024 2:05:42 PM
 *  Author: Admin
 */ 
#include "TPL5010.h"
#include "scheduler.h"
#include "watchdog.h"
#include "uart.h"

static void watchdog_pulse(void);
typedef struct __Watchdog_TimerContextTypedef__
{
	SCH_TIMER_HANDLE              timerHandle;
	SCH_TimerPropertyTypedef      timerProperty;
} __Watchdog_TimerContextTypedef_t;

static __Watchdog_TimerContextTypedef_t          s_WatchdogTimerContext =
{
	SCH_INVALID_TIMER_HANDLE,             // Will be updated by Schedular
	{
		SCH_TIMER_PERIODIC,                   // timerType;
		100,                                 // timerPeriodInMS;
		watchdog_pulse         // timerCallbackFunction;
	}
};
void watchdog_init(void)
{
	TPL5010_init();
}
static void watchdog_pulse(void)
{
	TPL5010_done_pulse ();
}

void	watchdog_create_timer(void)
{
	SCH_TIM_CreateTimer(&s_WatchdogTimerContext.timerHandle, &s_WatchdogTimerContext.timerProperty);
	SCH_TIM_RestartTimer(s_WatchdogTimerContext.timerHandle);
}