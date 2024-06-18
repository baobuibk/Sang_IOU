/*
 * COPC.h
 *
 * Created: 5/18/2024 6:04:17 AM
 *  Author: Admin
 */ 


#ifndef COPC_H_
#define COPC_H_

#include "fsp.h"
#include "uart.h"

typedef struct _COMMON_FRAME_
{
	uint8_t Cmd;	
}COMMON_FRAME;


// FROM COPC TO IOU
//--------------------------------------------------------------------------------------------
typedef struct _COPC_SET_TEMP_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
	uint8_t		setpoint_High;
	uint8_t		setpoint_Low;
}COPC_SET_TEMP_COMMAND_FRAME;

typedef struct _COPC_GET_TEMP_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		device;
	uint8_t		channel;
}COPC_GET_TEMP_COMMAND_FRAME;

typedef struct _COPC_GET_TEMP_SETPOINT_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
}COPC_GET_TEMP_SETPOINT_COMMAND_FRAME;

typedef struct _COPC_TEC_ENA_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
}COPC_TEC_ENA_COMMAND_FRAME;

typedef struct _COPC_TEC_DIS_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
}COPC_TEC_DIS_COMMAND_FRAME;

typedef struct _COPC_TEC_ENA_AUTO_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;

}COPC_TEC_ENA_AUTO_COMMAND_FRAME;

typedef struct _COPC_TEC_DIS_AUTO_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
}COPC_TEC_DIS_AUTO_COMMAND_FRAME;

typedef struct _COPC_TEC_SET_OUTPUT_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
	uint8_t		heat_cool;
	uint8_t		voltage_High;				//250 mean 2.5V
	uint8_t		voltage_Low;				//250 mean 2.5V
}TEC_SET_OUTPUT_COMMAND_FRAME;

typedef struct _COPC_TEC_SET_AUTO_VOLTAGE_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		channel;
	uint8_t		voltage_High;				//250 mean 2.5V
	uint8_t		voltage_Low;				//250 mean 2.5V
}TEC_SET_AUTO_VOLTAGE_COMMAND_FRAME;

typedef struct _COPC_NEO_SET_RGB_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		red;			//0 to 255
	uint8_t		green;			//0 to 255
	uint8_t		blue;			//0 to 255
	uint8_t		white;
}COPC_NEO_SET_RGB_COMMAND_FRAME;

typedef struct _COPC_IR_SET_BRIGHTNESS_COMMAND_FRAME_
{
	uint8_t		Cmd;
	uint8_t		duty;			//0 to 100
}COPC_IR_SET_COMMAND_FRAME;


// Union to encapsulate all frame types
typedef union _COPC_Sfp_Payload_ {
	COMMON_FRAME							commonFrame;
	COPC_SET_TEMP_COMMAND_FRAME				setTempCommandFrame;
	COPC_GET_TEMP_COMMAND_FRAME				getTempCommandFrame;
	COPC_GET_TEMP_SETPOINT_COMMAND_FRAME	getTempSetpointCommandFrame;
	COPC_TEC_ENA_COMMAND_FRAME				tecEnaCommandFrame;
	COPC_TEC_DIS_COMMAND_FRAME				tecDisCommandFrame;
	COPC_TEC_ENA_AUTO_COMMAND_FRAME			tecEnaAutoCommandFrame;
	COPC_TEC_DIS_AUTO_COMMAND_FRAME			tecDisAutoCommandFrame;
	TEC_SET_OUTPUT_COMMAND_FRAME			tecSetOutputCommandFrame;
	TEC_SET_AUTO_VOLTAGE_COMMAND_FRAME		tecSetAutoVoltageCommandFrame;
	COPC_NEO_SET_RGB_COMMAND_FRAME			neoSetRGBCommandFrame;
	COPC_IR_SET_COMMAND_FRAME				IrSetCommandFrame;
} COPC_Sfp_Payload_t;


// FROM IOU TO COPC
//--------------------------------------------------------------------------------------------
typedef struct _IOU_GET_TEMP_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			device;
	uint8_t			channel;
	uint8_t			temperature_high;
	uint8_t			temperature_low;
}IOU_GET_TEMP_RESPONSE_FRAME;

typedef	struct _IOU_GET_TEMP_SETPOINT_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			channel;
	uint8_t			temperature_high;
	uint8_t			temperature_low;
}IOU_GET_TEMP_SETPOINT_RESPONSE_FRAME;

typedef struct _IOU_RINGLED_GET_RGB_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			red;
	uint8_t			green;
	uint8_t			blue;
	uint8_t			white;
}IOU_RINGLED_GET_RGB_RESPONSE_FRAME;

typedef struct _IOU_IRLED_GET_BRIGHT_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			duty;
}IOU_IRLED_GET_BRIGHT_RESPONSE_FRAME;

typedef struct _IOU_GET_ACCEL_GYRO_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			accel_x_high;
	uint8_t			accel_x_low;
	uint8_t			accel_y_high;
	uint8_t			accel_y_low;
	uint8_t			accel_z_high;
	uint8_t			accel_z_low;
	uint8_t			gyro_x_high;
	uint8_t			gyro_x_low;
	uint8_t			gyro_y_high;
	uint8_t			gyro_y_low;
	uint8_t			gyro_z_high;
	uint8_t			gyro_z_low;
}IOU_GET_ACCEL_GYRO_RESPONSE_FRAME;

typedef struct _IOU_GET_PRESS_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			pressure_high;
	uint8_t			pressure_low;
}IOU_GET_PRESS_RESPONSE_FRAME;

typedef struct _IOU_GET_PARAM_RESPONSE_FRAME_
{
	uint8_t			Cmd;
	uint8_t			Temp_NTC_channel_0_high;
	uint8_t			Temp_NTC_channel_0_low;
	uint8_t			Temp_NTC_channel_1_high;
	uint8_t			Temp_NTC_channel_1_low;
	uint8_t			Temp_NTC_channel_2_high;
	uint8_t			Temp_NTC_channel_2_low;
	uint8_t			Temp_NTC_channel_3_high;
	uint8_t			Temp_NTC_channel_3_low;

	uint8_t			Temp_onewire_channel_0_high;
	uint8_t			Temp_onewire_channel_0_low;
	uint8_t			Temp_onewire_channel_1_high;
	uint8_t			Temp_onewire_channel_1_low;

	uint8_t			Temp_i2c_sensor_high;
	uint8_t			Temp_i2c_sensor_low;

	uint8_t			Temp_setpoint_channel_0_high;
	uint8_t			Temp_setpoint_channel_0_low;
	uint8_t			Temp_setpoint_channel_1_high;
	uint8_t			Temp_setpoint_channel_1_low;
	uint8_t			Temp_setpoint_channel_2_high;
	uint8_t			Temp_setpoint_channel_2_low;
	uint8_t			Temp_setpoint_channel_3_high;
	uint8_t			Temp_setpoint_channel_3_low;

	uint8_t			Voltage_out_tec_channel_0_high;
	uint8_t			Voltage_out_tec_channel_0_low;
	uint8_t			Voltage_out_tec_channel_1_high;
	uint8_t			Voltage_out_tec_channel_1_low;
	uint8_t			Voltage_out_tec_channel_2_high;
	uint8_t			Voltage_out_tec_channel_2_low;
	uint8_t			Voltage_out_tec_channel_3_high;
	uint8_t			Voltage_out_tec_channel_3_low;

	uint8_t			Neo_led_R;
	uint8_t			Neo_led_G;
	uint8_t			Neo_led_B;
	uint8_t			Neo_led_W;
	
	uint8_t			IRled_duty;
}IOU_GET_PARAM_RESPONSE_FRAME;

typedef union _IOU_Sfp_Payload_{
	COMMON_FRAME							commonFrame;
	IOU_GET_TEMP_RESPONSE_FRAME				iouGetTempResponseFrame;
	IOU_GET_TEMP_SETPOINT_RESPONSE_FRAME	iouGetTempSetpointResponseFrame;
	IOU_RINGLED_GET_RGB_RESPONSE_FRAME		iouRingledGetRGBResponseFrame;
	IOU_IRLED_GET_BRIGHT_RESPONSE_FRAME		iouIRledGetBrightResponseFrame;
	IOU_GET_ACCEL_GYRO_RESPONSE_FRAME		iouGetAccelGyroResponseFrame;
	IOU_GET_PRESS_RESPONSE_FRAME			iouGetPressResponseFrame;
	IOU_GET_PARAM_RESPONSE_FRAME			iouGetParamResponseFrame;
}IOU_Sfp_Payload_t;

void	COPC_init(void);
void	COPC_create_task(void);

#endif /* COPC_H_ */