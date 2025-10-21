#ifndef __OTTO_MOVEMENTS_H__
#define __OTTO_MOVEMENTS_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oscillator.h"

//-- Hằng số
#define FORWARD 1
#define BACKWARD -1
#define LEFT 1
#define RIGHT -1
#define BOTH 0
#define SMALL 5
#define MEDIUM 15
#define BIG 30

// -- Giới hạn delta servo mặc định. độ / giây
#define SERVO_LIMIT_DEFAULT 240

// -- Chỉ số servo để truy cập dễ dàng
#define LEFT_LEG 0
#define RIGHT_LEG 1
#define LEFT_FOOT 2
#define RIGHT_FOOT 3
#define LEFT_HAND 4
#define RIGHT_HAND 5
#define SERVO_COUNT 6

class Otto {
public:
    Otto();
    ~Otto();

    //-- Otto initialization
    void Init(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand = -1,
              int right_hand = -1);
    //-- Attach & detach functions
    void AttachServos();
    void DetachServos();

    //-- Oscillator Trims
    void SetTrims(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand = 0,
                  int right_hand = 0);

    //-- Predetermined Motion Functions
    void MoveServos(int time, int servo_target[]);
    void MoveSingle(int position, int servo_number);
    void OscillateServos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                         double phase_diff[SERVO_COUNT], float cycle);

    //-- HOME = Otto at rest position
    void Home(bool hands_down = true);
    bool GetRestState();
    void SetRestState(bool state);

    //-- Predetermined Motion Functions
    void Jump(float steps = 1, int period = 2000);

    void Walk(float steps = 4, int period = 1000, int dir = FORWARD, int amount = 0);
    void Turn(float steps = 4, int period = 2000, int dir = LEFT, int amount = 0);
    void Bend(int steps = 1, int period = 1400, int dir = LEFT);
    void ShakeLeg(int steps = 1, int period = 2000, int dir = RIGHT);

    void UpDown(float steps = 1, int period = 1000, int height = 20);
    void Swing(float steps = 1, int period = 1000, int height = 20);
    void TiptoeSwing(float steps = 1, int period = 900, int height = 20);
    void Jitter(float steps = 1, int period = 500, int height = 20);
    void AscendingTurn(float steps = 1, int period = 900, int height = 20);

    void Moonwalker(float steps = 1, int period = 900, int height = 20, int dir = LEFT);
    void Crusaito(float steps = 1, int period = 900, int height = 20, int dir = FORWARD);
    void Flapping(float steps = 1, int period = 1000, int height = 20, int dir = FORWARD);

    // -- Hành động tay
    void HandsUp(int period = 1000, int dir = 0);      // Giơ cả hai tay
    void HandsDown(int period = 1000, int dir = 0);    // Hạ cả hai tay
    void HandWave(int period = 1000, int dir = LEFT);  // Vẫy tay
    void HandWaveBoth(int period = 1000);              // Vẫy cả hai tay cùng lúc

    // -- Servo limiter
    void EnableServoLimit(int speed_limit_degree_per_sec = SERVO_LIMIT_DEFAULT);
    void DisableServoLimit();

private:
    Oscillator servo_[SERVO_COUNT];

    int servo_pins_[SERVO_COUNT];
    int servo_trim_[SERVO_COUNT];

    unsigned long final_time_;
    unsigned long partial_time_;
    float increment_[SERVO_COUNT];

    bool is_otto_resting_;
    bool has_hands_;  // Có servo tay hay không

    void Execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                 double phase_diff[SERVO_COUNT], float steps);
};

#endif  // __OTTO_MOVEMENTS_H__