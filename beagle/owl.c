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
#include "imu/bno_support.h"

#define KISSPORT 8001
#define LOCALHOST "127.0.0.1"

#define CAMERAPORT 8080
#define CAMERAIP "192.168.8.10"

#define PI 3.14159265358979323846

servo rotation_servo;
servo swivel_servo;

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

// Calculate Error (-180 to 180)
float swivel_err(double value) {
    double error = servo_position(&swivel_servo) - value;
    error = fmod(error + 360, 360.0);
    if (error > 180) {
        error -= 360;
    }
    return error;
}

void swivel(double value) {

    double error = 0, prev_error = 0, output = 0;
    clock_t start_time = clock();

    while (1) {

        error = swivel_err(value);

        // Exist Condition
        if (fabs(error) <= 1) {
            servo_set_speed(&swivel_servo, 0);

            start_time = clock();
            while (clock() - start_time < CLOCKS_PER_SEC/10);

            if (fabs(swivel_err(value)) <= 1) break;
        }

        if (clock() - start_time >= CLOCKS_PER_SEC/100)
        {
            //printf("derivative: %f error: %f output %f\n", error - prev_error, error, output);
            if (output * error < 0) {
                output = 0;
                servo_set_speed(&swivel_servo, output);
            }

            if (fabs(error - prev_error) < 1) {
                if (error > 0) output += 0.1;
                if (error < 0) output -= 0.1;
                servo_set_speed(&swivel_servo, output);
            }

            prev_error = error;
            start_time = clock();
        }
    }
}

void self_right() {

    double error = 0, prev_error = 0, output = 0, integral = 0, derivative = 0;
    double x = 0, y = 0, z = 0;
    clock_t start_time = clock();

    while (1) {

        bno055_read_accel_norm(&x, &y, &z);

        error = atan2(y, -z) * 180/PI;

        if (fabs(x) > 0.92 || fabs(error) < 0.1) {
            servo_set_speed(&rotation_servo, 0);

            start_time = clock();
            while (clock() - start_time < CLOCKS_PER_SEC);

            bno055_read_accel_norm(&x, &y, &z);
            if (fabs(x) > 0.92 || fabs(error) < 0.1) break;
        }

        integral += error;
        derivative = error - prev_error;

        output = 3 * error + 0.0001 * integral + 0 * derivative;
        servo_set_speed(&rotation_servo, output);

        prev_error = error;
    }

    servo_set_speed(&rotation_servo, 0);
}

void rotate(double value) {

    self_right();
    servo_zero(&rotation_servo);

    int reverse = value < 0;

    double error = 0, prev_error = 0, output = 0, position = 0;
    clock_t start_time = clock();

    value = fabs(value);
    value /= .316;

    while (1) {

        double position_temp = servo_position(&rotation_servo);
        if (reverse) {
            position_temp = 360 - position_temp;
        }
        if (fabs(position_temp - position) < 180) {
            position = position_temp;
        }

        error = position - value;
        if (reverse) error *= -1;

        if (fabs(error) <= 4) {
            servo_set_speed(&rotation_servo, 0);

            start_time = clock();
            while (clock() - start_time < CLOCKS_PER_SEC/10);

            if (reverse && fabs((360 - servo_position(&rotation_servo)) - value) < 4) break;
            else if (fabs(servo_position(&rotation_servo) - value) < 4) break;
        }

        if (clock() - start_time >= CLOCKS_PER_SEC/100)
        {
            //printf("derivative: %f position %f error: %f output %f\n", error - prev_error, position, error, output);
            if (output * error < 0) {
                output = 0;
                servo_set_speed(&rotation_servo, output);
            }

            if (fabs(error - prev_error) < 4) {
                if (error > 0) output += 0.1;
                if (error < 0) output -= 0.1;
                servo_set_speed(&rotation_servo, output);
            }

            prev_error = error;
            start_time = clock();
        }
    }
}

int main() {

    int camera_socket_fd = init_conn(CAMERAPORT, CAMERAIP);
    int radio_socket_fd = init_conn(KISSPORT, LOCALHOST);

    imu_init();

    swivel_servo.ctrl_pin = "P9_16";
    swivel_servo.prunum = 0;
    init_servo(&swivel_servo);
    servo_zero(&swivel_servo);

    rotation_servo.ctrl_pin = "P8_13";
    rotation_servo.prunum = 1;
    init_servo(&rotation_servo);

    char request[256];
    char response[256];

    strcpy(request, "GET / HTTP/1.1\r\nHost: 192.168.8.10:8000\r\n\r\n");
    write_string(camera_socket_fd, request);
    read_string(camera_socket_fd, response);

    float angle;
    servo_zero(&swivel_servo);
    while (1) {
        /*res = read_buffer(radio_socket_fd, response, 256);
        if (res < 0) {
            //fprintf(stderr, "Failed reading\n");
        }*/
        scanf("%f", &angle);
        swivel(angle);
    }

    close(radio_socket_fd);
    close(camera_socket_fd);
    return 0;
}
