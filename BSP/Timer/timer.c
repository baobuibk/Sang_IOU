/*
 * PDU_timer.c
 *
 * Created: 5/15/2024 5:16:05 PM
 *  Author: Admin
 */ 
#include <avr/interrupt.h>
#include "timer.h"

void	timer_start_timer0(void)
{

	TCCR0 |= (1 << CS02);	//prescaler = 64
			  // enable compare interrupt
	TIMSK |= (1 << OCIE0);

			  // enable global interrupts
	sei();
}
void	timer_stop_timer0(void)
{
	TCCR0 &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));	
}
void timer_timer0_init(void)
{
	  // set up timer with prescaler = 64 and CTC mode
	  TCCR0 |= (1 << WGM01);

	  // initialize counter
	  TCNT0 = 0;

	  // initialize compare value (tick every 1ms)
	  OCR0 = 125 - 1;
}

//void timer_timer1_init(void)
//{
	//// set up timer with prescaler = 64 and CTC mode
//
//
	//// initialize counter
	//TCNT1 = 0;
//
	//// initialize compare value (tick every 1ms)
	//OCR1 = 100 - 1;
//}