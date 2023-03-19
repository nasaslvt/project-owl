#ifndef IMU_H
#define IMU_H

int imu_init();
void bno055_read_accel_norm(double *x, double *y, double *z);
void bno055_read_accel(double *x, double *y, double *z);

#endif
