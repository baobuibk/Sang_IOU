/*
 * temperature.h
 *
 * Created: 5/20/2024 6:02:47 AM
 *  Author: Admin
 */ 


#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include "scheduler.h"
#include "IOU_board.h"
#include "ntc_10k.h"
#include "BMP390.h"
#include "MCP4921.h"
#include "TEC.h"
#include "uart.h"
#include "IOU_data.h"

void	temperature_init(void);
void	temperature_create_task(void);
void	temperature_set_point(uint16_t	setpoint, uint8_t channel);
int16_t		temperature_get_NTC(uint8_t	channel);
int16_t		temperature_get_onewire(uint8_t	channel);
int16_t		temperature_get_bmp390(void);
void	temperature_enable_auto_control_TEC(uint8_t	channel);
void	temperature_disable_auto_control_TEC(uint8_t	channel);
void	temperature_enable_TEC(uint8_t	channel);
void	temperature_disable_TEC(uint8_t	channel);
void	temperature_enable_channel(uint8_t channel);
void	temperature_disable_channel(uint8_t channel);
void    temperature_set_TEC_output(uint8_t channel, uint8_t HeatCool, uint16_t	voltage);
uint16_t	temperature_get_setpoint(uint8_t channel);
uint16_t	temperature_get_voltage(uint8_t	channel);
void	temperature_set_auto_voltage(uint8_t channel, uint16_t voltage);
void	temperature_get_status(void);
void	temperature_enable_log(void);
void	temperature_disable_log(void);
#endif /* TEMPERATURE_H_ */