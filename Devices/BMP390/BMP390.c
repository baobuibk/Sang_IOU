/*
 * BMP390.c
 *
 * Created: 6/14/2024 1:04:29 PM
 *  Author: HTSANG
 */ 

#include "BMP390.h"
#include "twi.h"
#include <util/delay.h>
#include <stdint.h>
#include <util/twi.h>
#include <avr/io.h>
#include <stddef.h>
#include <math.h>
#include "uart.h"
#include "scheduler.h"
#include "IOU_board.h"

// BMP390 address
#define BMP390_I2C_ADDR					0x77
#define BMP390_CALIB_DATA_ADDR			0x31 // Starting address for calibration data in BMP390
#define BMP390_TEMP_PRESS_DATA_ADDR		0x04 // Starting address for temperature and pressure data
#define BMP390_REG_PWR_CTRL				0x1B

// BMP390 data
BMP390_Data TempPress_data;
uint16_t tmp;
// Function to read raw calibration data from BMP390 sensor
bool BMP390_read_raw_calibration(BMP390_Data *data) {
	uint8_t calib[21];

	twi_start();

	// Send device address with write flag
	twi_write(BMP390_I2C_ADDR << 1);

	// Send register address of calibration data
	twi_write(BMP390_CALIB_DATA_ADDR);

	// Send repeated start condition
	twi_start();

	// Send device address with read flag
	twi_write((BMP390_I2C_ADDR << 1) | 1);

	// Receive calibration data
	for (uint8_t i = 0; i < 20; i++) {
		calib[i] = twi_read_ack();
	}
	calib[20] = twi_read_nack();
	if (!calib[20])
		return false;
	// Store the raw calibration data into the _BMP390_Raw_Calib_Data_ structure
	data->NVM.u16_NVM_T1 = ((uint16_t)calib[1] << 8) | (uint16_t)calib[0];
	data->NVM.u16_NVM_T2 = ((uint16_t)calib[3] << 8) | (uint16_t)calib[2];
	data->NVM.i8_NVM_T3 = calib[4];
	data->NVM.i16_NVM_P1 = ((int16_t)calib[6] << 8) | (int16_t)calib[5];
	data->NVM.i16_NVM_P2 = ((int16_t)calib[8] << 8) | (int16_t)calib[7];
	data->NVM.i8_NVM_P3 = calib[9];
	data->NVM.i8_NVM_P4 = calib[10];
	data->NVM.u16_NVM_P5 = ((uint16_t)calib[12] << 8) | (uint16_t)calib[11];
	data->NVM.u16_NVM_P6 = ((uint16_t)calib[14] << 8) | (uint16_t)calib[13];
	data->NVM.i8_NVM_P7 = calib[15];
	data->NVM.i8_NVM_P8 = calib[16];
	data->NVM.i16_NVM_P9 = ((int16_t)calib[18] << 8) | (int16_t)calib[17];
	data->NVM.i8_NVM_P10 = calib[19];
	data->NVM.i8_NVM_P11 = calib[20];
	
	// Send stop condition
	twi_stop();
	return true;
}

// Function to convert raw calibration data to floating-point calibration data
void BMP390_convert_calibration(BMP390_Data *data) {
	data->PAR.f_PAR_T1 = data->NVM.u16_NVM_T1 * 256.0;
	data->PAR.f_PAR_T2 = data->NVM.u16_NVM_T2 / 1073741824.0;
	data->PAR.f_PAR_T3 = data->NVM.i8_NVM_T3 / 281474976710656.0;
	data->PAR.f_PAR_P1 = (data->NVM.i16_NVM_P1 - 16384) / 1048576.0;
	data->PAR.f_PAR_P2 = (data->NVM.i16_NVM_P2 - 16384) / 536870912.0;
	data->PAR.f_PAR_P3 = data->NVM.i8_NVM_P3 / 4294967296.0;
	data->PAR.f_PAR_P4 = data->NVM.i8_NVM_P4 / 137438953472.0;
	data->PAR.f_PAR_P5 = data->NVM.u16_NVM_P5 / 0.125;
	data->PAR.f_PAR_P6 = data->NVM.u16_NVM_P6 / 64.0;
	data->PAR.f_PAR_P7 = data->NVM.i8_NVM_P7 / 256.0;
	data->PAR.f_PAR_P8 = data->NVM.i8_NVM_P8 / 32768.0;
	data->PAR.f_PAR_P9 = data->NVM.i16_NVM_P9 / 281474976710656.0;
	data->PAR.f_PAR_P10 = data->NVM.i8_NVM_P10 / 281474976710656.0;
	data->PAR.f_PAR_P11 = data->NVM.i8_NVM_P11 / 36893488147419103232.0;
}

//
void BMP390_set_mode(BMP390_Mode mode) {
	uint8_t reg_value;
	twi_start();

	// Send device address with write flag
	twi_write(BMP390_I2C_ADDR << 1);

	// Send register address of PWR_CTRL register
	twi_write(BMP390_REG_PWR_CTRL);

	// Send repeated start condition
	twi_start();

	// Send device address with read flag
	twi_write((BMP390_I2C_ADDR << 1) | 1);

	// Receive the current value of PWR_CTRL register
	reg_value = twi_read_nack();

	// Clear bits 5 and 4 (mode bits)
	reg_value &= ~(0x30);

	// Set mode bits according to input mode
	reg_value |= (mode << 4);
	
	reg_value |= (3 << 0);
	
	// Send stop condition
	twi_stop();

	// Write modified value back to PWR_CTRL register
	twi_start();
	twi_write(BMP390_I2C_ADDR << 1);
	twi_write(BMP390_REG_PWR_CTRL);
	twi_write(reg_value);
	twi_stop();
	return true;
}

// Function to initialize BMP390 sensor
bool BMP390_init(void) 
{
	BMP390_set_mode(BMP390_MODE_NORMAL);
	bool err = BMP390_read_raw_calibration(&TempPress_data);
	BMP390_convert_calibration(&TempPress_data);
	return err;
}

// Function to read temperature and pressure data from BMP390 sensor
void BMP390_read_raw_temp_press(BMP390_Data *data) {
	uint8_t temp_press_data[6];

	twi_start();

	// Send device address with write flag
	twi_write(BMP390_I2C_ADDR << 1);

	// Send register address of temperature and pressure data
	twi_write(BMP390_TEMP_PRESS_DATA_ADDR);

	// Send repeated start condition
	twi_start();

	// Send device address with read flag
	twi_write((BMP390_I2C_ADDR << 1) | 1);

	// Receive temperature and pressure data
	for (uint8_t i = 0; i < 5; i++) {
		temp_press_data[i] = twi_read_ack();
	}
	temp_press_data[5] = twi_read_nack();

	// Combine the bytes to form the temperature and pressure readings
	int32_t temp_raw = ((uint32_t)temp_press_data[5] << 16) | ((uint32_t)temp_press_data[4] << 8) | (uint32_t)temp_press_data[3];
	int32_t press_raw = ((uint32_t)temp_press_data[2] << 16) | ((uint32_t)temp_press_data[1] << 8) | (uint32_t)temp_press_data[0];

	// Store the raw readings into the BMP390_Data structure
	data->temperature_raw = temp_raw;
	data->pressure_raw = press_raw;
	// Send stop condition
	twi_stop();
}


// Function to compensate temperature reading
void BMP390_compensate_temperature(BMP390_Data *data) {
	float partial_data1;
	float partial_data2;
	
	partial_data1 = (float)(data->temperature_raw-data->PAR.f_PAR_T1);
	partial_data2 = (float)(partial_data1*data->PAR.f_PAR_T2);
	
	data->temperature = partial_data2+(partial_data1*partial_data1)*data->PAR.f_PAR_T3;
	//Pressure 10 times
	data->temperature *=10;
}

// Function to compensate pressure reading
void BMP390_compensate_pressure(BMP390_Data *data) {
	float partial_data1;
	float partial_data2;
	float partial_data3;
	float partial_data4;
	float partial_out1;
	float partial_out2;
	
	partial_data1=data->PAR.f_PAR_P6*(data->temperature);
	partial_data2=data->PAR.f_PAR_P7*(data->temperature*data->temperature);
	partial_data3=data->PAR.f_PAR_P8*(data->temperature*data->temperature*data->temperature);
	partial_out1=data->PAR.f_PAR_P5 + partial_data1 + partial_data2 + partial_data3;
	
	partial_data1=data->PAR.f_PAR_P2*data->temperature;
	partial_data2=data->PAR.f_PAR_P3*(data->temperature*data->temperature);
	partial_data3=data->PAR.f_PAR_P4*(data->temperature*data->temperature*data->temperature);
	partial_out2=(float)data->pressure_raw*(data->PAR.f_PAR_P1 + partial_data1 + partial_data2 + partial_data3);
	
	partial_data1=(float)data->pressure_raw*(float)data->pressure_raw;
	partial_data2=data->PAR.f_PAR_P9*data->PAR.f_PAR_P10*data->temperature;
	partial_data3=partial_data1*partial_data2;
	partial_data4=partial_data3 + ((float)data->pressure_raw*(float)data->pressure_raw*(float)data->pressure_raw)*data->PAR.f_PAR_P11;
	
	//partial_out2=(float)data->pressure_raw*(data->PAR.f_PAR_P1 + partial_data1 + partial_data2 + partial_data3);

	data->pressure = partial_out1 + partial_out2 + partial_data4;
	//Pressure in 10 times hPa, value is 10.000 mean 1.000 hPa (hPa = Pa/100, 10xhPa = Pa/10)
	data->pressure /=10.0;
}

void BMP390_temp_press_update(void)
{
	BMP390_init();
	BMP390_read_raw_temp_press(&TempPress_data);
	BMP390_compensate_temperature(&TempPress_data);
	BMP390_compensate_pressure(&TempPress_data);
}

int16_t	get_BMP390_temperature(void)
{
	return (int16_t)TempPress_data.temperature;
}
uint16_t get_BMP390_pressure(void)
{
	return (uint16_t)TempPress_data.pressure;
}