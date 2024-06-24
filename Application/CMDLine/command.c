/*
 * command.c
 *
 * Created: 5/19/2024 8:37:53 PM
 *  Author: Admin
 */ 
#include "scheduler.h"
#include "command.h"
#include "uart.h"
#include "temperature.h"
#include <stdlib.h>
#include "cmdline.h"
#include "ring_led.h"
#include "ir_led.h"
#include "Accel_Gyro.h"
#include "Pressure.h"
#include "AD.h"
#include <avr/pgmspace.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct _Command_TaskContextTypedef_
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} Command_TaskContextTypedef;


//typedef enum{OK = 0, CMDLINE_BAD_CMD, CMDLINE_TOO_MANY_ARGS, CMDLINE_TOO_FEW_ARGS, CMDLINE_INVALID_ARG} command_error_code_t;
const char  * ErrorCode[5] = {"OK\r\n", "CMDLINE_BAD_CMD\r\n", "CMDLINE_TOO_MANY_ARGS\r\n",
"CMDLINE_TOO_FEW_ARGS\r\n", "CMDLINE_INVALID_ARG\r\n" };

static	void	command_task_update(void);
tCmdLineEntry g_psCmdTable[] = {{"set_temp", Cmd_set_temp," : set desired temperature, format: set_temp channel setpoint (250 mean 25 Celcius)"},
								{"help", Cmd_help," : Display list of commands, format: help" },
								{"get_temp", Cmd_get_temp , " : Get the current temperature of TEC, format: get_temp NTC channel"},
								{"get_temp_setpoint", Cmd_get_temp_setpoint , " : Get the current temperature setpoint of TEC, format: get_temp_setpoint  channel",},
								{"tec_ena", Cmd_TEC_enable, " : enable TEC channel, format: tec_ena channel"},
								{"tec_dis", Cmd_TEC_disable, " : disable TEC channel, format: tec_dis channel"}	,
								{"tec_dis_auto", Cmd_TEC_disable_auto_control, " : disable auto control, format tec_dis_auto channel"},
								{"tec_ena_auto", Cmd_TEC_enable_auto_control, " :  enable auto control,  format: tec_ena_auto channel"},
								{"tec_set_output", Cmd_TEC_set_output, " :  set the output manually,  format: tec_set_output channel heat_cool(0:COOL, 1: HEAT) voltage(150 mean 1.5)"},
								{"tec_set_auto_voltage", Cmd_tec_set_auto_voltage, " :  set the voltage for auto control mode,  format: tec_set_auto_voltage channel  voltage	(150 mean 1.5)"},
								{"tec_get_status", Cmd_tec_get_status, " :  get tec status,  format: tec_get_status, response ntc0 ntc1 tec0_stat tec1_stat tec2_stat tec3_stat H/C0 H/C1"},
								{"tec_log_ena", Cmd_TEC_log_enable, " : enable periodic log, format: tec_log_ena"},
								{"tec_log_dis", Cmd_TEC_log_disable, " : disable periodic log, format: tec_log_ena"},
								{"ringled_set_RGBW", Cmd_ringled_set_RGBW, " : detail, format: ringled_set_RGB <R> <G> <B> <W>"},
								{"ringled_get_RGBW", Cmd_ringled_get_RGBW, " : detail, format: ringled_get_RGB"},
								{"ir_led_set_bright", Cmd_IRled_set_bright, " : set brightness (0-100%) for IR led, format: ir_led_set_bright 70"},
								{"ir_led_get_bright", Cmd_IRled_get_bright, " : get brightness (0-100%) for IR led, format: ir_led_get_bright"},
								{"get_accel_gyro", Cmd_get_acceleration_gyroscope, " : Get the current acceleration and gyroscope, format: get_accel_gyro"},
								{"get_press", Cmd_get_pressure, " : Get the current pressure, format: get_press"},
								{"get_parameters", Cmd_get_parameters, " : ... , format: get_parameters"},
								{0,0,0}
								};

volatile static	ringbuffer_t *p_CommandRingBuffer;
volatile static	char s_commandBuffer[COMMAND_MAX_LENGTH];
static uint8_t	s_commandBufferIndex = 0;


static Command_TaskContextTypedef           s_CommandTaskContext =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		10,                                // taskPeriodInMS;
		command_task_update                // taskFunction;
	}
};

static	void	command_task_update(void)
{
	char rxData;
	int8_t	ret_val;
	while (! rbuffer_empty(p_CommandRingBuffer))
	{
		rxData = rbuffer_remove(p_CommandRingBuffer);
		usart0_send_char(rxData);
		if ((rxData == '\r') || (rxData == '\n'))		//got a return or new line
		{
			if (s_commandBufferIndex > 0)		//if we got the CR or LF at the begining, discard	
			{
				s_commandBuffer[s_commandBufferIndex] = 0;
				s_commandBufferIndex++;
				ret_val = CmdLineProcess(s_commandBuffer);		
				s_commandBufferIndex = 0;		
				usart0_send_string(ErrorCode[ret_val]);
				usart0_send_string("> ");
			}	
			else usart0_send_string("\r\n> ");
		}
		else if ((rxData == 8) || (rxData == 127))	
		{
			if (s_commandBufferIndex > 0) s_commandBufferIndex--;
		}
		else
		{
			s_commandBuffer[s_commandBufferIndex] = rxData;
			s_commandBufferIndex ++;
			if (s_commandBufferIndex > COMMAND_MAX_LENGTH) s_commandBufferIndex= 0;
		}
	}	
}

void	command_init(void)
{
	usart0_init();
	p_CommandRingBuffer = uart_get_uart0_rx_buffer_address();
	memset((void *)s_commandBuffer, 0, sizeof(s_commandBuffer));
	s_commandBufferIndex = 0;
	usart0_send_string("IOU FIRMWARE V1.0.0 \r\n");
	usart0_send_string("> ");
	command_send_splash();
}

void	command_create_task(void)
{
	SCH_TASK_CreateTask(&s_CommandTaskContext.taskHandle, &s_CommandTaskContext.taskProperty);
}

int Cmd_help(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;

	usart0_send_string("\nAvailable commands\r\n");
	usart0_send_string("------------------\r\n");

	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[0];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		usart0_send_string(pEntry->pcCmd);
		usart0_send_string(pEntry->pcHelp);
		usart0_send_string("\r\n");

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}
//*****************************************************************************
//
// Format: set_temp channel value
//
//*****************************************************************************
int
Cmd_set_temp(int argc, char *argv[]) 
{
	if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
	if (argc >3) return CMDLINE_TOO_MANY_ARGS;
	//if (argv[1] > "3" || argv[1] < "0") return CMDLINE_INVALID_ARG;
	uint8_t channel = atoi(argv[1]);
	if (channel > 3 || channel < 0) return CMDLINE_INVALID_ARG;
	uint16_t setpoint = atoi(argv[2]);
	temperature_set_point(setpoint, channel);
	UARTprintf("Channel %d set point: %d \r\n",channel, setpoint);
	return CMDLINE_OK;
}

//*****************************************************************************
//
// Format: get_temp NTC	channel, get_temp onewire channel, get_temp bmp390 channel
// Get the temperature from NTC / onewire / bmp390
//
//*****************************************************************************
int
Cmd_get_temp(int argc, char *argv[])
{
	if (!strcmp(argv[1], "bmp390"))
	{
		if (argc >2) return CMDLINE_TOO_MANY_ARGS;
		char buffer[5];
		sprintf(buffer, "Temp of BMP490: %i", temperature_get_bmp390());
		usart0_send_string(buffer);
		//UARTprintf("Temp of BMP490: %s \r\n", buffer);
	}
	else
	{
		if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
		if (argc >3) return CMDLINE_TOO_MANY_ARGS;
		uint8_t channel = atoi(argv[2]);
		if (channel > (MAX_CHANNEL -1)) return CMDLINE_INVALID_ARG;
		if (!strcmp(argv[1], "NTC"))
		{
			char buffer[20];
			sprintf(buffer, "NTC[%d]: %i", channel, temperature_get_NTC(channel));
//			UARTprintf("NTC[%d]: %s \r\n", channel, buffer);
			usart0_send_array(buffer, strlen(buffer));

		
		}
		else if (!strcmp(argv[1], "onewire"))
		{
			char buffer[20];
			sprintf(buffer, "1-Wire[%d]: %i", channel, temperature_get_onewire(channel));
//			UARTprintf("1-Wire[%d]: %s \r\n", channel, buffer);
			usart0_send_array(buffer, strlen(buffer));
		}
		else return CMDLINE_INVALID_ARG;
		return CMDLINE_OK;
	}
}

//*****************************************************************************
//
// Format: tec_set_auto_voltage channel auto_voltage
//
//*****************************************************************************
int
Cmd_tec_set_auto_voltage(int argc, char *argv[]) 
{
	  if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
	  if (argc >3) return CMDLINE_TOO_MANY_ARGS;
	  uint8_t channel = atoi(argv[1]);
	  if (channel > 3)	return CMDLINE_INVALID_ARG;
	  
	uint16_t voltage = atoi(argv[2]);
	temperature_set_auto_voltage( channel, voltage);
	UARTprintf("Channel %d auto voltage: %d \r\n",channel, voltage);
	return CMDLINE_OK;
}
//*****************************************************************************
//
// Format: TEC_ena channel 
//
//*****************************************************************************
int
Cmd_TEC_enable(int argc, char *argv[])
{
		  if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
		  if (argc >2) return CMDLINE_TOO_MANY_ARGS;
		  uint8_t channel = atoi(argv[1]);
		  if (channel > 3)	return CMDLINE_INVALID_ARG;
		  temperature_enable_TEC(channel);
		  return CMDLINE_OK;
}
//*****************************************************************************
//
// Format: TEC_dis channel 
//
//*****************************************************************************
int
Cmd_TEC_disable(int argc, char *argv[])
{
	if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
	if (argc >2) return CMDLINE_TOO_MANY_ARGS;
	if (!strcmp(argv[1], "a"))
	{
		for (uint8_t udx=0; udx < 3; udx++)	 temperature_disable_TEC(udx);
		UARTprintf("disabled all channel\r\n");
		return CMDLINE_OK;
	}
	uint8_t channel = atoi(argv[1]);
	if (channel > 3)	return CMDLINE_INVALID_ARG;
	temperature_disable_TEC(channel);
	return CMDLINE_OK;
}

//*****************************************************************************
//
// Format: TEC_ena_control channel 
//
//*****************************************************************************
int
Cmd_TEC_enable_auto_control(int argc, char *argv[])
{
		  if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
		  if (argc >2) return CMDLINE_TOO_MANY_ARGS;
		  uint8_t channel = atoi(argv[1]);
		  if (channel > 3)	return CMDLINE_INVALID_ARG;
		  temperature_enable_auto_control_TEC(channel << 1);
		  temperature_disable_TEC(channel*2);
		  temperature_disable_TEC(channel*2 + 1);
		  return CMDLINE_OK;
}

//*****************************************************************************
//
// Format: TEC_dis_control channel 
//
//*****************************************************************************
int
Cmd_TEC_disable_auto_control(int argc, char *argv[])
{
		  if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
		  if (argc >2) return CMDLINE_TOO_MANY_ARGS;
		  uint8_t channel = atoi(argv[1]);
		  if (channel > 3)	return CMDLINE_INVALID_ARG;
		  temperature_disable_auto_control_TEC(channel << 1);
		  
		  return CMDLINE_OK;
}

int
Cmd_tec_get_status(int argc, char *argv[])
{

	if (argc >1) return CMDLINE_TOO_MANY_ARGS;
	temperature_get_status();
	return CMDLINE_OK;
	
}

int
Cmd_TEC_log_enable(int argc, char *argv[])
{

	if (argc >1) return CMDLINE_TOO_MANY_ARGS;
	temperature_enable_log();
	return CMDLINE_OK;
	
}

int
Cmd_TEC_log_disable(int argc, char *argv[])
{

	if (argc >1) return CMDLINE_TOO_MANY_ARGS;
	temperature_disable_log();
	return CMDLINE_OK;
	
}


//*****************************************************************************
//
// Format: tec_set_output channel heat_cool(0:COOL, 1: HEAT) voltage(150 mean 1.5)
// set the TEC output manually
//
//*****************************************************************************

int
Cmd_TEC_set_output(int argc, char *argv[])
{

	if (argc < 4) return CMDLINE_TOO_FEW_ARGS;
	if (argc >4) return CMDLINE_TOO_MANY_ARGS;
	uint8_t _heatCool = atoi(argv[2]);
	uint8_t _channel = atoi(argv[1]);
	uint16_t	_voltage = atoi(argv[3]);
	if (_channel >=4 )	return CMDLINE_INVALID_ARG;
	temperature_set_TEC_output(_channel, _heatCool, _voltage);
	return CMDLINE_OK;
	 
}

int
Cmd_get_temp_setpoint(int argc, char *argv[])
{
	if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
	if (argc >2) return CMDLINE_TOO_MANY_ARGS;
	uint8_t _channel = atoi(argv[1]);
	uint16_t	_setpoint;
	if (_channel > 2)	return CMDLINE_INVALID_ARG;
	_setpoint = temperature_get_setpoint(_channel);
	UARTprintf("Setpoint channel[%d]: %d \r\n", _channel, _setpoint);
	return CMDLINE_OK;
}

int
Cmd_ringled_set_RGBW(int argc, char *argv[])
{
	if (argc < 5) return CMDLINE_TOO_FEW_ARGS;
	if (argc > 5) return CMDLINE_TOO_MANY_ARGS;
	uint8_t _red = atoi(argv[1]);
	uint8_t _green = atoi(argv[2]);
	uint8_t _blue = atoi(argv[3]);
	uint8_t _white = atoi(argv[4]);
	ringled_set_RGBW(_red, _green, _blue, _white);
	return CMDLINE_OK;
}

int
Cmd_ringled_get_RGBW(int argc, char *argv[])
{
	if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
	rgbw_color RGB = ringled_get_RGBW();
	UARTprintf("Ringled: RED = %d, GREEN = %d, BLUE = %d, WHITE = %d \r\n", RGB.red, RGB.green, RGB.blue, RGB.white);
	return CMDLINE_OK;
}

int
Cmd_IRled_set_bright(int argc, char *argv[])
{
	if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
	if (argc > 2) return CMDLINE_TOO_MANY_ARGS;
	
	uint8_t _DutyCyclePercent = atoi(argv[1]);
	if (_DutyCyclePercent > 100)
	{
		//UARTprintf("WARNING: Duty cycle ranges from 0 to 100 percent");
		_DutyCyclePercent = 100;
	}
	IR_led_set_DutyCyclesPercent(_DutyCyclePercent);
	UARTprintf("Led IR set brightness is %s %% \r\n",argv[1]);
	return CMDLINE_OK;
}

int
Cmd_IRled_get_bright(int argc, char *argv[])
{
	if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
	uint8_t Current_DutyCyclePercent = IR_led_get_Current_DutyCyclesPercent();
	UARTprintf("Led IR's brightness is %d %% \r\n",Current_DutyCyclePercent);
	return CMDLINE_OK;
}

int
Cmd_get_acceleration_gyroscope(int argc, char *argv[])
{
	if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
	Accel_Gyro_DataTypedef _accel_data = get_acceleration();
	Accel_Gyro_DataTypedef _gyro_data = get_gyroscope();
	char buffer[20];
	sprintf(buffer,"Acceleration %i %i %i \r\n", _accel_data.x, _accel_data.y, _accel_data.z);
	usart0_send_array(buffer, strlen(buffer));
	sprintf(buffer,"Gyroscope %i %i %i\r\n", _gyro_data.x, _gyro_data.y, _gyro_data.z);
	usart0_send_array(buffer, strlen(buffer));
	return CMDLINE_OK;
}

int
Cmd_get_pressure(int argc, char *argv[])
{
	if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
	uint16_t _pressure = get_pressure();
	UARTprintf("Press: %d\r\n", _pressure);
	return CMDLINE_OK;
}

int
Cmd_get_parameters(int argc, char *argv[])
{
	char buffer[20];
	for (uint8_t i = 0; i < 4; i++)
	{
		sprintf(buffer, "NTC[%d]: %i",i , temperature_get_NTC(i));
		usart0_send_array(buffer, strlen(buffer));
	}
	UARTprintf("Status TEC:");
	temperature_get_status();
	rgbw_color RGBW = ringled_get_RGBW();
	UARTprintf("RINLED: R=%d, G=%d, B=%d, W=%d\r\n", RGBW.red, RGBW.green, RGBW.blue, RGBW.white);
	UARTprintf("IRLED: duty %d %%\r\n", IR_led_get_Current_DutyCyclesPercent());
	
	// Accel and Gyro
	Accel_Gyro_DataTypedef _accel_data = get_acceleration();
	Accel_Gyro_DataTypedef _gyro_data = get_gyroscope();
	sprintf(buffer,"Acceleration %i %i %i \r\n", _accel_data.x, _accel_data.y, _accel_data.z);
	usart0_send_array(buffer, strlen(buffer));
	sprintf(buffer,"Gyroscope %i %i %i\r\n", _gyro_data.x, _gyro_data.y, _gyro_data.z);
	usart0_send_array(buffer, strlen(buffer));
	
	// Pressure
	uint16_t _pressure_data = get_pressure();
	UARTprintf("Press: %d\r\n", _pressure_data);
	
	return CMDLINE_OK;
}



const char SPLASH[][65] PROGMEM = {
	{".......................................................\r\n"},
	{".......................................................\r\n"},
	{"..                                                   ..\r\n"},
	{"..                                                   ..\r\n"},
	{"..              _____                                ..\r\n"},
	{"..             / ____|                               ..\r\n"},
	{"..            | (___  _ __   __ _  ___ ___           ..\r\n"},
	{"..             \\___ \\| '_ \\ / _` |/ __/ _ \\          ..\r\n"},
	{"..             ____) | |_) | (_| | (_|  __/          ..\r\n"},
	{"..            |_____/| .__/ \\__,_|\\___\\___|          ..\r\n"},
	{"..         _      _ _| | _______        _            ..\r\n"},
	{"..        | |    (_|_)_||__   __|      | |           ..\r\n"},
	{"..        | |     _ _ _ __ | | ___  ___| |__         ..\r\n"},
	{"..        | |    | | | '_ \\| |/ _ \\/ __| '_ \\        ..\r\n"},
	{"..        | |____| | | | | | |  __/ (__| | | |       ..\r\n"},
	{"..        |______|_|_|_| |_|_|\\___|\\___|_| |_|       ..\r\n"},
	{"..                                                   ..\r\n"},
	{"..                                                   ..\r\n"},
	{"..    _____ ____  _    _  __    ____   ___   ___     ..\r\n"},
	{"..   |_   _/ __ \\| |  | | \\ \\  / /_ | / _ \\ / _ \\    ..\r\n"},
	{"..     | || |  | | |  | |  \\ \\/ / | || | | | | | |   ..\r\n"},
	{"..    _| || |__| | |__| |   \\  /  | || |_| | |_| |   ..\r\n"},
	{"..   |_____\\____/ \\____/     \\/   |_(_)___(_)___/    ..\r\n"},
	{"..                                                   ..\r\n"},
	{"..                                                   ..\r\n"},
	{".......................................................\r\n"},
	{".......................................................\r\n"},
	{"\r\n"}
};

void	command_send_splash(void)
{
	for(uint8_t i = 0 ; i < 28 ; i++) 
		usart0_send_string_P(&SPLASH[i][0]);
	usart0_send_string("> ");
}