# Lichuang Dev Custom Board

Đây là board tùy chỉnh dựa trên Lichuang Dev ESP32-S3 với một số cải tiến và tính năng bổ sung.

## Tính Năng

- **Chip**: ESP32-S3
- **Màn hình**: ST7789 320x240 SPI với touch FT5x06
- **Âm thanh**: ES8311 + ES7210 với PCA9557 I/O expander, hỗ trợ AEC
- **Camera**: ESP32 Camera với độ phân giải VGA
- **Nút bấm**: Boot button với long press support
- **LED**: GPIO 47 (thay đổi từ GPIO 48)
- **Tính năng tùy chỉnh**: Custom features và logging nâng cao

## So Sánh Với Board Gốc

| Tính năng | Lichuang Dev | Lichuang Dev Custom |
|-----------|--------------|-------------------|
| LED GPIO | 48 | 47 |
| AEC Support | Có | Có (với logging) |
| Long Press | Không | Có |
| Custom Features | Không | Có |
| Logging | Cơ bản | Nâng cao |

## Cấu Hình Chân GPIO

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
| Boot Button| 0   | Nút khởi động (click, double-click, long-press) |
| LED       | 47   | Đèn LED (thay đổi từ GPIO 48) |

## Tính Năng Tùy Chỉnh

### Long Press Support
- Nhấn giữ nút boot để kích hoạt tính năng tùy chỉnh
- Log thông tin khi phát hiện long press

### Enhanced Logging
- Log chi tiết hơn trong quá trình khởi tạo
- Hiển thị version của board
- Log trạng thái AEC toggle

### Custom Features Framework
- Framework để thêm tính năng tùy chỉnh
- Dễ dàng mở rộng với sensor hoặc actuator mới

## Yêu Cầu Phần Cứng

- ESP32-S3 development board (Lichuang Dev Custom)
- Màn hình LCD SPI ST7789 320x240 với touch FT5x06
- Codec âm thanh ES8311 + ES7210
- PCA9557 I/O expander
- Camera ESP32
- Micro và loa

## Biên Dịch và Nạp

### Sử dụng script release:
```bash
python scripts/release.py lichuang-dev-custom
```

### Hoặc cấu hình thủ công:
```bash
idf.py set-target esp32s3
idf.py menuconfig  # Chọn "Lichuang Dev Custom (立创·实战派 ESP32-S3 定制版)"
idf.py build
idf.py flash monitor
```

## Khắc Phục Sự Cố

### LED không sáng
- Kiểm tra GPIO 47 có được kết nối đúng không (đã thay đổi từ GPIO 48)

### Long press không hoạt động
- Kiểm tra cấu hình button trong code
- Đảm bảo thời gian nhấn giữ đủ lâu

### Tính năng tùy chỉnh không hoạt động
- Kiểm tra log console để xem lỗi
- Đảm bảo `BOARD_CUSTOM_FEATURE_ENABLED` được define

## Phát Triển Thêm

Board này có thể được mở rộng với:
- Thêm sensor I2C mới
- Tích hợp LED RGB
- Thêm nút bấm vật lý bổ sung
- Kết nối với module không dây khác

## Version History

- **v1.0**: Board tùy chỉnh đầu tiên dựa trên Lichuang Dev
  - Thay đổi GPIO LED từ 48 xuống 47
  - Thêm long press support
  - Enhanced logging
  - Custom features framework