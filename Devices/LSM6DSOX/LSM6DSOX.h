/*
 * LSM6DSOX.h
 *
 * Created: 6/14/2024 1:18:06 AM
 *  Author: HTSANG
 */ 


#ifndef LSM6DSOX_H_
#define LSM6DSOX_H_

#include <stdio.h>
#include <stdbool.h>

#define LSM6DSOX_ADDRESS            0x6A

#define LSM6DSOX_CTRL1_XL           0X10
#define LSM6DSOX_CTRL2_G            0X11

#define LSM6DSOX_STATUS_REG         0X1E

#define LSM6DSOX_CTRL3_C            0X12
#define LSM6DSOX_CTRL6_C            0X15
#define LSM6DSOX_CTRL7_G            0X16
#define LSM6DSOX_CTRL8_XL           0X17

#define LSM6DSOX_OUTX_L_G           0X22
#define LSM6DSOX_OUTX_H_G           0X23
#define LSM6DSOX_OUTY_L_G           0X24
#define LSM6DSOX_OUTY_H_G           0X25
#define LSM6DSOX_OUTZ_L_G           0X26
#define LSM6DSOX_OUTZ_H_G           0X27

#define LSM6DSOX_OUTX_L_A			0X28
#define LSM6DSOX_OUTX_H_A			0X29
#define LSM6DSOX_OUTY_L_A			0X2A
#define LSM6DSOX_OUTY_H_A			0X2B
#define LSM6DSOX_OUTZ_L_A			0X2C
#define LSM6DSOX_OUTZ_H_A			0X2D

#define GYRO_SENSITIVITY_250DPS		8.75f
#define LSM6DSOX_ACCL_FS_2G			0.061f

typedef struct _Accel_Gyro_DataTypedef_
{
	int16_t x;
	int16_t y;
	int16_t z;
} Accel_Gyro_DataTypedef;

bool lsm6dsox_write_register(uint8_t _reg, uint8_t _value);
uint8_t lsm6dsox_read_register(uint8_t _reg);
bool lsm6dsox_init(void);
void read_accel(Accel_Gyro_DataTypedef* _accel);
void read_gyro(Accel_Gyro_DataTypedef* _gyro);

#endif /* LSM6DSOX_H_ */