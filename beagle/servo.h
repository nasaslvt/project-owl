#ifndef SERVO_H
#define SERVO_H

// Struct to store servo information
struct servo {
    uint32_t *pru; // Pointer to PRU memory
    uint32_t *pru_dram; // Pointer to PRU DRAM memory
    const char *ctrl_pin; // Control pin for the servo
    int prunum; // PRU number
    double zero; // Zero position of the servo
};
typedef struct servo servo;

// Initialize the servo
int init_servo(struct servo *servo);

// Set the current position as its zero position
void servo_zero(struct servo *servo);

// Set the speed of the servo
void servo_set_speed(struct servo *servo, double speed);

// Get the current position of the servo
double servo_position(struct servo *servo);

// Cleanup the servo
int servo_cleanup(struct servo *servo);

#endif