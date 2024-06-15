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

/* Private function ----------------------------------------------------------*/
static	void COPC_task_update(void);

/* Private variables ---------------------------------------------------------*/
volatile static	ringbuffer_t *p_COPCBuffer;
static	fsp_packet_t	s_COPC_FspPacket;
static	fsp_packet_t	s_IOU_FspPacket;
static	COPC_Sfp_Payload_t	*s_pCOPC_Sfp_Payload;
static	IOU_Sfp_Payload_t	*s_pIOU_Sfp_Payload;

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
	s_pIOU_Sfp_Payload = (IOU_Sfp_Payload_t *)(&s_IOU_FspPacket.payload);
}

static void COPC_task_update(void)
{
	char rxData;
	while (! rbuffer_empty(p_COPCBuffer))
	{

		rxData = rbuffer_remove(p_COPCBuffer);
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

void COPC_create_task(void)
{
	COPC_init();
	SCH_TASK_CreateTask(&s_COPC_task_context.taskHandle, &s_COPC_task_context.taskProperty);
}


void OK_Send(void) {
	uint8_t  frame_len = 0;
	uint8_t  payload[15];
	memset((void*) payload,0,sizeof(payload));
	payload[0] = s_pCOPC_Sfp_Payload->commonFrame.Cmd;
	fsp_gen_cmd_w_data_pkt(FSP_CMD_RESPONSE_DONE,  payload,  1, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
	fsp_encode(&s_IOU_FspPacket,  payload,  &frame_len);
	usart1_send_array(payload,frame_len);
}

void COPC_process_command(fsp_packet_t	*s_COPC_FspPacket)
{
	uint8_t _channel;
	uint8_t _device;
	uint8_t _frame_len = 0;
	uint8_t _payload[20];
	int16_t _temp;
	int16_t _setpoint;
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
				_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->getTempCommandFrame.Cmd;
				_payload[_frame_len ++] = _device;
				_payload[_frame_len ++] = _channel;
				if (_device == 0)
					_temp = (int16_t)temperature_get_NTC(_channel);
				if (_device == 1)
					_temp = (int16_t)temperature_get_onewire(_channel);
				if (_device == 2)
					_temp = (int16_t)temperature_get_bmp390();
				_payload[_frame_len ++] = (int8_t)(_temp >> 8);
				_payload[_frame_len ++] = (int8_t)(_temp & 0xFF);
				fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
				fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
				usart1_send_array(_payload, _frame_len);
			}
			OK_Send();
			break;
		case FSP_CMD_CODE_IOU_GET_TEMP_SETPOINT:
			_channel = s_pCOPC_Sfp_Payload->getTempSetpointCommandFrame.channel;
			if (_channel > (MAX_CHANNEL-1))
				break;
			_setpoint = (int16_t)temperature_get_setpoint(_channel);
			_payload[_frame_len ++] = s_pCOPC_Sfp_Payload->getTempCommandFrame.Cmd;
			_payload[_frame_len ++] = _channel;
			_payload[_frame_len ++] = (int8_t)(_setpoint >> 8);
			_payload[_frame_len ++] = (int8_t)(_setpoint & 0xFF);
			fsp_gen_data_pkt(_payload, _frame_len, FSP_ADR_COPC, FSP_PKT_WITHOUT_ACK,  &s_IOU_FspPacket);
			fsp_encode(&s_IOU_FspPacket,  _payload,  &_frame_len);
			usart1_send_array(_payload, _frame_len);	
			break;		
	}
}

