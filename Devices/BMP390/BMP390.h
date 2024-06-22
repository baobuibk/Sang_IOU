/*
 * BMP390.h
 *
 * Created: 6/14/2024 1:04:16 PM
 *  Author: HTSANG
 */ 


#ifndef BMP390_H_
#define BMP390_H_

#include <stdint.h>
#include <stdbool.h>
#include <util/twi.h>
#include <avr/io.h>
#include <stddef.h>
#include <math.h>
#include <util/delay.h>

// Calibration data structure for BMP390
typedef struct {
	uint16_t	u16_NVM_T1;
	uint16_t	u16_NVM_T2;
	int8_t		i8_NVM_T3;
	int16_t		i16_NVM_P1;
	int16_t		i16_NVM_P2;
	int8_t		i8_NVM_P3;
	int8_t		i8_NVM_P4;
	uint16_t	u16_NVM_P5;
	uint16_t	u16_NVM_P6;
	int8_t		i8_NVM_P7;
	int8_t		i8_NVM_P8;
	uint16_t	i16_NVM_P9;
	int8_t		i8_NVM_P10;
	int8_t		i8_NVM_P11;
} _BMP390_Raw_Calib_Data_;

typedef struct {
	float	f_PAR_T1;
	float	f_PAR_T2;
	float	f_PAR_T3;
	float	f_PAR_P1;
	float	f_PAR_P2;
	float	f_PAR_P3;
	float	f_PAR_P4;
	float	f_PAR_P5;
	float	f_PAR_P6;
	float	f_PAR_P7;
	float	f_PAR_P8;
	float	f_PAR_P9;
	float	f_PAR_P10;
	float	f_PAR_P11;
} _BMP390_Calib_Data_;

#define BMP390_AVAIL	1
#define BMP390_UNAVAIL	0

typedef enum {
	BMP390_MODE_SLEEP = 0,
	BMP390_MODE_FORCED = 1,
	BMP390_MODE_NORMAL = 3
} BMP390_Mode;

// Data structure for the BMP390 sensor
typedef struct _BMP390_Data_
{
	int32_t temperature_raw;		// Temperature reading
	float temperature;				// Temperature reading
	int32_t pressure_raw;			// Pressure reading
	float pressure;					// Temperature reading
	uint8_t chipID;
	_BMP390_Raw_Calib_Data_ NVM;	// RAW Calibration data
	_BMP390_Calib_Data_		PAR;
}BMP390_Data;

bool	BMP390_read_raw_calibration(BMP390_Data *data);
void	BMP390_convert_calibration(BMP390_Data *data);
void	BMP390_set_mode(BMP390_Mode mode);
bool	BMP390_init(void);
void	BMP390_read_raw_temp_press(BMP390_Data *data);
void	BMP390_compensate_temperature(BMP390_Data *data);
void	BMP390_compensate_pressure(BMP390_Data *data);
void	BMP390_temp_press_update(void);
int16_t		get_BMP390_temperature(void);
uint16_t	get_BMP390_pressure(void);

#endif /* BMP390_H_ */