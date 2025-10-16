# Hướng Dẫn Tạo Board Mới Cho Dự Án Xiaozhi ESP32

## Giới Thiệu

Dự án Xiaozhi AI là một trợ lý giọng nói thông minh chạy trên ESP32. Dự án hỗ trợ hơn 70 loại board phát triển ESP32 khác nhau. Tài liệu này hướng dẫn cách tạo một board tùy chỉnh mới cho dự án.

## Lưu Ý Quan Trọng

> **Cảnh Báo**: Khi tạo board tùy chỉnh, nếu cấu hình chân GPIO khác với board gốc, **không được** ghi đè trực tiếp lên cấu hình của board gốc để compile firmware. Phải tạo loại board mới hoặc sử dụng file config.json với tên build khác và macro sdkconfig để phân biệt. Sử dụng `python scripts/release.py [tên-thư-mục-board]` để compile và đóng gói firmware.
>
> Nếu ghi đè trực tiếp lên cấu hình gốc, firmware OTA sau này có thể bị ghi đè bởi firmware chuẩn của board gốc, khiến thiết bị của bạn không hoạt động. Mỗi board có ID duy nhất và kênh cập nhật firmware riêng, việc duy trì tính duy nhất của ID board rất quan trọng.

## Ví Dụ Cụ Thể: Board Lichuang-Dev

Board Lichuang-Dev là một board ESP32-S3 với các tính năng sau:
- **Chip**: ESP32-S3
- **Màn hình**: ST7789 320x240 SPI với touch FT5x06
- **Âm thanh**: ES8311 + ES7210 với PCA9557 I/O expander
- **Camera**: ESP32 Camera
- **Tính năng đặc biệt**: AEC (Acoustic Echo Cancellation)

Chúng ta sẽ sử dụng board này làm ví dụ để hướng dẫn tạo board tùy chỉnh.

## Lưu Ý Quan Trọng

> **Cảnh Báo**: Khi tạo board tùy chỉnh, nếu cấu hình chân GPIO khác với board gốc, **không được** ghi đè trực tiếp lên cấu hình của board gốc để compile firmware. Phải tạo loại board mới hoặc sử dụng file config.json với tên build khác và macro sdkconfig để phân biệt. Sử dụng `python scripts/release.py [tên-thư-mục-board]` để compile và đóng gói firmware.
>
> Nếu ghi đè trực tiếp lên cấu hình gốc, firmware OTA sau này có thể bị ghi đè bởi firmware chuẩn của board gốc, khiến thiết bị của bạn không hoạt động. Mỗi board có ID duy nhất và kênh cập nhật firmware riêng, việc duy trì tính duy nhất của ID board rất quan trọng.

## Cấu Trúc Thư Mục

Mỗi thư mục board thường chứa các file sau:

- `xxx_board.cc` - Mã khởi tạo board chính, triển khai các chức năng liên quan đến board
- `config.h` - File cấu hình board, định nghĩa mapping chân GPIO và các cấu hình khác
- `config.json` - Cấu hình compile, chỉ định chip đích và tùy chọn compile đặc biệt
- `README.md` - Tài liệu hướng dẫn board, mô tả tính năng, yêu cầu phần cứng, và cách compile/flash

## Các Bước Tạo Board Mới

### Bước 1: Tạo Thư Mục Board Mới

Tạo thư mục mới trong `boards/` với định dạng `[tên-hãng]-[tên-board]`, ví dụ `m5stack-tab5`:

```bash
mkdir main/boards/my-custom-board
```

### Bước 2: Tạo File Cấu Hình

#### config.h

Trong `config.h`, định nghĩa tất cả cấu hình phần cứng. Dựa trên board lichuang-dev:

```c
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// Cấu hình âm thanh
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000
#define AUDIO_INPUT_REFERENCE    true  // Sử dụng reference cho AEC

// Chân I2S cho âm thanh
#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_38
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_13
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_12
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_45

// Sử dụng PCA9557 I/O expander cho điều khiển âm thanh
#define AUDIO_CODEC_USE_PCA9557
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_2
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR
#define AUDIO_CODEC_ES7210_ADDR  0x82  // Địa chỉ ES7210

// LED và nút bấm
#define BUILTIN_LED_GPIO        GPIO_NUM_48
#define BOOT_BUTTON_GPIO        GPIO_NUM_0
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_NC

// Cấu hình màn hình
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_42
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

// Cấu hình camera (nếu có)
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 5
#define CAMERA_PIN_SIOD 1
#define CAMERA_PIN_SIOC 2

#define CAMERA_PIN_D7 9
#define CAMERA_PIN_D6 4
#define CAMERA_PIN_D5 6
#define CAMERA_PIN_D4 15
#define CAMERA_PIN_D3 17
#define CAMERA_PIN_D2 8
#define CAMERA_PIN_D1 18
#define CAMERA_PIN_D0 16
#define CAMERA_PIN_VSYNC 3
#define CAMERA_PIN_HREF 46
#define CAMERA_PIN_PCLK 7

#define XCLK_FREQ_HZ 24000000

#endif // _BOARD_CONFIG_H_
```

#### config.json

Trong `config.json`, định nghĩa cấu hình compile cho script `scripts/release.py`. Dựa trên lichuang-dev:

```json
{
    "target": "esp32s3",
    "builds": [
        {
            "name": "lichuang-dev",
            "sdkconfig_append": [
                "CONFIG_USE_DEVICE_AEC=y"
            ]
        }
    ]
}
```

**Giải thích các tùy chọn:**

- `target`: "esp32s3" - Chip đích là ESP32-S3
- `name`: "lichuang-dev" - Tên gói firmware output
- `sdkconfig_append`: 
  - `"CONFIG_USE_DEVICE_AEC=y"`: Bật Acoustic Echo Cancellation (AEC) phía thiết bị

**Các tùy chọn sdkconfig_append phổ biến:**

```json
// Kích thước Flash
"CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y"   // 4MB Flash
"CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y"   // 8MB Flash
"CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y"  // 16MB Flash

// Partition table
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/4m.csv\""  // 4MB partition
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/8m.csv\""  // 8MB partition
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/16m.csv\"" // 16MB partition

// Ngôn ngữ
"CONFIG_LANGUAGE_EN_US=y"  // Tiếng Anh
"CONFIG_LANGUAGE_ZH_CN=y"  // Tiếng Trung giản thể

// Wake word
"CONFIG_USE_DEVICE_AEC=y"          // Bật AEC thiết bị
"CONFIG_WAKE_WORD_DISABLED=y"      // Tắt wake word
```

### Bước 3: Viết Mã Khởi Tạo Board

Tạo file `lichuang_dev_board.cc` triển khai logic khởi tạo board. Board lichuang-dev có các tính năng đặc biệt:

1. **PCA9557 I/O Expander**: Điều khiển các chân output cho âm thanh và camera
2. **Custom Audio Codec**: Kế thừa từ BoxAudioCodec với PCA9557
3. **Touch Screen**: FT5x06 touch controller
4. **Camera**: ESP32 Camera
5. **AEC Support**: Double-click để bật/tắt AEC

```cpp
#include "wifi_board.h"
#include "codecs/box_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "esp32_camera.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <wifi_station.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>

#define TAG "LichuangDevBoard"

// Class PCA9557 I/O Expander
class Pca9557 : public I2cDevice {
public:
    Pca9557(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
        WriteReg(0x01, 0x03);  // Cấu hình chân output
        WriteReg(0x03, 0xf8);  // Cấu hình hướng
    }

    void SetOutputState(uint8_t bit, uint8_t level) {
        uint8_t data = ReadReg(0x01);
        data = (data & ~(1 << bit)) | (level << bit);
        WriteReg(0x01, data);
    }
};

// Custom Audio Codec với PCA9557
class CustomAudioCodec : public BoxAudioCodec {
private:
    Pca9557* pca9557_;

public:
    CustomAudioCodec(i2c_master_bus_handle_t i2c_bus, Pca9557* pca9557) 
        : BoxAudioCodec(i2c_bus, 
                       AUDIO_INPUT_SAMPLE_RATE, 
                       AUDIO_OUTPUT_SAMPLE_RATE,
                       AUDIO_I2S_GPIO_MCLK, 
                       AUDIO_I2S_GPIO_BCLK, 
                       AUDIO_I2S_GPIO_WS, 
                       AUDIO_I2S_GPIO_DOUT, 
                       AUDIO_I2S_GPIO_DIN,
                       GPIO_NUM_NC, 
                       AUDIO_CODEC_ES8311_ADDR, 
                       AUDIO_CODEC_ES7210_ADDR, 
                       AUDIO_INPUT_REFERENCE),
          pca9557_(pca9557) {
    }

    virtual void EnableOutput(bool enable) override {
        BoxAudioCodec::EnableOutput(enable);
        if (enable) {
            pca9557_->SetOutputState(1, 1);  // Bật loa qua PCA9557
        } else {
            pca9557_->SetOutputState(1, 0);  // Tắt loa qua PCA9557
        }
    }
};

class LichuangDevBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Button boot_button_;
    LcdDisplay* display_;
    Pca9557* pca9557_;
    Esp32Camera* camera_;

    void InitializeI2c() {
        // Khởi tạo I2C bus
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,  // Sử dụng I2C port 1
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));

        // Khởi tạo PCA9557
        pca9557_ = new Pca9557(i2c_bus_, 0x19);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = GPIO_NUM_40;  // Chân MOSI cho màn hình
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = GPIO_NUM_41;  // Chân SCK cho màn hình
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });

#if CONFIG_USE_DEVICE_AEC
        // Double-click để bật/tắt AEC
        boot_button_.OnDoubleClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateIdle) {
                app.SetAecMode(app.GetAecMode() == kAecOff ? kAecOnDeviceSide : kAecOff);
            }
        });
#endif
    }

    void InitializeSt7789Display() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        
        // Khởi tạo panel IO cho SPI
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = GPIO_NUM_NC;  // Không dùng CS
        io_config.dc_gpio_num = GPIO_NUM_39;  // Chân DC
        io_config.spi_mode = 2;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        // Khởi tạo driver ST7789
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
        
        esp_lcd_panel_reset(panel);
        pca9557_->SetOutputState(0, 0);  // Bật màn hình qua PCA9557
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        
        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeTouch() {
        esp_lcd_touch_handle_t tp;
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_HEIGHT,
            .y_max = DISPLAY_WIDTH,
            .rst_gpio_num = GPIO_NUM_NC,
            .int_gpio_num = GPIO_NUM_NC, 
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .flags = {
                .swap_xy = 1,
                .mirror_x = 1,
                .mirror_y = 0,
            },
        };
        
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
        tp_io_config.scl_speed_hz = 400000;

        esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle);
        esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp);

        // Thêm touch input vào LVGL
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lv_display_get_default(), 
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
    }

    void InitializeCamera() {
        // Bật nguồn camera qua PCA9557
        pca9557_->SetOutputState(2, 0);

        camera_config_t config = {};
        config.ledc_channel = LEDC_CHANNEL_2;
        config.ledc_timer = LEDC_TIMER_2;
        config.pin_d0 = CAMERA_PIN_D0;
        config.pin_d1 = CAMERA_PIN_D1;
        config.pin_d2 = CAMERA_PIN_D2;
        config.pin_d3 = CAMERA_PIN_D3;
        config.pin_d4 = CAMERA_PIN_D4;
        config.pin_d5 = CAMERA_PIN_D5;
        config.pin_d6 = CAMERA_PIN_D6;
        config.pin_d7 = CAMERA_PIN_D7;
        config.pin_xclk = CAMERA_PIN_XCLK;
        config.pin_pclk = CAMERA_PIN_PCLK;
        config.pin_vsync = CAMERA_PIN_VSYNC;
        config.pin_href = CAMERA_PIN_HREF;
        config.pin_sccb_sda = -1;   // Sử dụng I2C đã khởi tạo
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.sccb_i2c_port = 1;
        config.pin_pwdn = CAMERA_PIN_PWDN;
        config.pin_reset = CAMERA_PIN_RESET;
        config.xclk_freq_hz = XCLK_FREQ_HZ;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

        camera_ = new Esp32Camera(config);
    }

public:
    LichuangDevBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeSpi();
        InitializeSt7789Display();
        InitializeTouch();
        InitializeButtons();
        InitializeCamera();

        GetBacklight()->RestoreBrightness();
    }

    virtual AudioCodec* GetAudioCodec() override {
        static CustomAudioCodec audio_codec(i2c_bus_, pca9557_);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
    
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual Camera* GetCamera() override {
        return camera_;
    }
};

DECLARE_BOARD(LichuangDevBoard);
```

### Bước 3: Viết Mã Khởi Tạo Board

Tạo file `my_custom_board.cc` triển khai logic khởi tạo board.

Một class board cơ bản bao gồm:

1. **Định nghĩa class**: Kế thừa từ `WifiBoard` hoặc `Ml307Board`
2. **Hàm khởi tạo**: Bao gồm khởi tạo I2C, màn hình, nút bấm, IoT, v.v.
3. **Các phương thức ảo**: Như `GetAudioCodec()`, `GetDisplay()`, `GetBacklight()`, v.v.
4. **Đăng ký board**: Sử dụng macro `DECLARE_BOARD`

```cpp
#include "wifi_board.h"
#include "codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>

#define TAG "MyCustomBoard"

class MyCustomBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    LcdDisplay* display_;

    // Khởi tạo I2C
    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    // Khởi tạo SPI (cho màn hình)
    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    // Khởi tạo nút bấm
    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    // Khởi tạo màn hình (ví dụ ST7789)
    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = 2;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

        // Tạo đối tượng màn hình
        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    // Khởi tạo MCP Tools
    void InitializeTools() {
        // Tham khảo tài liệu MCP
    }

public:
    // Constructor
    MyCustomBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        InitializeTools();
        GetBacklight()->SetBrightness(100);
    }

    // Lấy codec âm thanh
    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_,
            I2C_NUM_0,
            AUDIO_INPUT_SAMPLE_RATE,
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK,
            AUDIO_I2S_GPIO_BCLK,
            AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT,
            AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN,
            AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }

    // Lấy màn hình
    virtual Display* GetDisplay() override {
        return display_;
    }

    // Lấy điều khiển đèn nền
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

// Đăng ký board
};

DECLARE_BOARD(MyCustomBoard);
```

## Hướng Dẫn Sử Dụng Board Lichuang-Dev

Board Lichuang-Dev là một board ESP32-S3 hoàn chỉnh với đầy đủ tính năng. Đây là hướng dẫn sử dụng board này:

### 1. Chuẩn Bị Phần Cứng

Board Lichuang-Dev bao gồm:
- ESP32-S3 chip
- Màn hình LCD 320x240 với touch screen
- Codec âm thanh ES8311 + ES7210
- Camera ESP32
- PCA9557 I/O expander
- Nút bấm và LED

### 2. Cấu Hình và Compile

#### Sử dụng script release.py (Khuyến nghị):
```bash
python scripts/release.py lichuang-dev
```

#### Hoặc cấu hình thủ công:
```bash
idf.py set-target esp32s3
idf.py menuconfig  # Chọn "立创·实战派 ESP32-S3"
idf.py build
idf.py flash monitor
```

### 3. Tính Năng Đặc Biệt

#### Acoustic Echo Cancellation (AEC)
- Board hỗ trợ AEC phía thiết bị để giảm tiếng vang
- Double-click nút boot để bật/tắt AEC khi thiết bị ở trạng thái idle

#### Touch Screen
- Màn hình cảm ứng FT5x06 được tích hợp với LVGL
- Hỗ trợ các thao tác chạm để điều khiển giao diện

#### Camera
- Camera ESP32 với độ phân giải VGA
- Frame buffer được lưu trong PSRAM
- Có thể sử dụng cho các tính năng thị giác máy tính

#### PCA9557 I/O Expander
- Mở rộng I/O thông qua I2C
- Điều khiển bật/tắt màn hình, loa, và camera

### 4. Cấu Hình Chân GPIO

| Chức năng | GPIO | Mô tả |
|-----------|------|-------|
| I2S MCLK  | 38   | Master clock cho âm thanh |
| I2S WS    | 13   | Word select cho âm thanh |
| I2S BCLK  | 14   | Bit clock cho âm thanh |
| I2S DIN   | 12   | Data input cho âm thanh |
| I2S DOUT  | 45   | Data output cho âm thanh |
| I2C SDA   | 1    | I2C data cho codec và PCA9557 |
| I2C SCL   | 2    | I2C clock cho codec và PCA9557 |
| SPI MOSI  | 40   | SPI data cho màn hình |
| SPI SCK   | 41   | SPI clock cho màn hình |
| SPI DC    | 39   | SPI data/command cho màn hình |
| Backlight | 42   | Điều khiển độ sáng màn hình |
| Boot Button| 0   | Nút khởi động |
| LED       | 48   | Đèn LED |

### 5. Khắc Phục Sự Cố

#### Màn hình không hiển thị
- Kiểm tra kết nối SPI (MOSI, SCK, DC)
- Kiểm tra PCA9557 có bật chân 0 không

#### Không có âm thanh
- Kiểm tra kết nối I2S (MCLK, WS, BCLK, DIN, DOUT)
- Kiểm tra PCA9557 có bật chân 1 không
- Kiểm tra địa chỉ I2C của ES8311 (0x18) và ES7210 (0x82)

#### Camera không hoạt động
- Kiểm tra PCA9557 có bật chân 2 không
- Kiểm tra kết nối camera pins
- Kiểm tra PSRAM có được cấu hình đúng không

#### Touch screen không phản hồi
- Kiểm tra kết nối I2C cho FT5x06
- Kiểm tra cấu hình touch trong LVGL

### 6. Phát Triển Thêm

Board Lichuang-Dev là một nền tảng tốt để phát triển các tính năng mới:
- Thêm MCP tools để điều khiển thiết bị qua AI
- Tích hợp thêm sensor I2C
- Phát triển ứng dụng camera với thị giác máy tính
- Mở rộng giao diện người dùng với touch screen

Để phát triển board tùy chỉnh mới, bạn có thể tham khảo cấu trúc của board lichuang-dev làm mẫu.
```

### Bước 4: Cập Nhật Hệ Thống Build

#### Thêm vào Kconfig.projbuild

Mở file `main/Kconfig.projbuild`, thêm tùy chọn board mới vào phần `choice BOARD_TYPE`:

```kconfig
choice BOARD_TYPE
    prompt "Board Type"
    default BOARD_TYPE_BREAD_COMPACT_WIFI
    help
        Board type. Loại board

    # ... các tùy chọn board khác ...

    config BOARD_TYPE_MY_CUSTOM_BOARD
        bool "My Custom Board (Board tùy chỉnh của tôi)"
        depends on IDF_TARGET_ESP32S3  # Thay đổi theo chip đích
endchoice
```

**Lưu ý:**
- `BOARD_TYPE_MY_CUSTOM_BOARD` là tên cấu hình, phải viết hoa và dùng gạch dưới
- `depends on` chỉ định chip đích (ví dụ: `IDF_TARGET_ESP32S3`, `IDF_TARGET_ESP32C3`, v.v.)
- Mô tả có thể dùng tiếng Anh và tiếng Việt

#### Thêm vào CMakeLists.txt

Mở file `main/CMakeLists.txt`, thêm cấu hình board vào chuỗi elseif:

```cmake
# Thêm cấu hình board của bạn vào chuỗi elseif
elseif(CONFIG_BOARD_TYPE_MY_CUSTOM_BOARD)
    set(BOARD_TYPE "my-custom-board")  # Phải khớp với tên thư mục
    set(BUILTIN_TEXT_FONT font_puhui_basic_20_4)  # Chọn font phù hợp với kích thước màn hình
    set(BUILTIN_ICON_FONT font_awesome_20_4)
    set(DEFAULT_EMOJI_COLLECTION twemoji_64)  # Tùy chọn, nếu cần hiển thị emoji
endif()
```

**Giải thích cấu hình font và emoji:**

Chọn font phù hợp theo kích thước màn hình:
- Màn hình nhỏ (128x64 OLED): `font_puhui_basic_14_1` / `font_awesome_14_1`
- Màn hình nhỏ vừa (240x240): `font_puhui_basic_16_4` / `font_awesome_16_4`
- Màn hình vừa (240x320): `font_puhui_basic_20_4` / `font_awesome_20_4`
- Màn hình lớn (480x320+): `font_puhui_basic_30_4` / `font_awesome_30_4`

Tùy chọn emoji:
- `twemoji_32` - Emoji 32x32 pixel (màn hình nhỏ)
- `twemoji_64` - Emoji 64x64 pixel (màn hình lớn)

### Bước 5: Cấu Hình và Compile

#### Phương Pháp 1: Cấu hình thủ công với idf.py

1. **Thiết lập chip đích** (lần đầu hoặc khi đổi chip):
   ```bash
   # Cho ESP32-S3
   idf.py set-target esp32s3

   # Cho ESP32-C3
   idf.py set-target esp32c3

   # Cho ESP32
   idf.py set-target esp32
   ```

2. **Xóa cấu hình cũ**:
   ```bash
   idf.py fullclean
   ```

3. **Vào menu cấu hình**:
   ```bash
   idf.py menuconfig
   ```

   Điều hướng đến: `Xiaozhi Assistant` -> `Board Type`, chọn board tùy chỉnh của bạn.

4. **Compile và flash**:
   ```bash
   idf.py build
   idf.py flash monitor
   ```

#### Phương Pháp 2: Sử dụng script release.py (Khuyến nghị)

Nếu thư mục board có file `config.json`, có thể dùng script này để tự động cấu hình và compile:

```bash
python scripts/release.py my-custom-board
```

Script này sẽ tự động:
- Đọc `target` từ `config.json` và thiết lập chip đích
- Áp dụng các tùy chọn `sdkconfig_append`
- Hoàn thành compile và đóng gói firmware

### Bước 6: Tạo Tài Liệu README.md

Trong README.md, mô tả tính năng board, yêu cầu phần cứng, và cách compile/flash:

## Tính Năng
- **Chip**: ESP32-S3
- **Màn hình**: LCD SPI 320x240
- **Âm thanh**: Codec ES8311
- **Nút bấm**: 1 nút khởi động

## Yêu Cầu Phần Cứng
- ESP32-S3 development board
- Màn hình LCD SPI (ST7789)
- Codec âm thanh ES8311
- Micro và loa

## Compile và Flash

### Sử dụng script release:
```bash
python scripts/release.py my-custom-board
```

### Hoặc cấu hình thủ công:
```bash
idf.py set-target esp32s3
idf.py menuconfig  # Chọn Board Type -> My Custom Board
idf.py build
idf.py flash monitor
```

## Các Thành Phần Board Phổ Biến

### 1. Màn Hình

Dự án hỗ trợ nhiều driver màn hình:
- ST7789 (SPI)
- ILI9341 (SPI)
- SH8601 (QSPI)
- v.v.

### 2. Codec Âm Thanh

Các codec được hỗ trợ:
- ES8311 (phổ biến)
- ES7210 (mảng mic)
- AW88298 (amp)
- v.v.

### 3. Quản Lý Nguồn

Một số board sử dụng chip quản lý nguồn:
- AXP2101
- Các PMIC khác

### 4. MCP Device Control

Có thể thêm các MCP tools để AI điều khiển:
- Speaker (điều khiển loa)
- Screen (điều chỉnh độ sáng màn hình)
- Battery (đọc pin)
- Light (điều khiển đèn)
- v.v.

## Cấu Trúc Kế Thừa Class Board

- `Board` - Class board cơ sở
  - `WifiBoard` - Board kết nối Wi-Fi
  - `Ml307Board` - Board sử dụng module 4G
  - `DualNetworkBoard` - Board hỗ trợ Wi-Fi và 4G

## Mẹo Phát Triển

1. **Tham khảo board tương tự**: Nếu board mới của bạn tương tự board hiện có, có thể tham khảo implementation có sẵn
2. **Debug từng bước**: Khởi tạo các tính năng cơ bản (như hiển thị) trước, sau đó thêm tính năng phức tạp (như âm thanh)
3. **Mapping chân**: Đảm bảo cấu hình chân GPIO chính xác trong config.h
4. **Kiểm tra tương thích phần cứng**: Xác nhận tất cả chip và driver tương thích

## Các Vấn Đề Thường Gặp

1. **Màn hình không hiển thị**: Kiểm tra cấu hình SPI, cài đặt mirror và invert màu
2. **Không có âm thanh output**: Kiểm tra cấu hình I2S, chân PA enable và địa chỉ codec
3. **Không kết nối mạng**: Kiểm tra thông tin Wi-Fi và cấu hình mạng
4. **Không giao tiếp với server**: Kiểm tra cấu hình MQTT hoặc WebSocket

## Tài Liệu Tham Khảo

- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/
- LVGL Documentation: https://docs.lvgl.io/
- ESP-SR Documentation: https://github.com/espressif/esp-sr