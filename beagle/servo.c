#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/mman.h>

#include "servo.h"
#include "bbio/c_pwm.h"

#define PRU_ADDR   0x4A300000
#define PRU_LEN    0x80000
#define PRU0_DRAM  0x00000
#define PRU1_DRAM  0x02000

#define MAXSPEED 140

#define DCMIN 3
#define DCMAX 95.7

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

    double high_time = (double)servo->pru_dram[0];
    double total_time = (double)servo->pru_dram[1];

    if (total_time == 0) return 0;

    double duty_cycle = high_time/total_time * 100;
    double angle = (duty_cycle - DCMIN) * 360 / (DCMAX - DCMIN + 1);

    angle = (angle - servo->zero) % 360;

    printf("%f\n", angle);
    return angle;
}

void servo_zero(struct servo *servo) {
    servo->zero = 0.0;
    servo->zero = servo_position(servo);
    syslog(LOG_DEBUG, "Servo: servo_zero: %f\n", servo->zero);
}

/*void servo_rotate(struct servo *servo, double value) {

    double error = 0, prev_error = 0, output = 0;
    clock_t start_time = clock();

    while (1) {

        error = servo_position(servo) - value;
        if (error > 180) {
            error = -360 + error;
        } else if (error < -180) {
            error = 360 + error;
        }

        if (abs(error) <= 0.01) {
            servo_set_speed(servo, 0);
            start_time = clock();
            while (clock() - start_time < 0.1 * CLOCKS_PER_SEC);

            if (abs(servo_position(servo) - value) < 0.01) break;
        }

        if (clock() - start_time >= 0.1 * CLOCKS_PER_SEC)
        {
            if (abs(error - prev_error) < 0.01) {
                if (error > 0) { if (output > 0) output += 1; else output = 1;}
                if (error < 0) { if (output < 0) output -= 1; else output = -1;}
                servo_set_speed(servo, output);
            } else {
                output = 0;
            }

            prev_error = error;
            start_time = clock();
        }
    }
}*/

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
