/*
 * Accel_Gyro.h
 *
 * Created: 6/14/2024 12:26:51 AM
 *  Author: HTSANG
 */ 


#ifndef ACCEL_GYRO_H_
#define ACCEL_GYRO_H_

#include "LSM6DSOX.h"
#include <stdbool.h>

bool Accel_and_Gyro_init(void);
void Accel_and_Gyro_create_task(void);
Accel_Gyro_DataTypedef get_acceleration(void);
Accel_Gyro_DataTypedef get_gyroscope(void);

#endif /* ACCEL_GYRO_H_ */