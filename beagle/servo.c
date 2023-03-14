#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/mman.h>

#include "servo.h"
#include "bbio/c_pwm.h"

#define PRU_ADDR   0x4A300000
#define PRU_LEN    0x80000
#define PRU0_DRAM  0x00000
#define PRU1_DRAM  0x02000

#define MAXSPEED 30

#define DCMIN 3
#define DCMAX 95.7

#define Kp 0.3
#define Ki 0.001
#define Kd 10

int init_servo(struct servo *servo) {
    BBIO_err err;

    err = pwm_start(servo->ctrl_pin, 7.5, 50, 0);
    if (err != BBIO_OK) {
        syslog(LOG_ERR, "Servo: pwm_start failed");
        return -1;
    }

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        syslog(LOG_ERR, "Servo: could not open /dev/mem");
        return 1;
    }

    servo->pru = mmap (0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (servo->pru == MAP_FAILED) {
        syslog(LOG_ERR, "Servo: could not map memory");
        return 1;
    }

    servo->pru_dram = servo->pru + ((servo->prunum ? PRU1_DRAM : PRU0_DRAM) + 0x200)/4;

    close(fd);

    syslog(LOG_DEBUG, "Servo: initialize_servo: OK");
    return 0;
}

double servo_position(struct servo *servo) {
    double duty_cycle = (double)servo->pru_dram[0] / (double)servo->pru_dram[1] * 100;
    double angle = (duty_cycle - DCMIN) * 360 / (DCMAX - DCMIN + 1);

    if (angle > 360) {
       exit(-1);
    }

    angle = angle - servo->zero;
    if (angle < 0) angle = 360 + angle;
    printf("%f\n", angle);
    return angle;
}

void servo_zero(struct servo *servo) {
    servo->zero = 0.0;
    servo->zero = servo_position(servo);
    printf("Servo Zero: %f\n", servo->zero);
}

void servo_rotate(struct servo *servo, double value) {
    double error = 1, integral, derivative, prev_error, output;
    while (1) {

        error = servo_position(servo) - value;
        if (error > 180) {
            error = -360 + error;
        } else if (error < -180) {
            error = 360 + error;
        }

        if (abs(error) <= 0.01 && output < 1) break;

        integral += error;
        derivative = error - prev_error;

        output = Kp * error + Ki * integral + Kd * derivative;
        printf("%f,", output);
        servo_set_speed(servo, output);

        prev_error = error;
    }

    servo_set_speed(servo, 0);
}

void servo_set_speed(struct servo *servo, double speed) {

    if (speed > MAXSPEED) speed = MAXSPEED;
    if (speed < -MAXSPEED) speed = -MAXSPEED;

    if (speed == 0.0) {
        pwm_set_duty_cycle(servo->ctrl_pin, 7.5);
    } else if (speed < 0.0) {
        pwm_set_duty_cycle(servo->ctrl_pin, 7.4 + (speed/140.0));
    } else {
        pwm_set_duty_cycle(servo->ctrl_pin, 7.6 + (speed/140.0));
    }
}

int servo_cleanup(struct servo *servo) {

    pwm_disable(servo->ctrl_pin);
    munmap(servo->pru, PRU_LEN);
    syslog(LOG_DEBUG, "Servo: close_servo: OK");
    return 0;
}
