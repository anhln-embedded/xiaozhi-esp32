#include "led_rgb_strip.h"

#include <led_strip.h>
#include <esp_err.h>
#include <string.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const char* TAG = "LedRgbStrip";

// LED strip handle
static led_strip_handle_t led_strip = NULL;
static bool led_strip_initialized = false;

LedRgbStrip::LedRgbStrip()
    : pin_(GPIO_NUM_NC),
      num_leds_(0),
      initialized_(false),
      led_data_(nullptr),
      breathing_active_(false),
      rainbow_active_(false),
      effect_task_(nullptr) {
    ESP_LOGD(TAG, "LedRgbStrip object created");
}

LedRgbStrip::~LedRgbStrip() {
    if (effect_task_ != nullptr) {
        vTaskDelete(effect_task_);
        effect_task_ = nullptr;
    }

    if (led_data_ != nullptr) {
        free(led_data_);
        led_data_ = nullptr;
    }

    if (initialized_) {
        if (led_strip != NULL) {
            led_strip_del(led_strip);
            led_strip = NULL;
        }
        initialized_ = false;
    }

    ESP_LOGD(TAG, "LedRgbStrip object destroyed");
}

bool LedRgbStrip::Init(gpio_num_t pin, uint16_t num_leds) {
    ESP_LOGI(TAG, "Initializing LED RGB strip on GPIO %d with %d LEDs", pin, num_leds);

    if (num_leds == 0 || num_leds > 1000) {
        ESP_LOGE(TAG, "Invalid number of LEDs: %d (must be 1-1000)", num_leds);
        return false;
    }

    pin_ = pin;
    num_leds_ = num_leds;

    // Allocate LED data buffer (3 bytes per LED: RGB format)
    led_data_ = (uint8_t*)malloc(num_leds_ * 3);
    if (led_data_ == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate LED data buffer");
        return false;
    }

    // Clear LED data
    memset(led_data_, 0, num_leds_ * 3);

    // Initialize LED strip if not already done
    if (!led_strip_initialized) {
        // LED strip configuration
        led_strip_config_t strip_config = {
            .strip_gpio_num = pin_,
            .max_leds = num_leds_,
            .led_model = LED_MODEL_WS2812,
            .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,  // WS2812 uses GRB format
            .flags = {
                .invert_out = false,
            },
        };

        // RMT configuration
        led_strip_rmt_config_t rmt_config = {
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10 * 1000 * 1000,  // 10MHz resolution
            .mem_block_symbols = 0,  // Use default
            .flags = {
                .with_dma = false,
            },
        };

        esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create LED strip device: %s", esp_err_to_name(err));
            free(led_data_);
            led_data_ = nullptr;
            return false;
        }

        led_strip_initialized = true;
        ESP_LOGI(TAG, "LED strip device initialized successfully");
    }

    initialized_ = true;
    ESP_LOGI(TAG, "LED RGB strip initialized successfully");

    // Clear all LEDs initially
    Clear();
    Show();

    return true;
}

void LedRgbStrip::SetAllColor(uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    for (uint16_t i = 0; i < num_leds_; i++) {
        SetLedColor(i, red, green, blue);
    }
}

void LedRgbStrip::SetLedColor(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    if (index >= num_leds_) {
        ESP_LOGW(TAG, "LED index out of range: %d (max: %d)", index, num_leds_ - 1);
        return;
    }

    // WS2812B uses GRB format (not RGB)
    uint16_t data_index = index * 3;
    led_data_[data_index] = green;     // G
    led_data_[data_index + 1] = red;   // R
    led_data_[data_index + 2] = blue;  // B
}

void LedRgbStrip::SetRangeColor(uint16_t start_index, uint16_t end_index, uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    if (start_index > end_index || end_index >= num_leds_) {
        ESP_LOGW(TAG, "Invalid range: %d-%d (max: 0-%d)", start_index, end_index, num_leds_ - 1);
        return;
    }

    for (uint16_t i = start_index; i <= end_index; i++) {
        SetLedColor(i, red, green, blue);
    }
}

void LedRgbStrip::Clear() {
    if (!initialized_) {
        return;
    }

    memset(led_data_, 0, num_leds_ * 3);
}

void LedRgbStrip::Show() {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    // Set pixel colors using ESP-IDF LED strip API
    for (uint16_t i = 0; i < num_leds_; i++) {
        uint16_t data_index = i * 3;
        uint8_t green = led_data_[data_index];
        uint8_t red = led_data_[data_index + 1];
        uint8_t blue = led_data_[data_index + 2];

        esp_err_t err = led_strip_set_pixel(led_strip, i, red, green, blue);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set pixel %d: %s", i, esp_err_to_name(err));
        }
    }

    // Refresh the LED strip
    esp_err_t err = led_strip_refresh(led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh LED strip: %s", esp_err_to_name(err));
    }
}

uint32_t LedRgbStrip::ColorToUint32(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
}

void LedRgbStrip::Uint32ToColor(uint32_t color, uint8_t& red, uint8_t& green, uint8_t& blue) {
    red = (color >> 16) & 0xFF;
    green = (color >> 8) & 0xFF;
    blue = color & 0xFF;
}

void LedRgbStrip::StartBreathing(uint8_t red, uint8_t green, uint8_t blue, uint32_t period_ms) {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    breathing_red_ = red;
    breathing_green_ = green;
    breathing_blue_ = blue;
    breathing_period_ = period_ms;
    breathing_start_time_ = esp_timer_get_time() / 1000;  // milliseconds
    breathing_active_ = true;

    // Stop rainbow if active
    StopRainbow();

    // Create effect task if not exists
    if (effect_task_ == nullptr) {
        xTaskCreate(EffectTask, "led_effect", 2048, this, 5, &effect_task_);
    }

    ESP_LOGI(TAG, "Started breathing effect");
}

void LedRgbStrip::StopBreathing() {
    breathing_active_ = false;
    ESP_LOGI(TAG, "Stopped breathing effect");
}

void LedRgbStrip::StartRainbow(uint32_t period_ms) {
    if (!initialized_) {
        ESP_LOGW(TAG, "LED strip not initialized");
        return;
    }

    rainbow_period_ = period_ms;
    rainbow_start_time_ = esp_timer_get_time() / 1000;  // milliseconds
    rainbow_active_ = true;

    // Stop breathing if active
    StopBreathing();

    // Create effect task if not exists
    if (effect_task_ == nullptr) {
        xTaskCreate(EffectTask, "led_effect", 2048, this, 5, &effect_task_);
    }

    ESP_LOGI(TAG, "Started rainbow effect");
}

void LedRgbStrip::StopRainbow() {
    rainbow_active_ = false;
    ESP_LOGI(TAG, "Stopped rainbow effect");
}

void LedRgbStrip::UpdateBreathing() {
    if (!breathing_active_) return;

    uint32_t current_time = esp_timer_get_time() / 1000;
    uint32_t elapsed = current_time - breathing_start_time_;
    float phase = (float)(elapsed % breathing_period_) / breathing_period_;

    // Sine wave for smooth breathing effect
    float intensity = (sin(phase * 2 * M_PI - M_PI/2) + 1.0f) / 2.0f;  // 0 to 1

    uint8_t red = (uint8_t)(breathing_red_ * intensity);
    uint8_t green = (uint8_t)(breathing_green_ * intensity);
    uint8_t blue = (uint8_t)(breathing_blue_ * intensity);

    SetAllColor(red, green, blue);
    Show();
}

void LedRgbStrip::UpdateRainbow() {
    if (!rainbow_active_) return;

    uint32_t current_time = esp_timer_get_time() / 1000;
    uint32_t elapsed = current_time - rainbow_start_time_;
    float phase = (float)(elapsed % rainbow_period_) / rainbow_period_;

    // Rainbow colors: Red -> Orange -> Yellow -> Green -> Blue -> Indigo -> Violet
    for (uint16_t i = 0; i < num_leds_; i++) {
        float led_phase = phase + (float)i / num_leds_;  // Phase offset for each LED
        led_phase = fmod(led_phase, 1.0f);

        uint8_t red, green, blue;

        if (led_phase < 1.0f/6.0f) {
            // Red to Orange to Yellow (0-1/6)
            float t = led_phase * 6.0f;
            red = 255;
            green = (uint8_t)(255 * t);
            blue = 0;
        } else if (led_phase < 2.0f/6.0f) {
            // Yellow to Green (1/6-2/6)
            float t = (led_phase - 1.0f/6.0f) * 6.0f;
            red = (uint8_t)(255 * (1.0f - t));
            green = 255;
            blue = 0;
        } else if (led_phase < 3.0f/6.0f) {
            // Green to Blue (2/6-3/6)
            float t = (led_phase - 2.0f/6.0f) * 6.0f;
            red = 0;
            green = 255;
            blue = (uint8_t)(255 * t);
        } else if (led_phase < 4.0f/6.0f) {
            // Blue to Indigo (3/6-4/6)
            float t = (led_phase - 3.0f/6.0f) * 6.0f;
            red = 0;
            green = (uint8_t)(255 * (1.0f - t));
            blue = 255;
        } else if (led_phase < 5.0f/6.0f) {
            // Indigo to Violet (4/6-5/6)
            float t = (led_phase - 4.0f/6.0f) * 6.0f;
            red = (uint8_t)(128 * t);
            green = 0;
            blue = 255;
        } else {
            // Violet to Red (5/6-1)
            float t = (led_phase - 5.0f/6.0f) * 6.0f;
            red = (uint8_t)(128 + 127 * t);
            green = 0;
            blue = (uint8_t)(255 * (1.0f - t));
        }

        SetLedColor(i, red, green, blue);
    }

    Show();
}

void LedRgbStrip::EffectTask(void* param) {
    LedRgbStrip* strip = (LedRgbStrip*)param;

    while (true) {
        if (strip->breathing_active_) {
            strip->UpdateBreathing();
        } else if (strip->rainbow_active_) {
            strip->UpdateRainbow();
        } else {
            // No active effects, delay and check again
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Update effects at ~30 FPS
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}