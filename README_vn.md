# Một Chatbot dựa trên MCP

（[中文](README.md) | [English](README_en.md) | [日本語](README_ja.md) | [Tiếng Việt](README_vn.md)）

## Giới thiệu

👉 [Con người: Gắn camera cho AI vs AI: phát hiện chủ nhân ba ngày không gội đầu【bilibili】](https://www.bilibili.com/video/BV1bpjgzKEhd/)

👉 [Tạo bạn gái AI thủ công, hướng dẫn cho người mới【bilibili】](https://www.bilibili.com/video/BV1XnmFYLEJN/)

Xiaozhi (小智) là một chatbot AI dùng giao tiếp bằng giọng nói, tận dụng sức mạnh của các mô hình lớn như Qwen / DeepSeek và thực hiện điều khiển đa đầu cuối thông qua giao thức MCP.

<img src="docs/mcp-based-graph.jpg" alt="通过MCP控制万物" width="320">

### Ghi chú phiên bản

Phiên bản hiện tại v2 không tương thích bảng phân vùng (partition table) với v1, vì vậy không thể nâng cấp OTA từ v1 lên v2. Thông tin bảng phân vùng xem tại [partitions/v2/README.md](partitions/v2/README.md).

Các phần cứng dùng cho v1 vẫn có thể nâng cấp lên v2 bằng cách ghi (flash) thủ công firmware.

Phiên bản ổn định của v1 là 1.9.2; bạn có thể chuyển về nhánh v1 bằng `git checkout v1`. Nhánh này sẽ được duy trì đến tháng 2 năm 2026.

### Tính năng đã thực hiện

- Kết nối Wi‑Fi / ML307 Cat.1 4G
- Đánh thức bằng giọng nói offline (ESP‑SR)
- Hỗ trợ hai giao thức truyền thông ([WebSocket](docs/websocket.md) hoặc MQTT+UDP)
- Mã hóa âm thanh OPUS
- Kiến trúc tương tác giọng nói: ASR streaming + LLM + TTS
- Nhận diện giọng nói (voiceprint) để xác định người nói [3D Speaker]
- Màn hình OLED / LCD, hỗ trợ hiển thị biểu cảm
- Hiển thị pin và quản lý nguồn
- Hỗ trợ đa ngôn ngữ (Tiếng Trung, Tiếng Anh, Tiếng Nhật)
- Hỗ trợ nền tảng ESP32‑C3, ESP32‑S3, ESP32‑P4
- Điều khiển thiết bị (âm lượng, đèn, motor, GPIO, ...) qua MCP trên thiết bị
- Mở rộng năng lực mô hình lớn trên đám mây qua MCP (nhà thông minh, điều khiển desktop, tìm kiếm kiến thức, email...)
- Tùy chỉnh từ đánh thức (wakeword), phông chữ, biểu cảm và nền chat; hỗ trợ chỉnh sửa online qua trình tạo Assets ([xiaozhi-assets-generator](https://github.com/78/xiaozhi-assets-generator))

## Phần cứng

### Thực hành làm trên breadboard

Tham khảo bài hướng dẫn chi tiết trên Feishu:

👉 [“Bách khoa toàn thư Chatbot AI Xiaozhi”](https://ccnphfhqs21z.feishu.cn/wiki/F5krwD16viZoF0kKkvDcrZNYnhb?from=from_copylink)

Hình breadboard:

![面包板效果图](docs/v1/wiring2.jpg)

### Hỗ trợ hơn 70 phần cứng mã nguồn mở (chỉ liệt kê một số)

- 立创·实战派 ESP32‑S3 开发板
- 乐鑫 ESP32‑S3‑BOX3
- M5Stack CoreS3
- AtomS3R + Echo Base
- 神奇按钮 2.4
- 微雪电子 ESP32‑S3‑Touch‑AMOLED‑1.8
- LILYGO T‑Circle‑S3
- 虾哥 Mini C3
- Movecall CuiCan ESP32S3
- 无名科技 Nologo‑星智‑1.54TFT
- SenseCAP Watcher
- ESP‑HI robot

Hình ảnh minh họa nằm trong thư mục `docs/v1/`.

## Phần mềm

### Ghi firmware (flash)

Người mới nên sử dụng firmware có sẵn để ghi mà không cần thiết lập môi trường phát triển.

Firmware mặc định kết nối tới máy chủ chính thức tại [xiaozhi.me](https://xiaozhi.me); người dùng cá nhân có thể đăng ký tài khoản miễn phí để dùng mô hình Qwen thời gian thực.

👉 [Hướng dẫn ghi firmware cho người mới](https://ccnphfhqs21z.feishu.cn/wiki/Zpz4wXBtdimBrLk25WdcXzxcnNS)

### Môi trường phát triển

- Sử dụng Cursor hoặc VSCode
- Cài plugin ESP‑IDF, chọn SDK phiên bản 5.4 trở lên
- Linux thường tốt hơn Windows về tốc độ biên dịch và ít rắc rối driver
- Dự án tuân thủ Google C++ style; khi nộp code hãy đảm bảo định dạng phù hợp

### Tài liệu cho nhà phát triển

- [Hướng dẫn tạo bo mạch tùy chỉnh](docs/custom-board.md)
- [Hướng dẫn sử dụng MCP để điều khiển IoT](docs/mcp-usage.md)
- [Quy trình giao tiếp MCP trên thiết bị](docs/mcp-protocol.md)
- [Tài liệu MQTT + UDP](docs/mqtt-udp.md)
- [Tài liệu giao thức WebSocket chi tiết](docs/websocket.md)

## Cấu hình mô hình lớn (LLM)

Nếu bạn đã có thiết bị Xiaozhi và kết nối với máy chủ chính thức, đăng nhập vào bảng điều khiển tại [xiaozhi.me](https://xiaozhi.me) để cấu hình.

👉 [Video hướng dẫn thao tác trên backend (giao diện cũ)](https://www.bilibili.com/video/BV1jUCUY2EKM/)

## Các dự án mã nguồn liên quan

Nếu muốn triển khai server trên máy cá nhân, tham khảo các dự án sau:

- [xinnan-tech/xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server) (Python)
- [joey-zhou/xiaozhi-esp32-server-java](https://github.com/joey-zhou/xiaozhi-esp32-server-java) (Java)
- [AnimeAIChat/xiaozhi-server-go](https://github.com/AnimeAIChat/xiaozhi-server-go) (Golang)

Khách hàng (client) bên thứ ba dùng giao thức Xiaozhi:

- [huangjunsen0406/py-xiaozhi](https://github.com/huangjunsen0406/py-xiaozhi) (Python)
- [TOM88812/xiaozhi-android-client](https://github.com/TOM88812/xiaozhi-android-client) (Android)
- [100askTeam/xiaozhi-linux](http://github.com/100askTeam/xiaozhi-linux) (Linux)
- [78/xiaozhi-sf32] (https://github.com/78/xiaozhi-sf32) (firmware Bluetooth)
- [QuecPython/solution-xiaozhiAI](https://github.com/QuecPython/solution-xiaozhiAI) (QuecPython firmware)

## Về dự án

Dự án này do 虾哥 (78) phát hành mã nguồn mở dưới giấy phép MIT, cho phép sử dụng, sửa đổi và thương mại hóa.

Chúng tôi hy vọng dự án giúp mọi người hiểu rõ phát triển phần cứng AI và ứng dụng các mô hình ngôn ngữ lớn trên thiết bị thực tế.

Nếu bạn có ý tưởng hoặc góp ý, vui lòng mở Issues hoặc tham gia QQ group: 1011329060

## Lịch sử Star

Mục biểu đồ lịch sử Star vẫn dùng liên kết gốc trong `README.md`.

<a href="https://star-history.com/#78/xiaozhi-esp32&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date" />
 </picture>
</a>
