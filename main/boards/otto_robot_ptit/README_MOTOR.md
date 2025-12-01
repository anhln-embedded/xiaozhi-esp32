# TB6612FNG Motor Library for ESP32-S3

Thư viện điều khiển động cơ TB6612FNG cho board Otto Robot Ptit.

## Tính năng

- Điều khiển 2 động cơ DC với PWM speed control
- Hỗ trợ hướng: Forward, Backward, Stop
- PWM 13-bit resolution (8192 levels) tại 5kHz
- API đơn giản và dễ sử dụng
- Tích hợp với ESP-IDF LEDC driver

## Pin Mapping

| Chân TB6612FNG | ESP32-S3 GPIO | Mô tả |
|---------------|---------------|--------|
| AIN1 | GPIO12 | Motor A direction control |
| AIN2 | GPIO17 | Motor A direction control |
| PWMA | GPIO39 | Motor A PWM speed control |
| BIN1 | GPIO38 | Motor B direction control |
| BIN2 | GPIO40 | Motor B direction control (thay GPIO37) |
| PWMB | GPIO18 | Motor B PWM speed control |
| STBY | GPIO42 | Standby (enable/disable driver) |

## Cách sử dụng

### 1. Khởi tạo driver

```cpp
#include "motor.h"

// Khởi tạo motor driver (chỉ gọi 1 lần)
motor_init();
```

### 2. Điều khiển trực tiếp

```cpp
// Motor A forward với tốc độ 50%
motor_a_control(MOTOR_FORWARD, 50);

// Motor B backward với tốc độ 75%
motor_b_control(MOTOR_BACKWARD, 75);

// Dừng motor A
motor_a_control(MOTOR_STOP, 0);
```

### 3. Sử dụng Motor class (khuyên dùng)

```cpp
#include "motor.h"

Motor motorA, motorB;

// Khởi tạo motors
motorA.Attach(0);  // Motor A
motorB.Attach(1);  // Motor B

// Điều khiển tốc độ (-100 đến 100)
motorA.SetSpeed(50);   // Forward 50%
motorB.SetSpeed(-75);  // Backward 75%
motorA.Stop();         // Dừng motor A

// Kiểm tra trạng thái
if (motorA.IsAttached()) {
    int speed = motorA.GetSpeed();
}

// Hủy motors
motorA.Detach();
motorB.Detach();
```

## API Reference

### Functions

- `void motor_init()`: Khởi tạo motor driver
- `void motor_a_control(motor_direction_t direction, uint32_t speed)`: Điều khiển motor A
- `void motor_b_control(motor_direction_t direction, uint32_t speed)`: Điều khiển motor B

### Motor Class

- `Motor()`: Constructor
- `void Attach(int motor_id)`: Gán motor (0=Motor A, 1=Motor B)
- `void Detach()`: Hủy gán motor
- `void SetSpeed(int speed)`: Đặt tốc độ (-100 đến 100)
- `void Stop()`: Dừng motor
- `int GetSpeed() const`: Lấy tốc độ hiện tại
- `bool IsAttached() const`: Kiểm tra motor đã được gán chưa
- `void Brake()`: Phanh motor (giống Stop)

### Direction Enum

- `MOTOR_STOP`: Dừng motor
- `MOTOR_FORWARD`: Chạy thuận
- `MOTOR_BACKWARD`: Chạy ngược

## Lưu ý

- Speed range: 0-100 cho direct functions, -100 đến 100 cho Motor class
- PWM frequency: 5kHz với 13-bit resolution
- BIN2 sử dụng GPIO40 thay vì GPIO37 (do conflict với PSRAM)
- Motor driver tự động khởi tạo khi sử dụng Motor class