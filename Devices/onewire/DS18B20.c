/*
 * DS18B20.c
 *
 * Created: 5/28/2024 4:07:43 PM
 *  Author: HTSANG
 */ 

#define   F_CPU		8000000UL

#include "DS18B20.h"
#include <avr/io.h>
#include "IOU_board.h"
#include <util/delay.h>
#include <uart.h>

uint8_t onewire_init(uint8_t channel)
{
	uint8_t flag = 1;
	ONEWIRE_PORT &= ~(1 << channel); // Pull low
	ONEWIRE_DDR |= (1 << channel); // Set as output
	_delay_us(500); // Wait for 500 microseconds
	ONEWIRE_DDR &= ~(1 << channel); // Set as input
	ONEWIRE_PORT |= (1 << channel); // low pull up
	_delay_us(70); // Wait for 70 microseconds
	flag = (ONEWIRE_PIN &(1 << channel) ? 0:1);
	_delay_us(100);
	return flag;
}

void onewire_write_bit(uint8_t channel, uint8_t bit)
{
	ONEWIRE_PORT &= ~(1 << channel);
	ONEWIRE_DDR  |=  (1 << channel);
	_delay_us(2);
	if (bit)
	ONEWIRE_PORT |= (1 << channel);
	_delay_us(60);
	ONEWIRE_DDR  &= ~(1 << channel);
	_delay_us(2);
}

void onewire_write_byte(uint8_t channel, uint8_t value)
{
	for (uint8_t i=0; i<8; i++)
	onewire_write_bit(channel, (value>>i) & 0x01);
}

uint8_t onewire_read_bit(uint8_t channel)
{
	uint8_t bit = 0;
	ONEWIRE_PORT &= ~(1 << channel);
	ONEWIRE_DDR  |=  (1 << channel);
	_delay_us(2);
	ONEWIRE_DDR  &= ~(1 << channel);
	_delay_us(5);
	bit = (ONEWIRE_PIN & (1 << channel));
	_delay_us(50);
	return bit;
}

uint8_t onewire_read_byte(uint8_t channel)
{
	uint8_t value = 0;
	for (uint8_t i = 0; i<8; i++)
	if(onewire_read_bit(channel))
	value |= 1<<i;
	return value;
}

float onewire_read_temp(uint8_t channel)
{
	uint8_t tempHigh = 0;
	uint16_t temperature = 0;
	if (!onewire_init(channel))
	return -1;
	onewire_write_byte(channel, 0xCC);
	onewire_write_byte(channel, 0x44);
	while(!onewire_read_byte(channel));
	
	if (!onewire_init(channel))
	return -1;
	onewire_write_byte(channel, 0xCC);
	onewire_write_byte(channel, 0xBE);
	
	temperature = onewire_read_byte(channel);
	tempHigh    = onewire_read_byte(channel);
	temperature |= (uint16_t)(tempHigh << 8);
	if (tempHigh >> 7)
	{
		temperature -= 1;
		temperature = ~temperature;
		return (float)temperature/(-16.0);
	}
	return (float)temperature/16.0;
}