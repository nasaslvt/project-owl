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

    double error = 0, prev_error = 0, output = 0, integral = 0, derivative = 0;
    double x = 0, y = 0, z = 0;
    //clock_t start_time = clock();

    while (1) {

        bno055_read_accel_norm(&x,&y,&z);

        error = atan2(y, -z) * 180/PI;

        if (fabs(x) > 0.92) {
           break;
        }

        printf("Error: %f\n", error);

        /*if (fabs(error) < 10) {
            integral = prev_error = error = 0;
        }*/

        /*if (fabs(error) <= 0.01) {
            servo_set_speed(&rotation_servo, 0);
            start_time = clock();
            while (clock() - start_time < 1 * CLOCKS_PER_SEC);

            error = atan2(y, -z) * 180/PI;
            if (error < 0.01) break;
        }*/

        integral += error;
        derivative = error - prev_error;

        output = 3 * error + 0.01 * integral + 0 * derivative;
        servo_set_speed(&rotation_servo, output);

        prev_error = error;
    }

    servo_set_speed(&rotation_servo, 0);

}

void rotate(double value) {
    double error = 0, prev_error = 0, output = 0;
    clock_t start_time = clock();

    value = value / 0.316;

    while (1) {

        error = servo_position(&rotation_servo) - value;
        if (error > 360) {
            error = 360;
        } else if (error < -360) {
            error = -360;
        }

        printf("Error: %f %f\n", error, output);

        if (fabs(error) <= 0.5) {
            servo_set_speed(&rotation_servo, 0);
            start_time = clock();
            while (clock() - start_time < 0.1 * CLOCKS_PER_SEC);

            if (fabs(servo_position(&rotation_servo) - value) < 0.5) break;
        }

        if (clock() - start_time >= 0.1 * CLOCKS_PER_SEC)
        {
            if (output * error < 0) {
                output *= -1;
                servo_set_speed(&rotation_servo, output);
            }

            if (fabs(error - prev_error) < 0.01) {
                if (error > 0) output += 1;
                if (error < 0) output -= 1;
                servo_set_speed(&rotation_servo, output);
            } else {
                //output = 0;
            }

            prev_error = error;
            start_time = clock();
        }
    }
}

int main() {

    int res = 0;

    int camera_socket_fd = init_conn(CAMERAPORT, CAMERAIP);
    int radio_socket_fd = init_conn(KISSPORT, LOCALHOST);

    imu_init();

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

    float swivelangle;
    float rotateangle;
    while (1) {
        res = read_buffer(radio_socket_fd, response, 256);
        if (res < 0) {
            //fprintf(stderr, "Failed reading\n");
        }
        scanf("%f %f", &swivelangle, &rotateangle);
        printf("Self Right\n");
        self_right();
        printf("Swivel %f\n", swivelangle);
        swivel(swivelangle);
        servo_zero(&rotation_servo);
        printf("Rotate %f\n", rotateangle);
    }

    close(radio_socket_fd);
    close(camera_socket_fd);
    return 0;
}