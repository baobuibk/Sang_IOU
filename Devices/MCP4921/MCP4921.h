/*
 * MCP4921.h
 *
 * Created: 5/19/2024 8:26:17 AM
 *  Author: Admin
 */ 


#ifndef MCP4921_H_
#define MCP4921_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"
#include "IOU_board.h"
void	MCP4921_init(void);

uint8_t	MCP4291_set_output(uint16_t	Val, uint8_t shutdown, uint8_t gain_ena, uint8_t buf_ena, uint8_t DAC_num);


#endif /* MCP4921_H_ */