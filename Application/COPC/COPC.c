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

/* Private function ----------------------------------------------------------*/
static	void COPC_task_update(void);

/* Private variables ---------------------------------------------------------*/
volatile static	ringbuffer_t *p_COPCBuffer;
static	fsp_packet_t	s_COPC_FspPacket;
static	fsp_packet_t	s_IOU_FspPacket;
static	COPC_Sfp_Payload_t	*s_pCOPC_Sfp_Payload;
//static	IOU_Sfp_Payload_t	*s_pIOU_Sfp_Payload;
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

void OK_Send(void);
void COPC_process_command(fsp_packet_t	*s_COPC_FspPacket);


void COPC_init(void)
{	
	usart1_init();
	fsp_init(FSP_ADR_IOU);
	p_COPCBuffer = uart_get_uart1_rx_buffer_address();
	s_pCOPC_Sfp_Payload = (COPC_Sfp_Payload_t *)(&s_COPC_FspPacket.payload);
	//s_pIOU_Sfp_Payload = (IOU_Sfp_Payload_t *)(&s_IOU_FspPacket.payload);
}

static void COPC_task_update(void)
{
	char rxData;
	while (! rbuffer_empty(p_COPCBuffer))
	{
		rxData = rbuffer_remove(p_COPCBuffer);
		if(rxData == FSP_PKT_ESC)	{
			swap_byte = 1;
			break;
		}
		if(swap_byte) {
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
//				UARTprintf("FSP_PKT_READY");
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

void COPC_create_task(void)
{
	COPC_init();
	SCH_TASK_CreateTask(&s_COPC_task_context.taskHandle, &s_COPC_task_context.taskProperty);
}


void OK_Send() {
	uint8_t  frame_len = 0;
	uint8_t  payload[15];
	memset((void* ) payload,0,sizeof(payload));
	payload[0] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	fsp_gen_cmd_w_data_pkt(FSP_CMD_RESPONSE_DONE,  payload,  1, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket,  payload,  &frame_len);
	usart1_send_array(payload,frame_len);
}

void COPC_process_command(fsp_packet_t	*s_COPC_FspPacket)
{
	uint8_t _channel;
	uint8_t _device;
	uint8_t _mode;
	uint8_t _duty;
	uint8_t _frame_len = 0;
	uint8_t _payload[FSP_PAYLOAD_MAX_LENGTH];
	uint16_t _temp;
	uint16_t _setpoint;
	uint16_t _voltage;
	int16_t _pressure;
	
	rgb_color _RGB;
	Accel_Gyro_DataTypedef _accel_data;
	Accel_Gyro_DataTypedef _gyro_data;
	switch (s_pCOPC_Sfp_Payload->commonFrame.Cmd)
	{
		case FSP_CMD_CODE_IOU_SET_TEMP:
			_channel = s_pCOPC_Sfp_Payload->setTempCommandFrame.channel;
			_setpoint = s_pCOPC_Sfp_Payload->setTempCommandFrame.setpoint;
			if (_channel > (MAX_CHANNEL-1))
				break;
			temperature_set_point(_setpoint, _channel);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_GET_TEMP:
			_device = s_pCOPC_Sfp_Payload->getTempCommandFrame.device;
			_channel = s_pCOPC_Sfp_Payload->getTempCommandFrame.channel;
			if (_device > 2 || _channel > (MAX_CHANNEL-1))
				break;
			else
			{
//				_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->getTempCommandFrame.Cmd;
				_payload[_frame_len ++] = _device;
				_payload[_frame_len ++] = _channel;
				if (_device == 0)
					_temp = (uint16_t)temperature_get_NTC(_channel);
				if (_device == 1)
					_temp = (uint16_t)temperature_get_onewire(_channel);
				if (_device == 2)
					_temp = (uint16_t)temperature_get_bmp390();
				_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
				_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
				fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->getTempCommandFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
//				fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
				fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
				usart1_send_array(_payload, _frame_len);
			}
//			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_GET_TEMP_SETPOINT:
			_channel = s_pCOPC_Sfp_Payload->getTempSetpointCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			_setpoint = (uint16_t)temperature_get_setpoint(_channel);
			_payload[_frame_len ++] = _channel;
			_payload[_frame_len ++] = (uint8_t)(_setpoint);
			_payload[_frame_len ++] = (uint8_t)(_setpoint >> 8);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->getTempSetpointCommandFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		case FSP_CMD_CODE_IOU_TEC_ENA:
			_channel = s_pCOPC_Sfp_Payload->tecEnaCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			temperature_enable_TEC(_channel);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_DIS:
			_channel = s_pCOPC_Sfp_Payload->tecDisCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			temperature_disable_TEC(_channel);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_ENA_AUTO:
			_channel = s_pCOPC_Sfp_Payload->tecEnaAutoCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			temperature_enable_auto_control_TEC(_channel << 1);
			temperature_disable_TEC(_channel*2);
			temperature_disable_TEC(_channel*2 + 1);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_DIS_AUTO:
			_channel = s_pCOPC_Sfp_Payload->tecEnaAutoCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			temperature_disable_auto_control_TEC(_channel << 1);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_SET_OUTPUT:
			_channel = s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			_mode = s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.heat_cool;
			_voltage = s_pCOPC_Sfp_Payload->tecSetOutputCommandFrame.voltage;
			temperature_set_TEC_output(_channel, _mode, _voltage);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_AUTO_VOL:
			_channel = s_pCOPC_Sfp_Payload->tecSetAutoVoltageCommandFrame.channel;
			if (_channel > (MAX_CHANNEL -1))
				break;
			_voltage = s_pCOPC_Sfp_Payload->tecSetAutoVoltageCommandFrame.voltage;
			temperature_set_auto_voltage(_channel, _voltage);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_TEC_STATUS:
			break;
		case FSP_CMD_CODE_IOU_TEC_LOG_ENA:
			break;
		case FSP_CMD_CODE_IOU_TEC_LOG_DIS:
			break;
		case FSP_CMD_CODE_IOU_RINGLED_SET_RGB:
			_RGB.red = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.red;
			_RGB.green = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.green;
			_RGB.blue = s_pCOPC_Sfp_Payload->neoSetRGBCommandFrame.blue;
			ringled_set_RGB(_RGB.red, _RGB.green, _RGB.blue, _RGB.white);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_RINGLED_GET_RGB:
			_RGB = ringled_get_RGB();
//			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
			_payload[_frame_len ++]	= (uint8_t)_RGB.red;
			_payload[_frame_len ++]	= (uint8_t)_RGB.green;
			_payload[_frame_len ++]	= (uint8_t)_RGB.blue;
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->commonFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		case FSP_CMD_CODE_IOU_IRLED_SET_BRIGHT:
			_duty = s_pCOPC_Sfp_Payload->IrSetCommandFrame.duty;
			if (_duty > 100) _duty = 100;
			IR_led_set_DutyCyclesPercent(_duty);
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_IRLED_GET_BRIGHT:
			_duty = IR_led_get_Current_DutyCyclesPercent();
//			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
			_payload[_frame_len ++] = _duty;
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->commonFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		case FSP_CMD_CODE_IOU_GET_ACCEL_GYRO:
			_accel_data = get_acceleration();
			_gyro_data = get_gyroscope();
//			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
			_payload[_frame_len ++] = (uint8_t)(_accel_data.x >> 8);
			_payload[_frame_len ++] = (uint8_t)(_accel_data.x & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_accel_data.y >> 8);
			_payload[_frame_len ++] = (uint8_t)(_accel_data.y & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_accel_data.z >> 8);
			_payload[_frame_len ++] = (uint8_t)(_accel_data.z & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.x >> 8);
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.x & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.y >> 8);
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.y & 0xFF);			
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.z >> 8);
			_payload[_frame_len ++] = (uint8_t)(_gyro_data.z & 0xFF);
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->commonFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		case FSP_CMD_CODE_IOU_GET_PRESS:
			_pressure = get_pressure();
//			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
			_payload[_frame_len ++] = (uint8_t)(_pressure >> 8);
			_payload[_frame_len ++] = (uint8_t)(_pressure & 0xFF);
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->commonFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		case FSP_CMD_CODE_IOU_GET_PARAMETERS:
//			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
		// GET TEMP NTC
			_temp = temperature_get_NTC(0);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);			
			_temp = temperature_get_NTC(1);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
			_temp = temperature_get_NTC(2);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
			_temp = temperature_get_NTC(3);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
		// GET TEMP ONEWIRE
			_temp = temperature_get_onewire(0);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
			_temp = temperature_get_onewire(1);
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
		// GET TEMP BMP390
			_temp = temperature_get_bmp390();
			_payload[_frame_len ++] = (uint8_t)(_temp >> 8);
			_payload[_frame_len ++] = (uint8_t)(_temp & 0xFF);
		// GET TEMP SETPOINT
			_setpoint = (uint16_t)temperature_get_setpoint(0);
			_payload[_frame_len ++] = (uint8_t)(_setpoint & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_setpoint >> 8);
			_setpoint = (uint16_t)temperature_get_setpoint(1);
			_payload[_frame_len ++] = (uint8_t)(_setpoint & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_setpoint >> 8);
			_setpoint = (uint16_t)temperature_get_setpoint(2);
			_payload[_frame_len ++] = (uint8_t)(_setpoint & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_setpoint >> 8);
			_setpoint = (uint16_t)temperature_get_setpoint(3);
			_payload[_frame_len ++] = (uint8_t)(_setpoint & 0xFF);
			_payload[_frame_len ++] = (uint8_t)(_setpoint >> 8);
		// GET VOLTAGE OUTPUT TEC
		
		
		// GET NEO LED DATA
			_RGB = ringled_get_RGB();
			_payload[_frame_len ++]	= (uint8_t)_RGB.red;
			_payload[_frame_len ++]	= (uint8_t)_RGB.green;
			_payload[_frame_len ++]	= (uint8_t)_RGB.blue;
			_payload[_frame_len ++]	= (uint8_t)_RGB.white;
		// GET IR LED BRIGHT
			_duty = IR_led_get_Current_DutyCyclesPercent();
			_payload[_frame_len ++] = _duty;
		// DECODE FRAME	
//			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_gen_cmd_w_data_pkt(s_pCOPC_Sfp_Payload->commonFrame.Cmd, _payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK, &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);
			break;
		default:
			break;
	}
}

