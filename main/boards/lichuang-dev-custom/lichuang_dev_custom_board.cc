#include "wifi_board.h"
#include "codecs/box_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "esp32_camera.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <wifi_station.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>
#include <mqtt_client.h>
#include <freertos/task.h>
#include <esp_event.h>

#define TAG "LichuangDevCustomBoard"

// PCA9557 I/O Expander class - giữ nguyên từ lichuang-dev
class Pca9557 : public I2cDevice {
public:
    Pca9557(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
        WriteReg(0x01, 0x03);
        WriteReg(0x03, 0xf8);
    }

    void SetOutputState(uint8_t bit, uint8_t level) {
        uint8_t data = ReadReg(0x01);
        data = (data & ~(1 << bit)) | (level << bit);
        WriteReg(0x01, data);
    }

    uint8_t GetInputState() {
        return ReadReg(0x00);
    }

    uint8_t GetOutputState() {
        return ReadReg(0x01);
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

// Class board tùy chỉnh
class LichuangDevCustomBoard : public WifiBoard {
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
        buscfg.mosi_io_num = GPIO_NUM_40;  // SPI MOSI cho màn hình
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = GPIO_NUM_41;  // SPI SCK cho màn hình
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
                ESP_LOGI(TAG, "AEC mode toggled");
            }
        });
#endif

        // Thêm tính năng tùy chỉnh: Long press để reset
        boot_button_.OnLongPress([this]() {
            ESP_LOGI(TAG, "Long press detected - custom feature");
            // Thêm logic tùy chỉnh ở đây
        });
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
        ESP_LOGI(TAG, "Camera initialized successfully");
    }

    // Tính năng tùy chỉnh cho board mới
    void InitializeCustomFeatures() {
        ESP_LOGI(TAG, "Initializing custom features for LichuangDevCustomBoard v%s", BOARD_VERSION);

        // Thêm logic tùy chỉnh ở đây
        // Ví dụ: Khởi tạo sensor bổ sung, LED patterns, etc.
    }

    // MCP Tools initialization
    void InitializeTools() {
        auto& mcp_server = McpServer::GetInstance();
        // Thêm tool để điều khiển LED qua PCA9557
        mcp_server.AddTool("self.sum.two_numbers", 
            "Tính tổng 2 số và trả về kết quả",
            PropertyList({
            Property("a", kPropertyTypeInteger),
            Property("b", kPropertyTypeInteger)
            }), 
            [this](const PropertyList& properties) -> ReturnValue {
            int a = properties["a"].value<int>();
            int b = properties["b"].value<int>();
            ESP_LOGI(TAG, "Test MCP server: %d\n", a + b);
            return a + b;
            });
        // Thêm tool để điều khiển LED qua PCA9557
        mcp_server.AddTool("self.led.set_state", 
            "Điều khiển trạng thái LED trên board. Sử dụng bit 0-7 để chọn LED, level 0=tắt, 1=bật",
            PropertyList({
                Property("bit", kPropertyTypeInteger, 0, 7),
                Property("level", kPropertyTypeInteger, 0, 1)
            }), 
            [this](const PropertyList& properties) -> ReturnValue {
                int bit = properties["bit"].value<int>();
                int level = properties["level"].value<int>();
                pca9557_->SetOutputState(bit, level);
                ESP_LOGI(TAG, "LED bit %d set to %d", bit, level);
                return true;
            });
        
        // Thêm tool để đọc trạng thái các GPIO từ PCA9557
        mcp_server.AddTool("self.gpio.read_state",
            "Đọc trạng thái các GPIO từ PCA9557 I/O expander",
            PropertyList(),
            [this](const PropertyList& properties) -> ReturnValue {
                uint8_t input_state = pca9557_->GetInputState();
                uint8_t output_state = pca9557_->GetOutputState();
                return std::string("{\"input_state\": ") + std::to_string(input_state) + 
                       ", \"output_state\": " + std::to_string(output_state) + "}";
            });
        
        // Thêm tool để publish MQTT message
        mcp_server.AddTool("self.mqtt.publish",
            "Gửi tin nhắn đi cho tôi",
            PropertyList({
                Property("message", kPropertyTypeString),
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                std::string message = properties["message"].value<std::string>();
                ESP_LOGI(TAG, "Publishing MQTT message: %s", message.c_str());
                return PublishMqttMessage("Mqtt.mysignage.vn", 1883, "mcp_server", message);
            });
    }

    ReturnValue PublishMqttMessage(const std::string& server, int port, const std::string& topic, const std::string& message) {
        ESP_LOGI(TAG, "Publishing MQTT message to %s:%d topic: %s", server.c_str(), port, topic.c_str());
        
        std::string uri;
        if (server.find("://") != std::string::npos) {
            uri = server;
        } else {
            uri = "mqtt://" + server + ":" + std::to_string(port);
        }
        ESP_LOGI(TAG, "MQTT URI: %s", uri.c_str());
        
        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.uri = uri.c_str();
        mqtt_cfg.session.keepalive = 60;
        mqtt_cfg.network.timeout_ms = 5000;
        
        bool connected = false;
        bool published = false;
        
        auto event_handler = [](void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
            auto args = static_cast<std::pair<bool*, bool*>*>(handler_args);
            bool* connected_ptr = args->first;
            bool* published_ptr = args->second;
            
            switch (event_id) {
                case MQTT_EVENT_CONNECTED:
                    *connected_ptr = true;
                    break;
                case MQTT_EVENT_DISCONNECTED:
                    *connected_ptr = false;
                    break;
                case MQTT_EVENT_PUBLISHED:
                    *published_ptr = true;
                    break;
                case MQTT_EVENT_ERROR:
                    ESP_LOGE("MQTT", "MQTT event error");
                    break;
            }
        };
        
        std::pair<bool*, bool*> handler_args(&connected, &published);
        
        esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
        if (client == nullptr) {
            ESP_LOGE(TAG, "Failed to initialize MQTT client");
            return false;
        }
        
        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, event_handler, &handler_args);
        
        esp_err_t err = esp_mqtt_client_start(client);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
            esp_mqtt_client_destroy(client);
            return false;
        }
        
        auto start_time = xTaskGetTickCount();
        while (!connected && (xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(3000)) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        if (!connected) {
            ESP_LOGE(TAG, "MQTT connection timeout");
            esp_mqtt_client_stop(client);
            esp_mqtt_client_destroy(client);
            return false;
        }
        
        int msg_id = esp_mqtt_client_publish(client, topic.c_str(), message.c_str(), message.length(), 0, 0);
        if (msg_id < 0) {
            ESP_LOGE(TAG, "Failed to publish MQTT message");
            esp_mqtt_client_stop(client);
            esp_mqtt_client_destroy(client);
            return false;
        }

        start_time = xTaskGetTickCount();
        while (!published && (xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(2000)) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        ESP_LOGI(TAG, "MQTT message published successfully, msg_id: %d", msg_id);
        
    
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        
        return true;
    }
public:
    LichuangDevCustomBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        ESP_LOGI(TAG, "Initializing LichuangDevCustomBoard");

        InitializeI2c();
        InitializeSpi();
        InitializeSt7789Display();
        InitializeTouch();
        InitializeButtons();
        InitializeCamera();
        InitializeCustomFeatures();
        InitializeTools();

        GetBacklight()->RestoreBrightness();

        ESP_LOGI(TAG, "LichuangDevCustomBoard initialized successfully");
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

    // Phương thức tùy chỉnh để truy cập PCA9557
    Pca9557* GetPca9557() {
        return pca9557_;
    }
};

DECLARE_BOARD(LichuangDevCustomBoard);