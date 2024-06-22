/*
 * LSM6DSOX.c
 *
 * Created: 6/14/2024 1:19:01 AM
 *  Author: HTSANG
 */ 
#define F_CPU 8000000UL
#include "LSM6DSOX.h"
#include "math.h"
#include "twi.h"
#include "IOU_board.h"
#include "uart.h"

bool lsm6dsox_write_register(uint8_t _reg, uint8_t _value) {
	twi_start();
	twi_write(LSM6DSOX_ADDRESS << 1);
	twi_write(_reg);
	twi_write(_value);
	twi_stop();
	return true;
}

uint8_t result;

uint8_t lsm6dsox_read_register(uint8_t _reg) {
	twi_start();
	twi_write(LSM6DSOX_ADDRESS << 1);
	twi_write(_reg);
	twi_start();
	twi_write((LSM6DSOX_ADDRESS << 1) | 0x01);
	result = twi_read_nack();
	twi_stop();
	return result;
}

bool lsm6dsox_init(void) {
	// Accelerometer: 104 Hz, 2g
	lsm6dsox_write_register(LSM6DSOX_CTRL1_XL, 0x40);
	// Gyroscope: 104 Hz, 250 dps
	lsm6dsox_write_register(LSM6DSOX_CTRL2_G, 0x40);
	if (!lsm6dsox_read_register(LSM6DSOX_OUTZ_H_A))
		return false;
	return true;
}


void read_accel(Accel_Gyro_DataTypedef* _accel)
{
	uint8_t ax_low	= lsm6dsox_read_register(LSM6DSOX_OUTX_L_A);
	uint8_t ax_high = lsm6dsox_read_register(LSM6DSOX_OUTX_H_A);
	uint8_t ay_low	= lsm6dsox_read_register(LSM6DSOX_OUTY_L_A);
	uint8_t ay_high = lsm6dsox_read_register(LSM6DSOX_OUTY_H_A);
	uint8_t az_low	= lsm6dsox_read_register(LSM6DSOX_OUTZ_L_A);
	uint8_t az_high = lsm6dsox_read_register(LSM6DSOX_OUTZ_H_A);
	
	_accel->x = (int16_t)(ax_high << 8 | ax_low) * LSM6DSOX_ACCL_FS_2G;
	_accel->y = (int16_t)(ay_high << 8 | ay_low) * LSM6DSOX_ACCL_FS_2G;
	_accel->z = (int16_t)(az_high << 8 | az_low) * LSM6DSOX_ACCL_FS_2G;
}
void read_gyro(Accel_Gyro_DataTypedef* _gyro)
{
	uint8_t gx_low	= lsm6dsox_read_register(LSM6DSOX_OUTX_L_G);
	uint8_t gx_high = lsm6dsox_read_register(LSM6DSOX_OUTX_H_G);
	uint8_t gy_low	= lsm6dsox_read_register(LSM6DSOX_OUTY_L_G);
	uint8_t gy_high	= lsm6dsox_read_register(LSM6DSOX_OUTY_H_A);
	uint8_t gz_low	= lsm6dsox_read_register(LSM6DSOX_OUTZ_L_A);
	uint8_t gz_high	= lsm6dsox_read_register(LSM6DSOX_OUTZ_H_A);

	_gyro->x = (int16_t)(gx_high << 8 | gx_low) * GYRO_SENSITIVITY_250DPS / 1000.0f;  // convert mdps to dps
	_gyro->y = (int16_t)(gy_high << 8 | gy_low) * GYRO_SENSITIVITY_250DPS / 1000.0f;  // convert mdps to dps
	_gyro->z = (int16_t)(gz_high << 8 | gz_low) * GYRO_SENSITIVITY_250DPS / 1000.0f;  // convert mdps to dps
}