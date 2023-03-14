#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "bno055.h"

//  sudo apt-get install -y libi2c-dev

int i2c_fd;

int8_t BNO055_I2C_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int8_t iError = BNO055_INIT_VALUE;
    uint8_t array[cnt];
    uint8_t stringpos;

    array[0] = reg_addr;
    if (write(i2c_fd, array, 1) != 1) {
        printf("Error writing to i2c slave\n");
        return BNO055_ERROR;
    }

    if (read(i2c_fd, array, cnt) != cnt) {
        printf("Error reading from i2c slave\n");
        return BNO055_ERROR;
    }
    else {
        for (stringpos = 0; stringpos < cnt; stringpos++) {
            *(reg_data + stringpos) = array[stringpos];
        }
    }
    return (s8)iError;
}

int8_t BNO055_I2C_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int8_t iError = BNO055_INIT_VALUE;
    uint8_t array[cnt];
    uint8_t stringpos;

    array[0] = reg_addr;
    for (stringpos = 0; stringpos < cnt; stringpos++) {
        array[stringpos + 1] = *(reg_data + stringpos);
    }
    if (write(i2c_fd, array, cnt + 1) != cnt + 1) {
        printf("Error writing to i2c slave\n");
        return BNO055_ERROR;
    }

    return (s8)iError;
}

void BNO055_delay_msek(uint32_t msek)
{
    usleep(msek * 1000);
}

int main() {

   i2c_fd = open("/dev/i2c-2", O_RDWR);
   if (i2c_fd < 0) {
        printf("Failed to open the bus.");
        return BNO055_ERROR;
   }

   if (ioctl(i2c_fd, I2C_SLAVE, BNO055_I2C_ADDR1) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        return BNO055_ERROR;
   }

   struct bno055_t bno;
   bno.bus_read = BNO055_I2C_bus_read;
   bno.bus_write = BNO055_I2C_bus_write;
   bno.delay_msec = BNO055_delay_msek;

   bno.dev_addr = BNO055_I2C_ADDR1;
   bno055_init(&bno);
   bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);

   while(1)
   {
       int16_t accel_x_s16 = 0, accel_y_s16 = 0, accel_z_s16 = 0;
       bno055_read_accel_x(&accel_x_s16);
       bno055_read_accel_y(&accel_y_s16);
       bno055_read_accel_z(&accel_z_s16);

       printf("Accelerometer X: %d\nAccelerometer Y: %d\nAccelerometer Z: %d\n", accel_x_s16, accel_y_s16, accel_z_s16);
   }

   return 0;
}
