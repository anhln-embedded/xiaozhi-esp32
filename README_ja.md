# Chatbot dựa trên MCP

（Tiếng Nhật | [Tiếng Trung](README.md) | [Tiếng Anh](README_en.md) | [Tiếng Việt](README_vn.md)）

## Giới thiệu

👉 [Con người: Trang bị camera cho AI vs AI: Phát hiện chủ nhân không gội đầu 3 ngày【bilibili】](https://www.bilibili.com/video/BV1bpjgzKEhd/)

👉 [Tự làm AI bạn gái, hướng dẫn nhập môn cho người mới bắt đầu【bilibili】](https://www.bilibili.com/video/BV1XnmFYLEJN/)

Xiaozhi AI Chatbot là cửa ngõ tương tác giọng nói, tận dụng khả năng AI của các mô hình lớn như Qwen / DeepSeek, và thực hiện kiểm soát đa đầu cuối thông qua giao thức MCP.

<img src="docs/mcp-based-graph.jpg" alt="Kiểm soát mọi thứ với MCP" width="320">

## Ghi chú phiên bản

Phiên bản v2 hiện tại không tương thích với bảng phân vùng v1, vì vậy không thể nâng cấp OTA từ v1 lên v2. Để biết chi tiết về bảng phân vùng, vui lòng tham khảo [partitions/v2/README.md](partitions/v2/README.md).

Tất cả phần cứng đang chạy v1 có thể nâng cấp lên v2 bằng cách ghi firmware thủ công.

Phiên bản ổn định của v1 là 1.9.2. Bạn có thể chuyển sang v1 bằng `git checkout v1`. Nhánh v1 sẽ được duy trì liên tục cho đến tháng 2 năm 2026.

### Các tính năng đã triển khai

- Wi-Fi / ML307 Cat.1 4G
- Đánh thức giọng nói offline [ESP-SR](https://github.com/espressif/esp-sr)
- Hỗ trợ 2 loại giao thức truyền thông ([Websocket](docs/websocket.md) hoặc MQTT+UDP)
- Sử dụng codec âm thanh OPUS
- Tương tác giọng nói dựa trên kiến trúc streaming ASR + LLM + TTS
- Nhận dạng người nói, xác định người đang nói [3D Speaker](https://github.com/modelscope/3D-Speaker)
- Hiển thị OLED / LCD, hỗ trợ biểu cảm khuôn mặt
- Hiển thị pin và quản lý nguồn
- Hỗ trợ đa ngôn ngữ (Tiếng Trung, Tiếng Anh, Tiếng Nhật, Tiếng Việt)
- Hỗ trợ nền tảng chip ESP32-C3, ESP32-S3, ESP32-P4
- Kiểm soát thiết bị qua MCP phía thiết bị (điều chỉnh âm lượng, độ sáng, kiểm soát hành động, v.v.)
- Mở rộng khả năng mô hình lớn qua MCP phía đám mây (kiểm soát nhà thông minh, thao tác desktop PC, tìm kiếm kiến thức, gửi/nhận email, v.v.)
- Hỗ trợ tùy chỉnh từ đánh thức, font chữ, emoji, nền chat, chỉnh sửa web online ([Trình tạo tài sản tùy chỉnh](https://github.com/78/xiaozhi-assets-generator))

## Phần cứng

### Thực hành tự làm breadboard

Vui lòng xem hướng dẫn tài liệu Feishu:

👉 ["Bách khoa toàn thư Xiaozhi AI Chatbot"](https://ccnphfhqs21z.feishu.cn/wiki/F5krwD16viZoF0kKkvDcrZNYnhb?from=from_copylink)

Demo breadboard:

![Demo breadboard](docs/v1/wiring2.jpg)

### Hỗ trợ hơn 70 loại phần cứng mã nguồn mở (chỉ hiển thị một phần)

- <a href="https://oshwhub.com/li-chuang-kai-fa-ban/li-chuang-shi-zhan-pai-esp32-s3-kai-fa-ban" target="_blank" title="Lichuang・Thực chiến ESP32-S3 Development Board">Lichuang・Thực chiến ESP32-S3 Development Board</a>
- <a href="https://github.com/espressif/esp-box" target="_blank" title="Espressif ESP32-S3-BOX3">Espressif ESP32-S3-BOX3</a>
- <a href="https://docs.m5stack.com/zh_CN/core/CoreS3" target="_blank" title="M5Stack CoreS3">M5Stack CoreS3</a>
- <a href="https://docs.m5stack.com/en/atom/Atomic%20Echo%20Base" target="_blank" title="AtomS3R + Echo Base">M5Stack AtomS3R + Echo Base</a>
- <a href="https://gf.bilibili.com/item/detail/1108782064" target="_blank" title="Magic Button 2.4">Magic Button 2.4</a>
- <a href="https://www.waveshare.net/shop/ESP32-S3-Touch-AMOLED-1.8.htm" target="_blank" title="Waveshare ESP32-S3-Touch-AMOLED-1.8">Waveshare ESP32-S3-Touch-AMOLED-1.8</a>
- <a href="https://github.com/Xinyuan-LilyGO/T-Circle-S3" target="_blank" title="LILYGO T-Circle-S3">LILYGO T-Circle-S3</a>
- <a href="https://oshwhub.com/tenclass01/xmini_c3" target="_blank" title="Tenclass Mini C3">Tenclass Mini C3</a>
- <a href="https://oshwhub.com/movecall/cuican-ai-pendant-lights-up-y" target="_blank" title="Movecall CuiCan ESP32S3">CuiCan AI Pendant</a>
- <a href="https://github.com/WMnologo/xingzhi-ai" target="_blank" title="WM Nologo-Xingzhi-1.54">WM Nologo-Xingzhi-1.54TFT</a>
- <a href="https://www.seeedstudio.com/SenseCAP-Watcher-W1-A-p-5979.html" target="_blank" title="SenseCAP Watcher">SenseCAP Watcher</a>
- <a href="https://www.bilibili.com/video/BV1BHJtz6E2S/" target="_blank" title="ESP-HI siêu rẻ robot chó">ESP-HI siêu rẻ robot chó</a>

<div style="display: flex; justify-content: space-between;">
  <a href="docs/v1/lichuang-s3.jpg" target="_blank" title="立創・実戦派 ESP32-S3 開発ボード">
    <img src="docs/v1/lichuang-s3.jpg" width="240" />
  </a>
  <a href="docs/v1/espbox3.jpg" target="_blank" title="楽鑫 ESP32-S3-BOX3">
    <img src="docs/v1/espbox3.jpg" width="240" />
  </a>
  <a href="docs/v1/m5cores3.jpg" target="_blank" title="M5Stack CoreS3">
    <img src="docs/v1/m5cores3.jpg" width="240" />
  </a>
  <a href="docs/v1/atoms3r.jpg" target="_blank" title="AtomS3R + Echo Base">
    <img src="docs/v1/atoms3r.jpg" width="240" />
  </a>
  <a href="docs/v1/magiclick.jpg" target="_blank" title="マジックボタン2.4">
    <img src="docs/v1/magiclick.jpg" width="240" />
  </a>
  <a href="docs/v1/waveshare.jpg" target="_blank" title="微雪電子 ESP32-S3-Touch-AMOLED-1.8">
    <img src="docs/v1/waveshare.jpg" width="240" />
  </a>
  <a href="docs/v1/lilygo-t-circle-s3.jpg" target="_blank" title="LILYGO T-Circle-S3">
    <img src="docs/v1/lilygo-t-circle-s3.jpg" width="240" />
  </a>
  <a href="docs/v1/xmini-c3.jpg" target="_blank" title="エビ兄さん Mini C3">
    <img src="docs/v1/xmini-c3.jpg" width="240" />
  </a>
  <a href="docs/v1/movecall-cuican-esp32s3.jpg" target="_blank" title="CuiCan">
    <img src="docs/v1/movecall-cuican-esp32s3.jpg" width="240" />
  </a>
  <a href="docs/v1/wmnologo_xingzhi_1.54.jpg" target="_blank" title="無名科技Nologo-星智-1.54">
    <img src="docs/v1/wmnologo_xingzhi_1.54.jpg" width="240" />
  </a>
  <a href="docs/v1/sensecap_watcher.jpg" target="_blank" title="SenseCAP Watcher">
    <img src="docs/v1/sensecap_watcher.jpg" width="240" />
  </a>
  <a href="docs/v1/esp-hi.jpg" target="_blank" title="ESP-HI 超低コストロボット犬">
    <img src="docs/v1/esp-hi.jpg" width="240" />
  </a>
</div>

## Phần mềm

### Ghi firmware

Người mới bắt đầu nên sử dụng firmware có thể ghi mà không cần xây dựng môi trường phát triển trước.

Firmware mặc định kết nối với máy chủ chính thức [xiaozhi.me](https://xiaozhi.me). Người dùng cá nhân có thể đăng ký tài khoản để sử dụng mô hình Qwen thời gian thực miễn phí.

👉 [Hướng dẫn ghi firmware cho người mới bắt đầu](https://ccnphfhqs21z.feishu.cn/wiki/Zpz4wXBtdimBrLk25WdcXzxcnNS)

### Môi trường phát triển

- Cursor hoặc VSCode
- Cài đặt plugin ESP-IDF, chọn phiên bản SDK 5.4 trở lên
- Linux tốt hơn Windows, biên dịch nhanh hơn và ít vấn đề về driver
- Dự án này sử dụng phong cách code Google C++, vui lòng tuân thủ khi submit code

### Tài liệu nhà phát triển

- [Hướng dẫn tạo board tùy chỉnh](docs/custom-board.md) - Cách tạo board phát triển tùy chỉnh cho Xiaozhi AI
- [Cách sử dụng MCP Protocol để kiểm soát IoT](docs/mcp-usage.md) - Cách kiểm soát thiết bị IoT bằng giao thức MCP
- [Luồng tương tác giao thức MCP](docs/mcp-protocol.md) - Cách triển khai giao thức MCP phía thiết bị
- [Tài liệu giao thức truyền thông MQTT + UDP hybrid](docs/mqtt-udp.md)
- [Tài liệu giao thức truyền thông WebSocket chi tiết](docs/websocket.md)

## Cài đặt mô hình lớn

Nếu bạn đã có thiết bị Xiaozhi AI Chatbot và đã kết nối với máy chủ chính thức, bạn có thể cấu hình tại bảng điều khiển [xiaozhi.me](https://xiaozhi.me).

👉 [Video hướng dẫn thao tác backend (giao diện cũ)](https://www.bilibili.com/video/BV1jUCUY2EKM/)

## Các dự án mã nguồn mở liên quan

Nếu muốn triển khai máy chủ trên PC cá nhân, vui lòng tham khảo các dự án mã nguồn mở sau:

- [xinnan-tech/xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server) Máy chủ Python
- [joey-zhou/xiaozhi-esp32-server-java](https://github.com/joey-zhou/xiaozhi-esp32-server-java) Máy chủ Java
- [AnimeAIChat/xiaozhi-server-go](https://github.com/AnimeAIChat/xiaozhi-server-go) Máy chủ Golang

Các dự án client khác sử dụng giao thức truyền thông Xiaozhi:

- [huangjunsen0406/py-xiaozhi](https://github.com/huangjunsen0406/py-xiaozhi) Client Python
- [TOM88812/xiaozhi-android-client](https://github.com/TOM88812/xiaozhi-android-client) Client Android
- [100askTeam/xiaozhi-linux](http://github.com/100askTeam/xiaozhi-linux) Client Linux do 100askTeam cung cấp
- [78/xiaozhi-sf32](https://github.com/78/xiaozhi-sf32) Firmware chip Bluetooth do Sich Tech cung cấp
- [QuecPython/solution-xiaozhiAI](https://github.com/QuecPython/solution-xiaozhiAI) Firmware QuecPython do Quectel cung cấp

## Về dự án

Đây là dự án ESP32 mã nguồn mở được công khai bởi anh em Tenclass, dưới giấy phép MIT, mọi người đều có thể sử dụng miễn phí, bao gồm cả mục đích thương mại.

Thông qua dự án này, chúng tôi mong muốn mọi người có thể hiểu được việc phát triển phần cứng AI, và áp dụng các mô hình ngôn ngữ lớn đang phát triển nhanh chóng vào các thiết bị phần cứng thực tế.

Nếu bạn có ý kiến hoặc đề xuất, vui lòng tạo Issue hoặc tham gia nhóm QQ: 1011329060.

## Lịch sử Star

<a href="https://star-history.com/#78/xiaozhi-esp32&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=78/xiaozhi-esp32&type=Date" />
 </picture>
</a> 
