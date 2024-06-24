/*
 * IOU_data.h
 *
 * Created: 5/23/2024 11:49:09 AM
 *  Author: Admin
 */ 


#ifndef IOU_DATA_H_
#define IOU_DATA_H_

#include <stdint.h>
#include <time.h>

typedef	enum 
{IOU_POWERUP=0, IOU_NORMAL, IOU_ERROR, IOU_RECEIVING_COMMAND} IOU_StateTypedef_t;
#define HEATING	1
#define COOLING	0
#define TEMPERATURE_HYSTERIS	2	//hysteris is 0.5 Celcius
#define TEMPERATURE_MAX_ERROR	30	//hysteris is 3 Celcius	
typedef struct __IRLed_DataTypeDef__
{
	uint8_t	status;			//0: disable; 1: enable
	uint8_t	duty;			//0->100
}	IRLed_DataTypeDef_t;
typedef struct __RingLed_DataTypeDef__
{
	uint8_t	status;			//0: disable; 1: enable
	uint8_t	Red;			//0->255
	uint8_t	Green;			//0->255
	uint8_t	Blue;			//0->255
	uint8_t	White;			//0->255
}	RingLed_DataTypeDef_t;
typedef	struct _Temperature_CurrentStateTypedef_
{
	int16_t					NTC_channel_temperature[4];
	int16_t					onewire_channel_temperature[2];
	int16_t					i2c_sensor_temperature;
	uint16_t				channel_temperature_setpoint[4];
	uint16_t				TEC_output_voltage[4];		
	uint8_t					TEC_status;	// tec3_auto tec3_ena tec2_auto tec2_ena tec1_auto tec1_ena tec0_auto tec0_ena
	uint16_t				DAC_channel_Val[4];
	uint8_t					DAC_status;	//x x x x DAC3_active	DAC2_active DAC1_active DAC0_active
	uint8_t					mode[4];	//0: HEAT mode; 1: COOL mode
	//uint8_t				tec_channel_auto_control[4];
}Temperature_CurrentStateTypedef_t;

#define	TEC0_ENA	0
#define	TEC0_AUTO	1
#define	TEC1_ENA	2
#define	TEC1_AUTO	3
#define	TEC2_ENA	4
#define	TEC2_AUTO	5
#define	TEC3_ENA	6
#define	TEC3_AUTO	7


typedef struct __IOU_DataTypeDef__
{
	IOU_StateTypedef_t				IOU_State_Data;
	Temperature_CurrentStateTypedef_t	IOU_TemperatureData;
	IRLed_DataTypeDef_t				IOU_IRLedData;
	RingLed_DataTypeDef_t			IOU_RingLedData;
	}IOU_DataTypeDef_t;

extern IOU_DataTypeDef_t	IOU_data;
#endif /* IOU_DATA_H_ */