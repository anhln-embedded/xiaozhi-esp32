#ifndef __LED_RGB_STRIP_H__
#define __LED_RGB_STRIP_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// LED RGB Strip class for controlling WS2812B LED strips
class LedRgbStrip {
public:
    LedRgbStrip();
    ~LedRgbStrip();

    // Initialize LED strip on specified GPIO pin with number of LEDs
    bool Init(gpio_num_t pin, uint16_t num_leds);

    // Set color for all LEDs (RGB values 0-255)
    void SetAllColor(uint8_t red, uint8_t green, uint8_t blue);

    // Set color for individual LED (0-based index)
    void SetLedColor(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);

    // Set color for a range of LEDs
    void SetRangeColor(uint16_t start_index, uint16_t end_index, uint8_t red, uint8_t green, uint8_t blue);

    // Clear all LEDs (turn off)
    void Clear();

    // Show current LED colors (must call after setting colors)
    void Show();

    // Get number of LEDs
    uint16_t GetNumLeds() const { return num_leds_; }

    // Check if strip is initialized
    bool IsInitialized() const { return initialized_; }

    // Predefined color functions
    void SetRed() { SetAllColor(255, 0, 0); Show(); }
    void SetGreen() { SetAllColor(0, 255, 0); Show(); }
    void SetBlue() { SetAllColor(0, 0, 255); Show(); }
    void SetWhite() { SetAllColor(255, 255, 255); Show(); }
    void SetOff() { Clear(); Show(); }

    // Breathing effect (smooth fade in/out)
    void StartBreathing(uint8_t red, uint8_t green, uint8_t blue, uint32_t period_ms = 2000);
    void StopBreathing();

    // Rainbow effect
    void StartRainbow(uint32_t period_ms = 5000);
    void StopRainbow();

private:
    gpio_num_t pin_;
    uint16_t num_leds_;
    bool initialized_;
    uint8_t* led_data_;  // RGB data buffer (3 bytes per LED)

    // Breathing effect variables
    bool breathing_active_;
    uint8_t breathing_red_, breathing_green_, breathing_blue_;
    uint32_t breathing_period_;
    uint32_t breathing_start_time_;

    // Rainbow effect variables
    bool rainbow_active_;
    uint32_t rainbow_period_;
    uint32_t rainbow_start_time_;

    // Task handle for effects
    TaskHandle_t effect_task_;

    // Private methods
    void UpdateBreathing();
    void UpdateRainbow();
    static void EffectTask(void* param);

    // Helper functions
    uint32_t ColorToUint32(uint8_t red, uint8_t green, uint8_t blue);
    void Uint32ToColor(uint32_t color, uint8_t& red, uint8_t& green, uint8_t& blue);
};

#endif  // __LED_RGB_STRIP_H__