#ifndef SERVO_H
#define SERVO_H

struct servo {
    uint32_t *pru;
    uint32_t *pru_dram;
    const char *ctrl_pin;
    int prunum;
    double zero;
};
typedef struct servo servo;

int init_servo(struct servo *servo);
void servo_zero(struct servo *servo);
//void servo_rotate(struct servo *servo, double value);
void servo_set_speed(struct servo *servo, double speed);
double servo_position(struct servo *servo);
int servo_cleanup(struct servo *servo);

#endif
