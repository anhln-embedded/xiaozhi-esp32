#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define POWER_CHARGE_DETECT_PIN GPIO_NUM_14
#define POWER_ADC_UNIT ADC_UNIT_2
#define POWER_ADC_CHANNEL ADC_CHANNEL_3

#define AIN1 GPIO_NUM_13
#define AIN2 GPIO_NUM_17
// #define BIN1 GPIO_NUM_38
// #define BIN2 GPIO_NUM_37
#define BIN1 GPIO_NUM_1
#define BIN2 GPIO_NUM_45
#define PWMA GPIO_NUM_39
#define PWMB GPIO_NUM_18
#define STBY GPIO_NUM_42

// PWM Configuration - Using timers 2,3 and channels 2,3 to avoid conflicts with backlight
#define LEDC_TIMER_A LEDC_TIMER_2
#define LEDC_TIMER_B LEDC_TIMER_3
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY (5000) // 5kHz

// PWM Channels - Using channels 2,3 to avoid conflicts with backlight
#define LEDC_CHANNEL_A LEDC_CHANNEL_2
#define LEDC_CHANNEL_B LEDC_CHANNEL_3

#define AUDIO_INPUT_SAMPLE_RATE 16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000
#define AUDIO_I2S_METHOD_SIMPLEX

#define AUDIO_I2S_MIC_GPIO_WS GPIO_NUM_4
#define AUDIO_I2S_MIC_GPIO_SCK GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_DIN GPIO_NUM_6
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_3
#define DISPLAY_MOSI_PIN GPIO_NUM_10
#define DISPLAY_CLK_PIN GPIO_NUM_9
#define DISPLAY_DC_PIN GPIO_NUM_46
#define DISPLAY_RST_PIN GPIO_NUM_11
#define DISPLAY_CS_PIN GPIO_NUM_12

#define LCD_TYPE_ST7789_SERIAL
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true
#define DISPLAY_INVERT_COLOR true
#define DISPLAY_RGB_ORDER LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 3

#define BOOT_BUTTON_GPIO GPIO_NUM_0

// RGB LED Strip Configuration - Circular 16-bit LEDs  
#define RGB_LED_DO_GPIO GPIO_NUM_2   // Data Output (không dùng)
#define RGB_LED_DI_GPIO GPIO_NUM_21  // Data Input - chân điều khiển LED
#define RGB_LED_COUNT 16  // Number of LEDs in each circular strip

// Custom MAC address for the device (format: 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX)
// Uncomment and modify the line below to set a custom MAC address
#define CUSTOM_MAC_ADDRESS {0x02, 0x1A, 0x3B, 0x4C, 0x5D, 0x6E}

#define OTTO_ROBOT_VERSION "1.4.5"

#endif  // _BOARD_CONFIG_H_
