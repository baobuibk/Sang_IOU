/*
 * DS18B20.h
 *
 * Created: 5/28/2024 4:06:27 PM
 *  Author: HTSANG
 */ 


#ifndef DS18B20_H_
#define DS18B20_H_


#include <stdint.h>

uint8_t onewire_init(uint8_t channel);
void onewire_write_bit(uint8_t channel, uint8_t bit);
void onewire_write_byte(uint8_t channel, uint8_t value);
uint8_t onewire_read_bit(uint8_t channel);
uint8_t onewire_read_byte(uint8_t channel);
float onewire_read_temp(uint8_t channel);


#endif /* DS18B20_H_ */