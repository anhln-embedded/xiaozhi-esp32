#include "otto_movements.h"

#include <algorithm>

static const char* TAG = "OttoMovements";

Otto::Otto() {
    is_otto_resting_ = false;
    has_motors_ = false;
}

Otto::~Otto() {
    DetachMotors();
}

unsigned long IRAM_ATTR millis() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

void Otto::Init() {
    is_otto_resting_ = false;
    AttachMotors();
    Home();  // Ensure Otto starts in rest position (motors stopped)
    ESP_LOGI(TAG, "Otto initialized with motors in rest position");
}

///////////////////////////////////////////////////////////////////
//-- ATTACH & DETACH FUNCTIONS ----------------------------------//
///////////////////////////////////////////////////////////////////
void Otto::AttachMotors() {
    if (has_motors_) {
        ESP_LOGI(TAG, "Motors already attached");
        return;
    }
    
    // Attach Motor A (left motor) and Motor B (right motor)
    ESP_LOGI(TAG, "Attaching motors...");
    
    motors_[LEFT_MOTOR].Attach(0);  // Motor A
    ESP_LOGI(TAG, "Motor A attached");
    
    motors_[RIGHT_MOTOR].Attach(1); // Motor B
    ESP_LOGI(TAG, "Motor B attached");
    
    has_motors_ = true;
    ESP_LOGI(TAG, "All motors attached for Otto robot");
}

void Otto::DetachMotors() {
    motors_[LEFT_MOTOR].Detach();
    motors_[RIGHT_MOTOR].Detach();
    has_motors_ = false;
    
    ESP_LOGI(TAG, "Motors detached");
}

///////////////////////////////////////////////////////////////////
//-- BASIC MOTOR FUNCTIONS -------------------------------------//
///////////////////////////////////////////////////////////////////
void Otto::ExecuteMovement(int left_speed, int right_speed, int duration_ms) {
    if (!has_motors_) {
        ESP_LOGW(TAG, "Motors not attached, skipping movement");
        return;
    }
    
    // Convert speed from 0-255 range to 0-100 range for Motor class
    int scaled_left = (left_speed * 100) / 255;
    int scaled_right = (right_speed * 100) / 255;
    
    // Clamp to valid range
    scaled_left = std::max(-100, std::min(100, scaled_left));
    scaled_right = std::max(-100, std::min(100, scaled_right));
    
    motors_[LEFT_MOTOR].SetSpeed(scaled_left);
    motors_[RIGHT_MOTOR].SetSpeed(scaled_right);
    
    if (duration_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        StopMotors();
    }
}

void Otto::StopMotors() {
    if (!has_motors_) {
        return;
    }
    
    motors_[LEFT_MOTOR].Stop();
    motors_[RIGHT_MOTOR].Stop();
}

///////////////////////////////////////////////////////////////////
//-- HOME = Otto at rest position -------------------------------//
///////////////////////////////////////////////////////////////////
void Otto::Home(bool dummy) {
    if (is_otto_resting_ == false) {  // Go to rest position only if necessary
        StopMotors();
        is_otto_resting_ = true;
    }
}

bool Otto::GetRestState() {
    return is_otto_resting_;
}

void Otto::SetRestState(bool state) {
    is_otto_resting_ = state;
}

///////////////////////////////////////////////////////////////////
//-- PREDETERMINED MOTION SEQUENCES -----------------------------//
///////////////////////////////////////////////////////////////////

//---------------------------------------------------------
//-- Otto gait: Walking (forward or backward)
//--  Parameters:
//--    * steps: Number of steps
//--    * period: Period between movements
//--    * dir: Direction: FORWARD / BACKWARD
//--    * speed: Motor speed (0-255)
//---------------------------------------------------------
void Otto::Walk(float steps, int period, int dir, int speed) {
    int num_steps = (int)steps;
    // Convert speed percentage (0-100%) to 0-255 range, apply direction
    int motor_speed = dir * (speed * 255 / 100);
    
    for (int i = 0; i < num_steps; i++) {
        ExecuteMovement(motor_speed, motor_speed, period);
    }
}

//---------------------------------------------------------
//-- Otto gait: Turning (left or right)
//--  Parameters:
//--   * steps: Number of turn steps
//--   * period: Period of each turn
//--   * dir: Direction: LEFT / RIGHT
//--   * speed: Motor speed (0-255)
//---------------------------------------------------------
void Otto::Turn(float steps, int period, int dir, int speed) {
    int num_steps = (int)steps;
    // Convert speed percentage (0-100%) to 0-255 range
    int motor_speed = speed * 255 / 100;
    
    for (int i = 0; i < num_steps; i++) {
        if (dir == LEFT) {
            // Turn left: left motor backward, right motor forward
            ExecuteMovement(-motor_speed, motor_speed, period);
        } else {
            // Turn right: left motor forward, right motor backward
            ExecuteMovement(motor_speed, -motor_speed, period);
        }
    }
}



















