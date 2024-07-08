/*
* temperature.c
*
* Created: 5/20/2024 6:03:07 AM
*  Author: Admin
*/


#include "temperature.h"

const	uint16_t Kp = 2;


typedef struct _Temperature_TaskContextTypedef_
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} Temperature_TaskContextTypedef;

int16_t		_DAC_channel0_Val;
int16_t		_DAC_channel1_Val;

static Temperature_TaskContextTypedef           s_temperature_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		temperature_task_update                // taskFunction;
	}
};

// static	Temperature_CurrentStateTypedef_t	s_Temperature_CurrentState =
// {
// 	{0,	0},		//NTC_channel_temperature[2];
// 	{0,	0},		//onewire_channel_temperature[2];
// 	0,			//i2c_sensor_temperature
// 	{250, 250},		//channel_temperature_setpoint[2] set point is 25.0
// 	{TEC_OUT_DEFAULT, TEC_OUT_DEFAULT, TEC_OUT_DEFAULT,	TEC_OUT_DEFAULT},	//TEC_output_voltage
// 	0,				//TEC_status;	// tec3_auto tec3_ena tec2_auto tec2_ena tec1_auto tec1_ena tec0_auto tec0_ena
// 	{0,	0, 0, 0},
// 	0,
// 	{HEATING, HEATING}
// };

static	Temperature_CurrentStateTypedef_t	s_Temperature_CurrentState =
{
	{0,	0, 0, 0},	//NTC_channel_temperature[2];
	{0,	0},			//onewire_channel_temperature[2];
	0,				//i2c_sensor_temperature
	{400, 400, 400, 400},		//channel_temperature_setpoint[2] set point is 25.0
	{TEC_OUT_DEFAULT, TEC_OUT_DEFAULT, TEC_OUT_DEFAULT,	TEC_OUT_DEFAULT},	//TEC_output_voltage
	{50, 50, 50, 50},	//TEC_output_voltage_now
	0x00,				//TEC_status;	// tec3_auto tec3_ena tec2_auto tec2_ena tec1_auto tec1_ena tec0_auto tec0_ena
	{0,	0, 0, 0},
	0,
	{HEATING, HEATING, HEATING, HEATING}
};

static	uint16_t	sTemperatureTimeCount[4] = {0,0,0,0};
static uint16_t		sTemperatureTimeChange[4] = {0,0,0,0};
static	uint8_t		sPrintLog = 0;
void	temperature_init(void)
{
	uint8_t	idx;
	init_adc();
	TEC_init();
	MCP4921_init();
	SPI_Init();

	for(idx =0; idx < 4; idx ++)
	{
		TEC_shutdown(idx);
		temperature_set_TEC_output(idx, HEATING, 50);
	}
}

void	temperature_enable_channel(uint8_t channel)
{
	TEC_enable(channel);
	s_Temperature_CurrentState.TEC_status |= (1 << (channel*2));
}

void	temperature_disable_channel(uint8_t channel)
{
	TEC_shutdown(channel);
	s_Temperature_CurrentState.TEC_status &= ~(1 << (channel*2));
}


static	void	temperature_task_update(void)
{
	int16_t _NTC_channel0_temperature = get_NTC_temperature_using_table(0);
	int16_t _NTC_channel1_temperature = get_NTC_temperature_using_table(1);
	int16_t _NTC_channel2_temperature = get_NTC_temperature_using_table(2);
	int16_t _NTC_channel3_temperature = get_NTC_temperature_using_table(3);
	
	if ((s_Temperature_CurrentState.TEC_status & (1 << TEC0_AUTO)) == (1 << TEC0_AUTO))		//channel 0 is enabled and auto
	{
		if (s_Temperature_CurrentState.mode[0] == HEATING)
		{
			temperature_TEC_hysteris_control_heating(_NTC_channel0_temperature, 0);
		}
		else
		{
			temperature_TEC_hysteris_control_cooling(_NTC_channel0_temperature, 0);
		}
	}
	
	if ((s_Temperature_CurrentState.TEC_status & (1 << TEC1_AUTO)) == (1 << TEC1_AUTO))		//channel 1 is enabled and auto
	{
		if (s_Temperature_CurrentState.mode[1] == HEATING)
		{
			temperature_TEC_hysteris_control_heating(_NTC_channel0_temperature, 1);
		}
		else
		{
			temperature_TEC_hysteris_control_cooling(_NTC_channel0_temperature, 1);
		}
	}
	
	if ((s_Temperature_CurrentState.TEC_status & (1 << TEC2_AUTO)) == (1 << TEC2_AUTO))		//channel 2 is enabled and auto
	{
		if (s_Temperature_CurrentState.mode[2] == HEATING)
		{
			temperature_TEC_hysteris_control_heating(_NTC_channel1_temperature, 2);
		}
		else
		{
			temperature_TEC_hysteris_control_cooling(_NTC_channel1_temperature, 2);
		}
	}
	
	if ((s_Temperature_CurrentState.TEC_status & (1 << TEC3_AUTO)) == (1 << TEC3_AUTO))		//channel 3 is enabled and auto
	{
		if (s_Temperature_CurrentState.mode[3] == HEATING)
		{
			temperature_TEC_hysteris_control_heating(10, 3);
		}
		else
		{
			temperature_TEC_hysteris_control_cooling(10, 3);
		}
	}
	
	s_Temperature_CurrentState.NTC_channel_temperature[0] = _NTC_channel0_temperature;
	s_Temperature_CurrentState.NTC_channel_temperature[1] = _NTC_channel1_temperature;
	s_Temperature_CurrentState.NTC_channel_temperature[2] = _NTC_channel2_temperature;
	s_Temperature_CurrentState.NTC_channel_temperature[3] = _NTC_channel3_temperature;
	
	// 		// update temperature from onewire sensor
	// 		s_Temperature_CurrentState.onewire_channel_temperature[0] = get_NTC_temperature(0) /*get_onewire_temperature(0)*/;
	// 		s_Temperature_CurrentState.onewire_channel_temperature[1] = get_NTC_temperature(1) /*get_onewire_temperature(1)*/;
	
	// update temperature from BMP390 sensor
	s_Temperature_CurrentState.i2c_sensor_temperature = get_BMP390_temperature();
	
	if (SCH_TIM_HasCompleted(SCH_TIM_TEMPERATURE_LOG))
	{
		if (sPrintLog)
		{
			SCH_TIM_Start(SCH_TIM_TEMPERATURE_LOG,TEMPERATURE_LOG_PERIOD);
			temperature_get_status();
		}
	}

}

//if double output is enabled, turn on both output 0 and 1 if temperature is measuring based on NTC channel 0
void	temperature_TEC_hysteris_control_heating(uint16_t	NTC_temperature, uint8_t TEC_channel)
{
	//		if NTC channel and automatic control
	//		UARTprintf("TEC  %d is controlling heating\r\n", NTC_channel);
	//		UARTprintf("NTCchannel is %d\r\n",NTC_channel);
	if(s_Temperature_CurrentState.TEC_status & (1<<(TEC_channel * 2))) {
		if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] < s_Temperature_CurrentState.TEC_output_voltage[TEC_channel]) {
			sTemperatureTimeChange[TEC_channel]++;
			if(sTemperatureTimeChange[TEC_channel] >= TEC_CHANGE_TIME) {
				sTemperatureTimeChange[TEC_channel] = 0;
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] += 20;
				
				if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] > s_Temperature_CurrentState.TEC_output_voltage[TEC_channel])
					s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = s_Temperature_CurrentState.TEC_output_voltage[TEC_channel];
				temperature_set_TEC_output(TEC_channel, HEATING, s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			}
		} else if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] > s_Temperature_CurrentState.TEC_output_voltage[TEC_channel]) {
			sTemperatureTimeChange[TEC_channel]++;
			if(sTemperatureTimeChange[TEC_channel] >= TEC_CHANGE_TIME) {
				sTemperatureTimeChange[TEC_channel] = 0;
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] -= 20;
				if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] < s_Temperature_CurrentState.TEC_output_voltage[TEC_channel])
					s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = s_Temperature_CurrentState.TEC_output_voltage[TEC_channel];
				temperature_set_TEC_output(TEC_channel, HEATING, s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			}
		}		
	}

		
	
	if (!(s_Temperature_CurrentState.TEC_status & (1<<(TEC_channel * 2))))		//TEC is disabling
	{
		//				UARTprintf("TEC %d is disabling\r\n", NTC_channel);
		if (NTC_temperature < s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] - TEMPERATURE_HYSTERIS)
		{
			TEC_enable(TEC_channel);
			s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
			temperature_set_TEC_output(TEC_channel, COOLING,s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			
			s_Temperature_CurrentState.TEC_status |= (1<<(TEC_channel * 2));
		}
		else  // Temperature is higher than setpoint  but TEC is disabled
		if (NTC_temperature >  (s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] - TEMPERATURE_HYSTERIS))
		{
			//					check if temperature is still increasing, add to the time count
			if ((NTC_temperature >  s_Temperature_CurrentState.NTC_channel_temperature[TEC_channel])  ||(NTC_temperature >  (s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] + TEMPERATURE_MAX_ERROR)))
			{
				sTemperatureTimeCount[TEC_channel]++;
				//						if the temperature keep increasing without the TEC turned on, switch to COOL mode
				if (sTemperatureTimeCount[TEC_channel] > INCREASE_DECREASE_TIMES)
				{
					s_Temperature_CurrentState.mode[TEC_channel] = COOLING;
					sTemperatureTimeCount[TEC_channel] = 0;
					if ((sPrintLog)) UARTprintf("LOG:NTC channel %d Switch to COOLING mode \r\n", TEC_channel);
					s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
					temperature_set_TEC_output(TEC_channel, COOLING,s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
				}
			}
		}
	}
	
	else  //TEC is enabling
	{
		//					UARTprintf("TEC %d is enabling\r\n", NTC_channel);
		if (NTC_temperature > s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] + TEMPERATURE_HYSTERIS)
		{
			//					UARTprintf("TEC %d shutdown\r\n", NTC_channel);
			TEC_shutdown(TEC_channel);
			s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
			s_Temperature_CurrentState.TEC_status &= ~(1<<(TEC_channel * 2));
		}
	}
}

//if double output is enabled, turn on both output 0 and 1 if temperature is measuring based on NTC channel 0
//if double output is enabled, turn on both output 0 and 1 if temperature is measuring based on NTC channel 0
void	temperature_TEC_hysteris_control_cooling(uint16_t	NTC_temperature, uint8_t TEC_channel)
{
	//		if NTC channel and automatic control
	//	UARTprintf("TEC %d is controlling cooling\r\n", NTC_channel);
	if(s_Temperature_CurrentState.TEC_status & (1<<(TEC_channel * 2))) {
		if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] < s_Temperature_CurrentState.TEC_output_voltage[TEC_channel]) {
			sTemperatureTimeChange[TEC_channel]++;
			if(sTemperatureTimeChange[TEC_channel] == TEC_CHANGE_TIME) {
				sTemperatureTimeChange[TEC_channel] = 0;
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] += 20;
				if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] > s_Temperature_CurrentState.TEC_output_voltage[TEC_channel])
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = s_Temperature_CurrentState.TEC_output_voltage[TEC_channel];
				temperature_set_TEC_output(TEC_channel, COOLING, s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			}
			} else if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] > s_Temperature_CurrentState.TEC_output_voltage[TEC_channel]) {
			sTemperatureTimeChange[TEC_channel]++;
			if(sTemperatureTimeChange[TEC_channel] == TEC_CHANGE_TIME) {
				sTemperatureTimeChange[TEC_channel] = 0;
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] -= 20;
				if(s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] < s_Temperature_CurrentState.TEC_output_voltage[TEC_channel])
				s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = s_Temperature_CurrentState.TEC_output_voltage[TEC_channel];
				temperature_set_TEC_output(TEC_channel, COOLING, s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			}
		}
	}
	
	if (!(s_Temperature_CurrentState.TEC_status & (1<<(TEC_channel * 2))))		//TEC is disabling
	{
		//		UARTprintf("TEC is disabling\r\n");
		if (NTC_temperature > s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] + TEMPERATURE_HYSTERIS)
		{
			TEC_enable(TEC_channel);
			s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
			temperature_set_TEC_output(TEC_channel, COOLING,s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
			s_Temperature_CurrentState.TEC_status |= (1<<(TEC_channel * 2));
		}
		else  // Temperature is lower than setpoint  but TEC is disabled
		if (NTC_temperature <  (s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] - TEMPERATURE_HYSTERIS))
		{
			//					check if temperature is still increasing, add to the time count
			if ((NTC_temperature <  s_Temperature_CurrentState.NTC_channel_temperature[TEC_channel])  || (NTC_temperature <  (s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] - TEMPERATURE_MAX_ERROR)))
			{
				sTemperatureTimeCount[TEC_channel]++;
				//						if the temperature keep increasing without the TEC turned on, switch to COOL mode
				if (sTemperatureTimeCount[TEC_channel] > INCREASE_DECREASE_TIMES)
				{
					s_Temperature_CurrentState.mode[TEC_channel] = HEATING;
					sTemperatureTimeCount[TEC_channel] = 0;
					if ((sPrintLog)) UARTprintf("LOG: NTC channel %d Switch to HEATING mode\r\n",TEC_channel);
					s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
					temperature_set_TEC_output(TEC_channel, HEATING, s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel]);
//					temperature_set_TEC_output(TEC_channel , HEATING,s_Temperature_CurrentState.TEC_output_voltage[TEC_channel]);
					
				}
			}

		}
	}
	
	else  //TEC is enabling
	{
		//		UARTprintf("TEC %d is enabling\r\n",NTC_channel);
		if (NTC_temperature < s_Temperature_CurrentState.channel_temperature_setpoint[TEC_channel] - TEMPERATURE_HYSTERIS)
		{
			TEC_shutdown(TEC_channel);
			s_Temperature_CurrentState.TEC_output_voltage_now[TEC_channel] = 50;
			s_Temperature_CurrentState.TEC_status &= ~(1<<(TEC_channel * 2));
			
		}
	}
	
	
}

void	temperature_create_task(void)
{
	SCH_TASK_CreateTask(&s_temperature_task_context .taskHandle, &s_temperature_task_context .taskProperty);
}

void	temperature_set_point(uint16_t	setpoint, uint8_t channel)
{
	s_Temperature_CurrentState.channel_temperature_setpoint[channel] = setpoint;
}

int16_t	temperature_get_NTC(uint8_t	channel)
{
	return s_Temperature_CurrentState.NTC_channel_temperature[channel];
}

int16_t	temperature_get_onewire(uint8_t	channel)
{
	return s_Temperature_CurrentState.onewire_channel_temperature[channel];
}

int16_t	temperature_get_bmp390(void)
{
	return s_Temperature_CurrentState.i2c_sensor_temperature;
}

void	temperature_enable_TEC(uint8_t	channel)
{
	TEC_enable(channel);
	s_Temperature_CurrentState.TEC_status |= (1 << (2*channel));
}

void	temperature_disable_TEC(uint8_t	channel)
{
	TEC_shutdown(channel);
	s_Temperature_CurrentState.TEC_status &= ~ (1 << (2*channel));
}
void	temperature_enable_auto_control_TEC(uint8_t	channel)
{
	s_Temperature_CurrentState.TEC_status |=  (1 << (2*channel + 1));
	temperature_set_TEC_output(channel , s_Temperature_CurrentState.mode[channel>>1],s_Temperature_CurrentState.TEC_output_voltage[channel] );
}

void	temperature_disable_auto_control_TEC(uint8_t	channel)
{
	s_Temperature_CurrentState.TEC_status &=  ~(1 << (2*channel + 1));
}
// set the output of the TEC manually
//@param	channel: 0-3
//@param	HeatCool: 0 HEAT, 1 COOL
//@voltage	Real Voltage multiply with 100
void    temperature_set_TEC_output(uint8_t channel, uint8_t HeatCool, uint16_t	voltage)
{
	uint32_t	_delta = ((uint32_t)voltage * 4096) / 1150;
	uint32_t	_adcVal ;
	if (HeatCool == HEATING)		//want HEAT
	{
		_adcVal = DAC_at_2_5 - _delta;
	}
	else
	{
		_adcVal = DAC_at_2_5 + _delta;
	}

	MCP4291_set_output((uint16_t)_adcVal, 0, 0, 1, channel);
}

uint16_t	temperature_get_voltage(uint8_t	channel)
{
	return s_Temperature_CurrentState.TEC_output_voltage[channel];
}

uint16_t	temperature_get_voltage_now(uint8_t	channel)
{
	return s_Temperature_CurrentState.TEC_output_voltage_now[channel];
}

uint16_t	temperature_get_setpoint(uint8_t	channel)
{
	return s_Temperature_CurrentState.channel_temperature_setpoint[channel];
}

uint8_t	temperature_get_mode(uint8_t	channel)
{
	return s_Temperature_CurrentState.mode[channel];
}

uint8_t	temperature_tec_get_status(void) {
	return s_Temperature_CurrentState.TEC_status;
}

void	temperature_set_auto_voltage(uint8_t	channel, uint16_t voltage)
{
	s_Temperature_CurrentState.TEC_output_voltage[channel] = voltage;
}

void	temperature_get_status(void)
{
	uint8_t	_status = s_Temperature_CurrentState.TEC_status;
/*	UARTprintf("%d %d %d %d %d %d %d %d\r\n", ((_status & (1 << TEC0_ENA)) >> TEC0_ENA), ((_status & (1 << TEC1_ENA)) >> TEC1_ENA),
	((_status & (1 << TEC2_ENA)) >> TEC2_ENA), ((_status & (1 <<TEC3_ENA)) >> TEC3_ENA),
	s_Temperature_CurrentState.mode[0], s_Temperature_CurrentState.mode[1],
	s_Temperature_CurrentState.channel_temperature_setpoint[0], s_Temperature_CurrentState.channel_temperature_setpoint[1]);
*/
	
	UARTprintf("%d %d %d %d    %d %d %d %d    %d %d %d %d    %d %d %d %d    %d %d %d %d    %d %d %d %d\r\n",
	s_Temperature_CurrentState.NTC_channel_temperature[0],s_Temperature_CurrentState.NTC_channel_temperature[1], 
	s_Temperature_CurrentState.NTC_channel_temperature[2],s_Temperature_CurrentState.NTC_channel_temperature[3],
	s_Temperature_CurrentState.TEC_output_voltage_now[0],s_Temperature_CurrentState.TEC_output_voltage_now[1],
	s_Temperature_CurrentState.TEC_output_voltage_now[2],s_Temperature_CurrentState.TEC_output_voltage_now[3],
	s_Temperature_CurrentState.TEC_output_voltage[0],s_Temperature_CurrentState.TEC_output_voltage[1],
	s_Temperature_CurrentState.TEC_output_voltage[2],s_Temperature_CurrentState.TEC_output_voltage[3],
	((_status & (1 << TEC0_AUTO)) >> TEC0_AUTO), ((_status & (1 << TEC1_AUTO)) >> TEC1_AUTO),
	((_status & (1 << TEC2_AUTO)) >> TEC2_AUTO), ((_status & (1 << TEC3_AUTO)) >> TEC3_AUTO),
	((_status & (1 << TEC0_ENA)) >> TEC0_ENA), ((_status & (1 << TEC1_ENA)) >> TEC1_ENA),
	((_status & (1 << TEC2_ENA)) >> TEC2_ENA), ((_status & (1 <<TEC3_ENA)) >> TEC3_ENA),
	s_Temperature_CurrentState.mode[0],s_Temperature_CurrentState.mode[1],
	s_Temperature_CurrentState.mode[2],s_Temperature_CurrentState.mode[3]
	);
	
}

void	temperature_enable_log(void)
{
	sPrintLog  = 1;
}

void	temperature_disable_log(void)
{
	sPrintLog  = 0;
}