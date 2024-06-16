/*
 * sys.c
 *
 * Created: 5/16/2024 10:40:42 PM
 *  Author: Admin
 */ 
#include "sys.h"

typedef	void (*t_tpAppEventHandler) (void);
/* Private variables ---------------------------------------------------------*/


static	SYS_SystemStateTypeDef	s_SystemState = eSYS_STATE_PowerUp;
static FlagStatus	s_AppEvents[eSYS_EVENT_MAX] = 
{
	RESET,	//eSYS_EVENT_LostMainPower
	RESET	//eSYS_EVENT_ReceivedCommandFromCOPC
};

/* Private functions ---------------------------------------------------------*/
static void SYS_handle_lost_main_power_event(void);
static void SYS_handle_received_command_from_COPC_event(void);


/* Public functions ----------------------------------------------------------*/
/*******************************************************************************
  * @name   SYS_GetSystemState
  * @brief  Function returns the current state of the system
  * @param  None
  * @retval SYS_SystemStateTypedef - system state
  *****************************************************************************/
SYS_SystemStateTypeDef SYS_GetSystemState(void)
{
  return(s_SystemState);
}

/*******************************************************************************
  * @name   SYS_PostEvent
  * @brief  Function Sets the requested Application Event
  * @param  SYS_EventTypedef event - event to be posted
  * @retval None
  *****************************************************************************/
void SYS_PostEvent(SYS_EventTypeDef event)
{
  // check for the requested event
  if(event < eSYS_EVENT_MAX)
  {
    s_AppEvents[event] = SET;
  }
}

/*******************************************************************************
  * @name   SYS_HandleEvents
  * @brief  Function handles application Events
  * @param  None
  * @retval None
  *****************************************************************************/

static void	SYS_handle_events(void)
{
	const	t_tpAppEventHandler	ftpEventHandler[eSYS_EVENT_MAX] = 
	{
		SYS_handle_lost_main_power_event,
		SYS_handle_received_command_from_COPC_event
	};
	SYS_EventTypeDef	eventIndex;
	for(eventIndex = eSYS_EVENT_First;eventIndex < eSYS_EVENT_MAX;eventIndex++)
	{
		if (s_AppEvents[eventIndex] == SET)
		{
			ftpEventHandler[eventIndex]();
		}
	}
}