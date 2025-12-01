/*
    Otto机器人控制器 - MCP协议版本
*/

#include <cJSON.h>
#include <esp_log.h>

#include <cstring>

#include "application.h"
#include "board.h"
#include "config.h"
#include "led/circular_strip.h"
#include "mcp_server.h"
#include "otto_movements.h"
#include "sdkconfig.h"
#include "settings.h"

#define TAG "OttoController"

class OttoController {
private:
    Otto otto_;
    TaskHandle_t action_task_handle_ = nullptr;
    QueueHandle_t action_queue_;
    bool has_hands_ = false;
    bool is_action_in_progress_ = false;

    struct OttoActionParams {
        int action_type;
        int steps;
        int speed;
        int direction;
        int amount;
    };

    enum ActionType {
        ACTION_WALK = 1,
        ACTION_TURN = 2,
        ACTION_STOP = 99
    };

    static void ActionTask(void* arg) {
        OttoController* controller = static_cast<OttoController*>(arg);
        OttoActionParams params;
        
        ESP_LOGI(TAG, "ActionTask started - waiting for stabilization");
        vTaskDelay(pdMS_TO_TICKS(500));  // Give system more time to stabilize
        
        ESP_LOGI(TAG, "Attempting to attach motors...");
        controller->otto_.AttachMotors();
        ESP_LOGI(TAG, "Motors attachment completed");
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Additional stabilization after motor attachment

        while (true) {
            if (xQueueReceive(controller->action_queue_, &params, pdMS_TO_TICKS(1000)) == pdTRUE) {
                ESP_LOGI(TAG, "Thực hiện hành động: %d", params.action_type);
                controller->is_action_in_progress_ = true;
                
                vTaskDelay(pdMS_TO_TICKS(10));  // Yield before action

                switch (params.action_type) {
                    case ACTION_WALK:
                        controller->otto_.Walk(params.steps, params.speed, params.direction,
                                               params.amount);
                        vTaskDelay(pdMS_TO_TICKS(5));
                        break;
                    case ACTION_TURN:
                        controller->otto_.Turn(params.steps, params.speed, params.direction,
                                               params.amount);
                        vTaskDelay(pdMS_TO_TICKS(5));
                        break;
                    case ACTION_STOP:
                        controller->otto_.Home(true);
                        break;
                }
                if (params.action_type != ACTION_STOP) {
                    controller->otto_.Home(true);
                }
                controller->is_action_in_progress_ = false;
                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }
    }

    void StartActionTaskIfNeeded() {
        if (action_task_handle_ == nullptr) {
            ESP_LOGI(TAG, "Creating ActionTask...");
            BaseType_t result = xTaskCreate(ActionTask, "otto_action", 1024 * 4, this, configMAX_PRIORITIES - 2,
                        &action_task_handle_);
            if (result != pdPASS) {
                ESP_LOGE(TAG, "Failed to create ActionTask");
            } else {
                ESP_LOGI(TAG, "ActionTask created successfully");
            }
        }
    }

    void QueueAction(int action_type, int steps, int speed, int direction, int amount) {
        if (action_queue_ == nullptr) {
            ESP_LOGE(TAG, "Action queue not initialized");
            return;
        }

        ESP_LOGI(TAG, "Điều khiển hành động: type=%d, steps=%d, speed=%d, direction=%d, amount=%d", action_type, steps,
                 speed, direction, amount);

        OttoActionParams params = {action_type, steps, speed, direction, amount};
        BaseType_t result = xQueueSend(action_queue_, &params, pdMS_TO_TICKS(100));
        if (result != pdTRUE) {
            ESP_LOGW(TAG, "Failed to queue action, queue might be full");
            return;
        }
        
        StartActionTaskIfNeeded();
    }

public:
    OttoController() {
        ESP_LOGI(TAG, "Initializing OttoController...");
        
        // Initialize Otto without attaching motors
        otto_.Init();
        ESP_LOGI(TAG, "Otto initialized");

        action_queue_ = xQueueCreate(10, sizeof(OttoActionParams));
        if (action_queue_ == nullptr) {
            ESP_LOGE(TAG, "Failed to create action queue");
            return;
        }
        ESP_LOGI(TAG, "Action queue created");

        RegisterMcpTools();
        ESP_LOGI(TAG, "MCP tools registered - OttoController ready");
    }

    void RegisterMcpTools() {
        auto& mcp_server = McpServer::GetInstance();

        ESP_LOGI(TAG, "Bắt đầu đăng ký công cụ MCP...");

        // 基础移动动作
        mcp_server.AddTool("self.otto.walk_forward",
                           "Đi bộ. steps: Số bước đi (1-100); speed: Tốc độ động cơ (10-100%); "
                           "direction: Hướng đi (-1=lùi, 1=tiến)",
                           PropertyList({Property("steps", kPropertyTypeInteger, 3, 1, 100),
                                         Property("speed", kPropertyTypeInteger, 80, 10, 100),
                                         Property("direction", kPropertyTypeInteger, 1, -1, 1)}),
                           [this](const PropertyList& properties) -> ReturnValue {
                               int steps = properties["steps"].value<int>();
                               int speed = properties["speed"].value<int>();
                               int direction = properties["direction"].value<int>();
                               QueueAction(ACTION_WALK, steps, 1000, direction, speed);
                               return true;
                           });

        mcp_server.AddTool("self.otto.turn",
                           "Quay người. steps: Số bước quay (1-100); speed: Tốc độ động cơ (10-100%); "
                           "direction: Hướng quay (1=quay trái, -1=quay phải)",
                           PropertyList({Property("steps", kPropertyTypeInteger, 3, 1, 100),
                                         Property("speed", kPropertyTypeInteger, 80, 10, 100),
                                         Property("direction", kPropertyTypeInteger, 1, -1, 1)}),
                           [this](const PropertyList& properties) -> ReturnValue {
                               int steps = properties["steps"].value<int>();
                               int speed = properties["speed"].value<int>();
                               int direction = properties["direction"].value<int>();
                               QueueAction(ACTION_TURN, steps, 1000, direction, speed);
                               return true;
                           });

        // 系统工具
        mcp_server.AddTool("self.otto.stop", "Dừng ngay lập tức", PropertyList(),
                           [this](const PropertyList& properties) -> ReturnValue {
                               QueueAction(ACTION_STOP, 1, 1000, 1, 0);

                               QueueAction(ACTION_STOP, 1, 1000, 1, 0);
                               return true;
                           });

        mcp_server.AddTool("self.otto.get_status", "Lấy trạng thái robot, trả về moving hoặc idle",
                           PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
                               return is_action_in_progress_ ? "moving" : "idle";
                           });

        mcp_server.AddTool("self.battery.get_level", "Lấy mức pin và trạng thái sạc của robot", PropertyList(),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               int level = 0;
                               bool charging = false;
                               bool discharging = false;
                               board.GetBatteryLevel(level, charging, discharging);

                               std::string status =
                                   "{\"level\":" + std::to_string(level) +
                                   ",\"charging\":" + (charging ? "true" : "false") + "}";
                               return status;
                           });

        // LED RGB Control Tools
        mcp_server.AddTool("self.led.set_color", 
                           "Đặt màu LED RGB. r: Đỏ (0-255); g: Xanh lá (0-255); b: Xanh dương (0-255)",
                           PropertyList({
                               Property("r", kPropertyTypeInteger, 255, 0, 255),
                               Property("g", kPropertyTypeInteger, 255, 0, 255), 
                               Property("b", kPropertyTypeInteger, 255, 0, 255)
                           }),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   int r = properties["r"].value<int>();
                                   int g = properties["g"].value<int>();
                                   int b = properties["b"].value<int>();
                                   StripColor color = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                                   strip->SetAllColor(color);
                                   return true;
                               }
                               return false;
                           });

        mcp_server.AddTool("self.led.turn_off", "Tắt tất cả LED RGB", PropertyList(),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   StripColor off = {0, 0, 0};
                                   strip->SetAllColor(off);
                                   return true;
                               }
                               return false;
                           });

        mcp_server.AddTool("self.led.breathing_effect", 
                           "Hiệu ứng thở LED RGB. r: Đỏ (0-255); g: Xanh lá (0-255); b: Xanh dương (0-255); speed_ms: Tốc độ (50-1000ms)",
                           PropertyList({
                               Property("r", kPropertyTypeInteger, 255, 0, 255),
                               Property("g", kPropertyTypeInteger, 100, 0, 255),
                               Property("b", kPropertyTypeInteger, 100, 0, 255),
                               Property("speed_ms", kPropertyTypeInteger, 100, 50, 1000)
                           }),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   int r = properties["r"].value<int>();
                                   int g = properties["g"].value<int>();
                                   int b = properties["b"].value<int>();
                                   int speed_ms = properties["speed_ms"].value<int>();
                                   StripColor low = {10, 10, 10};
                                   StripColor high = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                                   strip->Breathe(low, high, speed_ms);
                                   return true;
                               }
                               return false;
                           });

        mcp_server.AddTool("self.led.blink_effect", 
                           "Hiệu ứng nhấp nháy LED RGB. r: Đỏ (0-255); g: Xanh lá (0-255); b: Xanh dương (0-255); interval_ms: Khoảng cách (100-2000ms)",
                           PropertyList({
                               Property("r", kPropertyTypeInteger, 255, 0, 255),
                               Property("g", kPropertyTypeInteger, 100, 0, 255),
                               Property("b", kPropertyTypeInteger, 100, 0, 255),
                               Property("interval_ms", kPropertyTypeInteger, 500, 100, 2000)
                           }),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   int r = properties["r"].value<int>();
                                   int g = properties["g"].value<int>();
                                   int b = properties["b"].value<int>();
                                   int interval_ms = properties["interval_ms"].value<int>();
                                   StripColor color = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                                   strip->Blink(color, interval_ms);
                                   return true;
                               }
                               return false;
                           });

        mcp_server.AddTool("self.led.scroll_effect", 
                           "Hiệu ứng chạy LED (đuổi nhau). r: Đỏ (0-255); g: Xanh lá (0-255); b: Xanh dương (0-255); length: Độ dài (1-8); speed_ms: Tốc độ (50-500ms)",
                           PropertyList({
                               Property("r", kPropertyTypeInteger, 255, 0, 255),
                               Property("g", kPropertyTypeInteger, 100, 0, 255),
                               Property("b", kPropertyTypeInteger, 100, 0, 255),
                               Property("length", kPropertyTypeInteger, 3, 1, 8),
                               Property("speed_ms", kPropertyTypeInteger, 200, 50, 500)
                           }),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   int r = properties["r"].value<int>();
                                   int g = properties["g"].value<int>();
                                   int b = properties["b"].value<int>();
                                   int length = properties["length"].value<int>();
                                   int speed_ms = properties["speed_ms"].value<int>();
                                   StripColor low = {5, 5, 5};
                                   StripColor high = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                                   strip->Scroll(low, high, length, speed_ms);
                                   return true;
                               }
                               return false;
                           });

        mcp_server.AddTool("self.led.set_brightness", 
                           "Điều chỉnh độ sáng LED RGB. default_brightness: Độ sáng bình thường (1-255); low_brightness: Độ sáng thấp (1-32)",
                           PropertyList({
                               Property("default_brightness", kPropertyTypeInteger, 64, 1, 255),
                               Property("low_brightness", kPropertyTypeInteger, 8, 1, 32)
                           }),
                           [](const PropertyList& properties) -> ReturnValue {
                               auto& board = Board::GetInstance();
                               Led* led = board.GetLed();
                               CircularStrip* strip = dynamic_cast<CircularStrip*>(led);
                               if (strip) {
                                   int default_brightness = properties["default_brightness"].value<int>();
                                   int low_brightness = properties["low_brightness"].value<int>();
                                   strip->SetBrightness(static_cast<uint8_t>(default_brightness), 
                                                      static_cast<uint8_t>(low_brightness));
                                   return true;
                               }
                               return false;
                           });

        ESP_LOGI(TAG, "Hoàn thành đăng ký công cụ MCP");
    }

    ~OttoController() {
        if (action_task_handle_ != nullptr) {
            vTaskDelete(action_task_handle_);
            action_task_handle_ = nullptr;
        }
        vQueueDelete(action_queue_);
    }
};

static OttoController* g_otto_controller = nullptr;

void InitializeOttoController() {
    if (g_otto_controller == nullptr) {
        g_otto_controller = new OttoController();
        ESP_LOGI(TAG, "Khởi tạo controller Otto và đăng ký công cụ MCP");
    }
}
