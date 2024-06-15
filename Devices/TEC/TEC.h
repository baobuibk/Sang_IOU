/*
 * TEC.h
 *
 * Created: 5/20/2024 7:44:12 AM
 *  Author: Admin
 */ 


#ifndef TEC_H_
#define TEC_H_

#include <stdint.h>

void	TEC_init(void);
uint8_t	TEC_enable(uint8_t	TEC_device);
uint8_t	TEC_shutdown(uint8_t	TEC_device);
#endif /* TEC_H_ */