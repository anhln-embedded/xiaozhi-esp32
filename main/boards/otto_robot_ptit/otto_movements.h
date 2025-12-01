#ifndef __OTTO_MOVEMENTS_H__
#define __OTTO_MOVEMENTS_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"

//-- Hằng số
#define FORWARD 1
#define BACKWARD -1
#define LEFT 1
#define RIGHT -1
#define BOTH 0
#define SMALL 5
#define MEDIUM 15
#define BIG 30

// -- Motor speed constants
#define MOTOR_MAX_SPEED 255
#define MOTOR_DEFAULT_SPEED 200

// -- Motor indices for easy access
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1
#define MOTOR_COUNT 2

class Otto {
public:
    Otto();
    ~Otto();

    //-- Otto initialization (no pins needed, uses config.h)
    void Init();
    
    //-- Attach & detach functions
    void AttachMotors();
    void DetachMotors();

    //-- HOME = Otto at rest position (stop motors)
    void Home(bool dummy = true);
    bool GetRestState();
    void SetRestState(bool state);

    //-- Basic Movement Functions for differential drive robot
    void Walk(float steps = 4, int period = 1000, int dir = FORWARD, int speed = 80);
    void Turn(float steps = 4, int period = 2000, int dir = LEFT, int speed = 80);

private:
    Motor motors_[MOTOR_COUNT];

    bool is_otto_resting_;
    bool has_motors_;  // Motors are attached

    void ExecuteMovement(int left_speed, int right_speed, int duration_ms);
    void StopMotors();
};

#endif  // __OTTO_MOVEMENTS_H__