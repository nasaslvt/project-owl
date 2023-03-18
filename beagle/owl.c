#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "tcp.h"
#include "servo.h"
#include "imu/bno055.h"

#define KISSPORT 8001
#define LOCALHOST "127.0.0.1"

#define CAMERAPORT 8080
#define CAMERAIP "192.168.8.10"

int i2c_fd;
struct bno055_t bno;

servo rotation_servo;
servo swivel_servo;

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

int init_imu() {

   i2c_fd = open("/dev/i2c-2", O_RDWR);
   if (i2c_fd < 0) {
        printf("Failed to open the bus.");
        return BNO055_ERROR;
   }

   if (ioctl(i2c_fd, I2C_SLAVE, BNO055_I2C_ADDR1) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        return BNO055_ERROR;
   }

   bno.bus_read = BNO055_I2C_bus_read;
   bno.bus_write = BNO055_I2C_bus_write;
   bno.delay_msec = BNO055_delay_msek;

   bno.dev_addr = BNO055_I2C_ADDR1;
   bno055_init(&bno);
   bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);

   return BNO055_SUCCESS;
}

int init_conn(int port, const char *addr) {

    int fd = create_tcp_socket();
    if (fd < 0) {
        fprintf(stderr, "%s : Failed to create tcp socket\n", addr);
        return -1;
    }

    sockaddr_in server = create_server(KISSPORT, LOCALHOST);

    if (connect(fd, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        fprintf(stderr, "%s : Failed to connect\n", addr);
        return -1;
    }
    printf("%s : Connection succesful.\n", addr);

    return fd;
}

void swivel(double value) {

    double error = 0, prev_error = 0, output = 0;
    clock_t start_time = clock();

    while (1) {

        error = servo_position(&swivel_servo) - value;
        if (error > 180) {
            error = -360 + error;
        } else if (error < -180) {
            error = 360 + error;
        }

        if (fabs(error) <= 0.5) {
            servo_set_speed(&swivel_servo, 0);
            start_time = clock();
            while (clock() - start_time < 0.1 * CLOCKS_PER_SEC);

            if (fabs(servo_position(&swivel_servo) - value) < 0.5) break;
        }

        if (clock() - start_time >= 0.1 * CLOCKS_PER_SEC)
        {
            if (output * error < 0) {
                output *= -1;
                servo_set_speed(&swivel_servo, output);
            }

            if (fabs(error - prev_error) < 0.01) {
                if (error > 0) output += 1;
                if (error < 0) output -= 1;
                servo_set_speed(&swivel_servo, output);
            } else {
                output = 0;
            }

            prev_error = error;
            start_time = clock();
        }
    }
}

#define PI 3.14159265358979323846

void self_right() {

    int16_t accel_x_s16 = 0, accel_y_s16 = 0, accel_z_s16 = 0;
    double error = 0, prev_error = 0, output = 0, integral = 0, derivative = 0, norm = 0, x = 0, y = 0, z = 0;

    while (1) {
        bno055_read_accel_x(&accel_x_s16); bno055_read_accel_y(&accel_y_s16); bno055_read_accel_z(&accel_z_s16);
        norm = sqrt(accel_x_s16 * accel_x_s16 + accel_y_s16 * accel_y_s16 + accel_z_s16 * accel_z_s16);

        x = accel_x_s16/norm; y = accel_y_s16/norm; z = accel_z_s16/norm;

        error = atan2(y, -z) * 180/PI;

        if (fabs(x) > 0.92) {
           break;
        }

        if (fabs(error) < 10) {
            integral = prev_error = error = 0;
        }

        if (fabs(error) < 0.1) {
            break;
        }

        integral += error;
        derivative = error - prev_error;

        output = 3 * error + 0 * integral + 0 * derivative;
        servo_set_speed(&rotation_servo, output);

        prev_error = error;
    }

    servo_set_speed(&rotation_servo, 0);

}

void rotate() {

}

int main() {

    int res = 0;

    int camera_socket_fd = init_conn(CAMERAPORT, CAMERAIP);
    int radio_socket_fd = init_conn(KISSPORT, LOCALHOST);

    init_imu();

    swivel_servo.ctrl_pin = "P8_13";
    swivel_servo.prunum = 0;
    init_servo(&swivel_servo);
    servo_zero(&swivel_servo);

    rotation_servo.ctrl_pin = "P9_16";
    rotation_servo.prunum = 1;
    init_servo(&rotation_servo);

    char request[256];
    char response[256];

    strcpy(request, "GET / HTTP/1.1\r\nHost: 192.168.8.10:8000\r\n\r\n");
    write_string(camera_socket_fd, request);
    read_string(camera_socket_fd, response);

    float f;
    while (1) {
        res = read_buffer(radio_socket_fd, response, 256);
        if (res < 0) {
            //fprintf(stderr, "Failed reading\n");
        }
        scanf("%f", &f);
        swivel(f);
        self_right();
    }

    close(radio_socket_fd);
    close(camera_socket_fd);
    return 0;
}