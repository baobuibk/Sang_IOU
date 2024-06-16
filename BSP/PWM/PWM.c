/*
 * PWM.c
 *
 * Created: 6/13/2024 4:00:14 PM
 *  Author: HTSANG
 */ 

#include "PWM.h"
#include "IOU_board.h"

#define MAX_COUNT_8BIT            255
#define MAX_COUNT_16BIT           65535


void pwm1_init(const uint8_t _Pin, const uint16_t _TimePeriodUs, const uint8_t _DutyCyclePercent)
{
	PORTB &= ~(1 << _Pin);					// Pull low
	DDRB  |=  (1 << _Pin);					// Set pin as Output
	
	TCCR1A |= (1 << WGM11);					// WGM13:0 = 14 (Fast PWM, ICR1 is top)
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
	
	TCCR1A |= (1 << COM1B1);				// Clear OC1A on compare match, set OC1A at BOTTOM (non-inverting mode)
	
	TCCR1B |= (1 << CS11) | (1 << CS10);	// Set prescaler to 64 and start the timer
	
	ICR1  = _TimePeriodUs * 0.125;					// Voi F = 8M thi T = 0.125us, va prescaler = 64 nen T' = 8us
	
	OCR1B = _DutyCyclePercent * ICR1 * 0.01;		// Output Compare
}

void pwm2_init(const uint8_t _Pin, const uint16_t _TimePeriodUs, const uint8_t _DutyCyclePercent)
{
	PORTB &= ~(1 << _Pin);					// Pull low
	DDRB  |=  (1 << _Pin);					// Set pin as Output
	
	TCCR1A |= (1 << WGM11);					// WGM13:0 = 14 (Fast PWM, ICR1 is top)
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
	
	TCCR1A |= (1 << COM1C1);				// Clear OC1A on compare match, set OC1A at BOTTOM (non-inverting mode)
	
	TCCR1B |= (1 << CS11) | (1 << CS10);	// Set prescaler to 64 and start the timer
	
	ICR1  = _TimePeriodUs * 0.125;					// Voi F = 8M thi T = 0.125us, va prescaler = 64 nen T' = 8us
	
	OCR1C = _DutyCyclePercent * ICR1 * 0.01;		// Output Compare
}

void pwm1_set_DutyCyclePercent(const uint8_t _DutyCyclePercent)
{
	OCR1B = _DutyCyclePercent * ICR1 * 0.01;
}

void pwm2_set_DutyCyclePercent(const uint8_t _DutyCyclePercent)
{
	OCR1C = _DutyCyclePercent * ICR1 * 0.01;
}