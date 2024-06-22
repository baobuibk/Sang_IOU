/*
 * COPC.c
 *
 * Created: 5/18/2024 6:29:56 AM
 *  Author: Admin
 */ 
#include "scheduler.h"
#include "COPC.h"
#include "fsp.h"
#include "IOU_board.h"
#include "temperature.h"
#include "Accel_Gyro.h"
#include "ring_led.h"
#include "ir_led.h"
#include "Pressure.h"
#include "crc.h"


/* Private function ----------------------------------------------------------*/
static	void COPC_task_update(void);
void COPC_process_command(fsp_packet_t	*s_COPC_FspPacket);
void OK_Send(void);
	//Function send to COPC data OK
void copc_iou_set_temp(void);
void copc_iou_tec_ena(void);
void copc_iou_tec_dis(void);
void copc_iou_tec_ena_auto(void);
void copc_iou_tec_dis_auto(void);
void copc_iou_tec_set_output(void);
void copc_iou_tec_set_auto_vol(void);
void copc_iou_tec_status(void);
void copc_iou_tec_log_ena(void);
void copc_iou_tec_log_dis(void);
void copc_iou_ringled_set_RGBW(void);
void copc_iou_irled_set_bright(void);
	//Function send to COPC data in struct payload
void copc_iou_get_temp(void);
void copc_iou_get_temp_setpoint(void);
void copc_iou_ringled_get_RGBW(void);
void copc_iou_irled_get_bright(void);
void copc_iou_get_accel_gyro(void);
void copc_iou_get_press(void);
void copc_iou_get_param(void);



/* Private variables ---------------------------------------------------------*/
volatile static	ringbuffer_t *p_COPCBuffer;
static	fsp_packet_t	s_COPC_FspPacket;
static	fsp_packet_t	s_IOU_FspPacket;
static	COPC_Sfp_Payload_t	*s_pCOPC_Sfp_Payload;
static	IOU_Sfp_Payload_t	*s_pIOU_Sfp_Payload;
volatile uint8_t swap_byte = 0;

/* Private typedef -----------------------------------------------------------*/
typedef struct COPC_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} COPC_TaskContextTypedef;

static COPC_TaskContextTypedef           s_COPC_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		10,									// taskPeriodInMS, call the task to check buffer every 10ms
											//with baudrate of 9600, buffer size can be less than 10 bytes
		COPC_task_update					// taskFunction;
	}
};

void COPC_init(void)
{	
	usart1_init();
	fsp_init(FSP_ADR_IOU);
	p_COPCBuffer = uart_get_uart1_rx_buffer_address();
	s_pCOPC_Sfp_Payload = (COPC_Sfp_Payload_t *)(&s_COPC_FspPacket.payload);
	s_pIOU_Sfp_Payload = (IOU_Sfp_Payload_t *)(&s_IOU_FspPacket.payload);
}

void COPC_create_task(void)
{
	COPC_init();
	SCH_TASK_CreateTask(&s_COPC_task_context.taskHandle, &s_COPC_task_context.taskProperty);
}

static void COPC_task_update(void)
{
	char rxData;
	while (!rbuffer_empty(p_COPCBuffer))
	{
		rxData = rbuffer_remove(p_COPCBuffer);
		if(rxData == FSP_PKT_ESC)	
		{
			swap_byte = 1;
			break;
		}
		if(swap_byte)
		{
			swap_byte = 0;
			if(rxData == FSP_PKT_TSOD)	rxData = FSP_PKT_SOD;
			if(rxData == FSP_PKT_TESC)	rxData = FSP_PKT_ESC;
			if(rxData == FSP_PKT_TEOF)	rxData = FSP_PKT_EOF;
		}
		switch ( fsp_decode(rxData,&s_COPC_FspPacket))
		{
			//process command
            case FSP_PKT_NOT_READY:
				break;
            case FSP_PKT_READY:
				COPC_process_command(&s_COPC_FspPacket);          
	            break;
            case FSP_PKT_INVALID:
	            break;
            case FSP_PKT_WRONG_ADR:
				usart0_send_string("Wrong module address \r\n");
	            break;
            case FSP_PKT_ERROR:
	            break;
            default:
	           break;
		}
	}
}

void COPC_process_command(fsp_packet_t	*s_COPC_FspPacket)
{
	s_pIOU_Sfp_Payload->commonFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	switch (s_pCOPC_Sfp_Payload->commonFrame.Cmd)
	{
		case FSP_CMD_CODE_IOU_SET_TEMP:
			copc_iou_set_temp();
			break;
		case FSP_CMD_CODE_IOU_GET_TEMP:
			copc_iou_get_temp();
			break;
		case FSP_CMD_CODE_IOU_GET_TEMP_SETPOINT:
			copc_iou_get_temp_setpoint();
			break;
		case FSP_CMD_CODE_IOU_TEC_ENA:
			copc_iou_tec_ena();
			break;
		case FSP_CMD_CODE_IOU_TEC_DIS:
			copc_iou_tec_dis();
			break;
		case FSP_CMD_CODE_IOU_TEC_ENA_AUTO:
			copc_iou_tec_ena_auto();
			break;
		case FSP_CMD_CODE_IOU_TEC_DIS_AUTO:
			copc_iou_tec_dis_auto();
			break;
		case FSP_CMD_CODE_IOU_TEC_SET_OUTPUT:
			copc_iou_tec_set_output();
			break;
		case FSP_CMD_CODE_IOU_TEC_AUTO_VOL:
			copc_iou_tec_set_auto_vol();
			break;
		case FSP_CMD_CODE_IOU_TEC_STATUS:
			break;
		case FSP_CMD_CODE_IOU_TEC_LOG_ENA:
			break;
		case FSP_CMD_CODE_IOU_TEC_LOG_DIS:
			break;
		case FSP_CMD_CODE_IOU_RINGLED_SET_RGBW:
			copc_iou_ringled_set_RGBW();
			break;
		case FSP_CMD_CODE_IOU_RINGLED_GET_RGBW:
			copc_iou_ringled_get_RGBW();
			break;
		case FSP_CMD_CODE_IOU_IRLED_SET_BRIGHT:
			copc_iou_irled_set_bright();
			break;
		case FSP_CMD_CODE_IOU_IRLED_GET_BRIGHT:
			copc_iou_irled_get_bright();
			break;
		case FSP_CMD_CODE_IOU_GET_ACCEL_GYRO:
			copc_iou_get_accel_gyro();
			break;
		case FSP_CMD_CODE_IOU_GET_PRESS:
			copc_iou_get_press();
			break;
		case FSP_CMD_CODE_IOU_GET_PARAMETERS:
			copc_iou_get_param();
			break;
		default:
			break;
	}
}


void OK_Send() {
	uint8_t  frame_len;
	uint8_t  payload[15];
	memset((void* ) payload,0,sizeof(payload));
	payload[0] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	fsp_gen_cmd_w_data_pkt(FSP_CMD_RESPONSE_DONE, payload, 1, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket,  payload,  &frame_len);
	usart1_send_array(payload,frame_len);
}

//Function send to COPC data OK
void copc_iou_set_temp(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->setTempCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	uint16_t setpoint = ((s_pCOPC_Sfp_Payload->setTempCommandFrame.setpoint_High) << 8) | (s_pCOPC_Sfp_Payload->setTempCommandFrame.setpoint_Low);
	temperature_set_point(setpoint, channel);
	OK_Send();
}
void copc_iou_tec_ena(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecEnaCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	temperature_enable_TEC(channel);
	OK_Send();
}
void copc_iou_tec_dis(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecDisCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	temperature_disable_TEC(channel);
	OK_Send();
}
void copc_iou_tec_ena_auto(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecEnaAutoCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	temperature_enable_auto_control_TEC(channel << 1);
	temperature_disable_TEC(channel*2);
	temperature_disable_TEC(channel*2 + 1);
	OK_Send();
}
void copc_iou_tec_dis_auto(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecEnaAutoCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	temperature_disable_auto_control_TEC(channel << 1);
	OK_Send();
}
void copc_iou_tec_set_output(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	uint8_t heta_cool = s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.heat_cool;
	uint16_t voltage = ((s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.voltage_High << 8) | s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.voltage_Low);
	temperature_set_TEC_output(channel, heta_cool, voltage);
	OK_Send();
}
void copc_iou_tec_set_auto_vol(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->tecSetAutoVoltageCommandFrame.channel;
	if (channel > (MAX_CHANNEL -1))
		return;
	uint16_t voltage = ((s_pCOPC_Sfp_Payload->tecSetAutoVoltageCommandFrame.voltage_High << 8) | s_pCOPC_Sfp_Payload->tecSetAutoVoltageCommandFrame.voltage_Low);
	temperature_set_auto_voltage(channel, voltage);
	OK_Send();
}

void copc_iou_tec_status(void)
{
	return;
}
void copc_iou_tec_log_ena(void)
{
	return;
}
void copc_iou_tec_log_dis(void)
{
	return;
}
void copc_iou_ringled_set_RGBW(void)
{
	rgbw_color RGBW;
	RGBW.red = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.red;
	RGBW.green = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.green;
	RGBW.blue = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.blue;
	RGBW.white = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.white;
	ringled_set_RGBW(RGBW.red, RGBW.green, RGBW.blue, RGBW.white);
	OK_Send();
}

void copc_iou_irled_set_bright(void)
{
	uint8_t duty = s_pCOPC_Sfp_Payload->IrSetCommandFrame.duty;
	if (duty > 100) duty = 100;
	IR_led_set_DutyCyclesPercent(duty);
	OK_Send();
}

//Function send to COPC data in struct payload
void copc_iou_get_temp(void)
{
	uint8_t device = s_pCOPC_Sfp_Payload->getTempCommandFrame.device;
	uint8_t channel = s_pCOPC_Sfp_Payload->getTempCommandFrame.channel;
	uint16_t temp;
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	if (device > 2 || channel > (MAX_CHANNEL-1))
		return;
	else
	{
		s_pIOU_Sfp_Payload->iouGetTempResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
		s_pIOU_Sfp_Payload->iouGetTempResponseFrame.device = device;
		s_pIOU_Sfp_Payload->iouGetTempResponseFrame.channel = channel;
		if (device == 0)
			temp = (uint16_t)temperature_get_NTC(channel);
		if (device == 1)
			temp = (uint16_t)temperature_get_onewire(channel);
		if (device == 2)
			temp = (uint16_t)temperature_get_bmp390();
		s_pIOU_Sfp_Payload->iouGetTempResponseFrame.temperature_high = (uint8_t)(temp >> 8);
		s_pIOU_Sfp_Payload->iouGetTempResponseFrame.temperature_low = (uint8_t)(temp);
		fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouGetTempResponseFrame, 5, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
 		fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
		usart1_send_array(encode_frame, frame_len);
	}
}
void copc_iou_get_temp_setpoint(void)
{
	uint8_t channel = s_pCOPC_Sfp_Payload->getTempSetpointCommandFrame.channel;
	if (channel > (MAX_CHANNEL-1))
		return;
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	uint16_t setpoint = (uint16_t)temperature_get_setpoint(channel);
	
	s_pIOU_Sfp_Payload->iouGetTempSetpointResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	s_pIOU_Sfp_Payload->iouGetTempSetpointResponseFrame.channel = channel;
	s_pIOU_Sfp_Payload->iouGetTempSetpointResponseFrame.temperature_high = (uint8_t)(setpoint >> 8);
	s_pIOU_Sfp_Payload->iouGetTempSetpointResponseFrame.temperature_low = (uint8_t)(setpoint);
	
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouGetTempSetpointResponseFrame, 4, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}
void copc_iou_ringled_get_RGBW(void)
{
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	rgbw_color RGBW = ringled_get_RGBW();
	s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame.red = RGBW.red;
	s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame.green = RGBW.green;
	s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame.blue = RGBW.blue;
	s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame.white = RGBW.white;
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouRingledGetRGBResponseFrame, 5, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}
void copc_iou_irled_get_bright(void)
{
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	uint8_t duty = IR_led_get_Current_DutyCyclesPercent();
	s_pIOU_Sfp_Payload->iouIRledGetBrightResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	s_pIOU_Sfp_Payload->iouIRledGetBrightResponseFrame.duty =duty;
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouIRledGetBrightResponseFrame, 2, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}
void copc_iou_get_accel_gyro(void)
{	
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	Accel_Gyro_DataTypedef accel_data = get_acceleration();
	Accel_Gyro_DataTypedef gyro_data = get_gyroscope();
	s_pIOU_Sfp_Payload->iouIRledGetBrightResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_x_high = (uint8_t)(accel_data.x >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_x_low = (uint8_t)accel_data.x;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_y_high = (uint8_t)(accel_data.y >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_y_low = (uint8_t)accel_data.y;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_z_high = (uint8_t)(accel_data.z >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.accel_z_low = (uint8_t)accel_data.z;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_x_high = (uint8_t)(gyro_data.x >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_x_low = (uint8_t)gyro_data.x;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_y_high = (uint8_t)(gyro_data.y >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_y_low = (uint8_t)gyro_data.y;
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_z_high = (uint8_t)(gyro_data.z >> 8);
	s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame.gyro_z_low = (uint8_t)gyro_data.z;
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouGetAccelGyroResponseFrame, 13, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}
void copc_iou_get_press(void)
{
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	uint16_t pressure = get_pressure();
	s_pIOU_Sfp_Payload->iouGetPressResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	s_pIOU_Sfp_Payload->iouGetPressResponseFrame.pressure_high = (uint8_t)(pressure >> 8);
	s_pIOU_Sfp_Payload->iouGetPressResponseFrame.pressure_high = (uint8_t)pressure;
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouGetPressResponseFrame, 3, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}

void copc_iou_get_param(void)
{
	uint8_t encode_frame[FSP_PKT_MAX_LENGTH];
	uint8_t frame_len;
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Cmd = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	// GET TEMP NTC
	uint16_t temp = temperature_get_NTC(0);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_0_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_0_low = (uint8_t)temp;
	temp = temperature_get_NTC(1);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_1_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_1_low = (uint8_t)temp;
	temp = temperature_get_NTC(2);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_2_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_2_low = (uint8_t)temp;
	temp = temperature_get_NTC(3);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_3_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_NTC_channel_3_low = (uint8_t)temp;
	// GET TEMP ONEWIRE
	temp = temperature_get_onewire(0);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_onewire_channel_0_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_onewire_channel_0_low = (uint8_t)temp;
	temp = temperature_get_onewire(1);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_onewire_channel_1_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_onewire_channel_1_low = (uint8_t)temp;
	// GET TEMP BMP390
	temp = temperature_get_bmp390();
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_i2c_sensor_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_i2c_sensor_low = (uint8_t)temp;
	// GET TEMP SETPOINT
	temp = (uint16_t)temperature_get_setpoint(0);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_0_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_0_low = (uint8_t)temp;
	temp = (uint16_t)temperature_get_setpoint(1);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_1_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_1_low = (uint8_t)temp;
	temp = (uint16_t)temperature_get_setpoint(2);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_2_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_2_low = (uint8_t)temp;
	temp = (uint16_t)temperature_get_setpoint(3);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_3_high = (uint8_t)(temp >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Temp_setpoint_channel_3_low = (uint8_t)temp;
	// GET VOLTAGE OUTPUT TEC
	uint16_t voltage = temperature_get_voltage(0);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_0_high = (uint8_t)(voltage >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_0_low = (uint8_t)voltage;
	voltage = temperature_get_voltage(1);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_1_high = (uint8_t)(voltage >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_1_low = (uint8_t)voltage;
	voltage = temperature_get_voltage(2);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_2_high = (uint8_t)(voltage >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_2_low = (uint8_t)voltage;
	voltage = temperature_get_voltage(3);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_3_high = (uint8_t)(voltage >> 8);
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Voltage_out_tec_channel_3_low = (uint8_t)voltage;
	// GET NEO LED DATA
	rgbw_color RGBW = ringled_get_RGBW();
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Neo_led_R = (uint8_t)RGBW.red;
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Neo_led_G = (uint8_t)RGBW.green;
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Neo_led_B = (uint8_t)RGBW.blue;
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.Neo_led_W = (uint8_t)RGBW.white;
	// GET IR LED BRIGHT
	s_pIOU_Sfp_Payload->iouGetParamResponseFrame.IRled_duty = (uint8_t)IR_led_get_Current_DutyCyclesPercent();
	// DECODE FRAME
	fsp_gen_pkt((void*)0, &s_pIOU_Sfp_Payload->iouGetParamResponseFrame, 36, FSP_ADR_COPC, FSP_PKT_TYPE_CMD_W_DATA, &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket, encode_frame, &frame_len);
	usart1_send_array(encode_frame, frame_len);
}
