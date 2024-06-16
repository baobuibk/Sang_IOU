/*
 * TPL5010.c
 *
 * Created: 5/20/2024 1:55:43 PM
 *  Author: Admin
 */ 
#include <avr/io.h>
#include "TPL5010.h"
#include "IOU_board.h"
void	TPL5010_init (void)
{
	TPL5010_DONE_DDR |=  (1 << TPL5010_DONE_PIN);	//done is output
	TPL5010_WAKE_DDR &= ~(1 << TPL5010_WAKE_PIN);	//wake is input
	TPL5010_WAKE_PORT |=  (1 << TPL5010_WAKE_PIN);	//pull up Wake
	TPL5010_DONE_PORT	&= ~(1 << TPL5010_DONE_PIN);	//init as 0
	return;
};
void	TPL5010_done_pulse (void)
{
	
	TPL5010_DONE_PORT	|= (1 << TPL5010_DONE_PIN);
	TPL5010_DONE_PORT	&= ~(1 << TPL5010_DONE_PIN);
}