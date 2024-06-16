/*
 * TEC.c
 *
 * Created: 5/20/2024 7:44:32 AM
 *  Author: Admin
 */ 
#include "TEC.h"
#include <avr/io.h>
#include "IOU_board.h"
void	TEC_init(void)
{
	TEC_SHUTDOWN_TEC0_DDR |= (1<< TEC_SHUTDOWN_TEC0_PIN);
	TEC_SHUTDOWN_TEC1_DDR |= (1<< TEC_SHUTDOWN_TEC1_PIN);
	TEC_SHUTDOWN_TEC2_DDR |= (1<< TEC_SHUTDOWN_TEC2_PIN);
	TEC_SHUTDOWN_TEC3_DDR |= (1<< TEC_SHUTDOWN_TEC3_PIN);
}

uint8_t	TEC_enable(uint8_t	TEC_device)
{
	switch (TEC_device)
	{
		case 0:
			TEC_SHUTDOWN_TEC0_PORT |= (1<< TEC_SHUTDOWN_TEC0_PIN);
			return 0;
		case 1:
			TEC_SHUTDOWN_TEC1_PORT |= (1<< TEC_SHUTDOWN_TEC1_PIN);
			return	0;
		case 2:
			TEC_SHUTDOWN_TEC2_PORT |= (1<< TEC_SHUTDOWN_TEC2_PIN);
			return 0;
		case 3:
			TEC_SHUTDOWN_TEC3_PORT |= (1<< TEC_SHUTDOWN_TEC3_PIN);
			return 0;
		default:
			return 1;
	}
}
	
uint8_t	TEC_shutdown(uint8_t	TEC_device)
{
	switch (TEC_device)
	{
		case 0:
			TEC_SHUTDOWN_TEC0_PORT &= ~(1<< TEC_SHUTDOWN_TEC0_PIN);
			return 0;
		case 1:
			TEC_SHUTDOWN_TEC1_PORT &= ~(1<< TEC_SHUTDOWN_TEC1_PIN);
			return	0;
		case 2:
			TEC_SHUTDOWN_TEC2_PORT &= ~(1<< TEC_SHUTDOWN_TEC2_PIN);
			return 0;
		case 3:
			TEC_SHUTDOWN_TEC3_PORT &= ~(1<< TEC_SHUTDOWN_TEC3_PIN);
			return 0;
		default:
			return 1;
	}
}