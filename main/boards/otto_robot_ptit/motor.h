#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

// Motor directions
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
} motor_direction_t;

// Initialize motor driver (call once before using motors)
// This configures GPIO pins, PWM timers, and enables the TB6612FNG driver
void motor_init();

// Direct control functions for motor A and B
// direction: MOTOR_FORWARD, MOTOR_BACKWARD, or MOTOR_STOP
// speed: 0-100 (percentage of maximum speed)
void motor_a_control(motor_direction_t direction, uint32_t speed);
void motor_b_control(motor_direction_t direction, uint32_t speed);

// Motor class for object-oriented motor control
class Motor {
public:
    Motor();
    ~Motor();

    // Initialize motor with motor ID (0 = Motor A, 1 = Motor B)
    // This will automatically initialize the motor driver if not already done
    void Attach(int motor_id);
    void Detach();

    // Set motor speed and direction
    // speed: -100 to 100 (negative = backward, positive = forward, 0 = stop)
    void SetSpeed(int speed);

    // Stop motor (set speed to 0)
    void Stop();

    // Get current speed setting
    int GetSpeed() const { return current_speed_; }

    // Check if motor is attached
    bool IsAttached() const { return is_attached_; }

    // Brake motor (same as stop for TB6612FNG)
    void Brake();

private:
    bool is_attached_;
    int motor_id_;  // 0 = Motor A, 1 = Motor B
    int current_speed_;  // Current speed setting (-100 to 100)
};

#endif  // __MOTOR_H__