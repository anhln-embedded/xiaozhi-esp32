#include "motor.h"

#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "Motor";

// Global initialization flag
static bool motor_driver_initialized = false;

// Function to initialize GPIO pins and PWM
void motor_init() {
    ESP_LOGI(TAG, "Starting motor_init() - GPIO and PWM");
    
    if (motor_driver_initialized) {
        ESP_LOGI(TAG, "Motor driver already initialized");
        return;
    }
    
    // Configure GPIO pins for direction control and standby
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << AIN1) | (1ULL << AIN2) | (1ULL << BIN1) | (1ULL << BIN2) | (1ULL << STBY),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t gpio_ret = gpio_config(&io_conf);
    if (gpio_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO pins: %s", esp_err_to_name(gpio_ret));
        return;
    }
    ESP_LOGI(TAG, "GPIO pins configured");

    // Initialize PWM timers for Motor A and B
    ledc_timer_config_t ledc_timer_a = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER_A,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_a));

    ledc_timer_config_t ledc_timer_b = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER_B,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_b));

    // Initialize PWM channels for Motor A and B
    ledc_channel_config_t ledc_channel_a = {
        .gpio_num = PWMA,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_A,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_A,
        .duty = 0,  // Start with 0% duty cycle (stopped)
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_a));

    ledc_channel_config_t ledc_channel_b = {
        .gpio_num = PWMB,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_B,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_B,
        .duty = 0,  // Start with 0% duty cycle (stopped)
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_b));

    ESP_LOGI(TAG, "PWM channels configured with 0% duty cycle");

    // Set all motor control pins to LOW initially (motors stopped)
    gpio_set_level(AIN1, 0);
    gpio_set_level(AIN2, 0);
    gpio_set_level(BIN1, 0);
    gpio_set_level(BIN2, 0);
    ESP_LOGI(TAG, "All motor control pins set to LOW");
    
    // Set STBY high to enable driver (after all pins are properly configured)
    gpio_set_level(STBY, 1);
    ESP_LOGI(TAG, "STBY pin set high - motor driver enabled");
    
    motor_driver_initialized = true;
    ESP_LOGI(TAG, "TB6612 Motor driver fully initialized successfully");
}

// Function to control motor A with PWM speed control
void motor_a_control(motor_direction_t direction, uint32_t speed) {
    ESP_LOGI(TAG, "Motor A control: direction=%d, speed=%lu", direction, speed);
    
    // Calculate PWM duty cycle (0-100% speed -> 0-8191 duty for 13-bit resolution)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 100;
    
    switch (direction) {
        case MOTOR_FORWARD:
            gpio_set_level(AIN1, 0);
            gpio_set_level(AIN2, 1);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A);
            ESP_LOGI(TAG, "Motor A: FORWARD, duty=%lu", duty);
            break;
        case MOTOR_BACKWARD:
            gpio_set_level(AIN1, 1);
            gpio_set_level(AIN2, 0);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A);
            ESP_LOGI(TAG, "Motor A: BACKWARD, duty=%lu", duty);
            break;
        case MOTOR_STOP:
        default:
            gpio_set_level(AIN1, 0);
            gpio_set_level(AIN2, 0);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, 0);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A);
            ESP_LOGI(TAG, "Motor A: STOP");
            break;
    }
}

// Function to control motor B with PWM speed control
void motor_b_control(motor_direction_t direction, uint32_t speed) {
    ESP_LOGI(TAG, "Motor B control: direction=%d, speed=%lu", direction, speed);
    
    // Calculate PWM duty cycle (0-100% speed -> 0-8191 duty for 13-bit resolution)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 100;
    
    switch (direction) {
        case MOTOR_FORWARD:
            gpio_set_level(BIN1, 1);
            gpio_set_level(BIN2, 0);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);
            ESP_LOGI(TAG, "Motor B: FORWARD, duty=%lu", duty);
            break;
        case MOTOR_BACKWARD:
            gpio_set_level(BIN1, 0);
            gpio_set_level(BIN2, 1);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);
            ESP_LOGI(TAG, "Motor B: BACKWARD, duty=%lu", duty);
            break;
        case MOTOR_STOP:
        default:
            gpio_set_level(BIN1, 0);
            gpio_set_level(BIN2, 0);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, 0);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);
            ESP_LOGI(TAG, "Motor B: STOP");
            break;
    }
}

// Motor class implementation
Motor::Motor() : is_attached_(false), motor_id_(-1), current_speed_(0) {
}

Motor::~Motor() {
    Detach();
}

void Motor::Attach(int motor_id) {
    if (is_attached_) {
        Detach();
    }
    
    // Initialize driver if not already done
    motor_init();
    
    motor_id_ = motor_id;
    is_attached_ = true;
    current_speed_ = 0;  // Ensure speed is 0
    
    // Make sure motor is stopped when attached
    Stop();
    
    ESP_LOGI(TAG, "Motor %s attached and stopped", (motor_id == 0) ? "A" : "B");
}

void Motor::Detach() {
    if (!is_attached_) {
        return;
    }
    
    Stop();
    is_attached_ = false;
    ESP_LOGI(TAG, "Motor %s detached", (motor_id_ == 0) ? "A" : "B");
}

void Motor::SetSpeed(int speed) {
    if (!is_attached_) {
        return;
    }
    
    // Clamp speed to valid range (-100 to 100)
    speed = std::max(-100, std::min(100, speed));
    current_speed_ = speed;
    
    motor_direction_t direction;
    uint32_t abs_speed;
    
    if (speed > 0) {
        direction = MOTOR_FORWARD;
        abs_speed = speed;
    } else if (speed < 0) {
        direction = MOTOR_BACKWARD;
        abs_speed = -speed;
    } else {
        direction = MOTOR_STOP;
        abs_speed = 0;
    }
    
    if (motor_id_ == 0) {
        motor_a_control(direction, abs_speed);
    } else {
        motor_b_control(direction, abs_speed);
    }
}

void Motor::Stop() {
    if (!is_attached_) {
        return;
    }
    
    current_speed_ = 0;
    
    if (motor_id_ == 0) {
        motor_a_control(MOTOR_STOP, 0);
    } else {
        motor_b_control(MOTOR_STOP, 0);
    }
}

void Motor::Brake() {
    Stop();  // Same as stop for this implementation
}