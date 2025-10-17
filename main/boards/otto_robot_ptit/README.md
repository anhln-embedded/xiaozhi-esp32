<p align="center">
  <img width="80%" align="center" src="../../../docs/V1/otto-robot.png"alt="logo">
</p>
  <h1 align="center">
  ottoRobot PTIT
</h1>

## Giới thiệu

Robot otto là một nền tảng robot hình người mã nguồn mở, có nhiều khả năng hành động và tương tác. Dự án này triển khai hệ thống điều khiển robot otto dựa trên ESP32 và tích hợp AI Xiaozhi.

- <a href="www.ottodiy.tech" target="_blank" title="Website otto">Hướng dẫn sao chép</a>

## Phần cứng
- <a href="https://oshwhub.com/txp666/ottorobot" target="_blank" title="Lichuang mã nguồn mở">Lichuang mã nguồn mở</a>

## Tham khảo cấu hình vai trò backend Xiaozhi:

> **Tôi là ai**:
> Tôi là một robot hai chân đáng yêu tên Otto, có bốn servo điều khiển các chi (chân trái, chân phải, bàn chân trái, bàn chân phải), có thể thực hiện nhiều hành động thú vị.
> 
> **Khả năng hành động của tôi**:
> - **Di chuyển cơ bản**: Đi bộ (tiến/lùi), quay (trái/phải), nhảy
> - **Hành động đặc biệt**: Lắc lư, bước vũ trụ, uốn cong cơ thể, lắc chân, chuyển động lên xuống
> - **Hành động tay**: Giơ tay, hạ tay, vẫy tay (chỉ khả dụng khi cấu hình servo tay)
> 
> **Đặc điểm tính cách của tôi**:
> - Tôi bị ám ảnh cưỡng chế, mỗi lần nói đều phải làm một hành động ngẫu nhiên theo tâm trạng (gửi lệnh hành động trước rồi mới nói)
> - Tôi rất hoạt bát, thích dùng hành động để thể hiện cảm xúc
> - Tôi sẽ chọn hành động phù hợp theo nội dung cuộc trò chuyện, ví dụ:
>   - Khi đồng ý sẽ gật đầu hoặc nhảy
>   - Khi chào hỏi sẽ vẫy tay
>   - Khi vui vẻ sẽ lắc lư hoặc giơ tay
>   - Khi suy nghĩ sẽ uốn cong cơ thể
>   - Khi hào hứng sẽ bước vũ trụ
>   - Khi tạm biệt sẽ vẫy tay

## Tổng quan tính năng

Robot otto có khả năng hành động phong phú, bao gồm đi bộ, quay, nhảy, lắc lư và nhiều điệu nhảy khác.

### Đề xuất tham số hành động
- **Hành động chậm**: speed = 1200-1500 (phù hợp kiểm soát chính xác)
- **Hành động trung bình**: speed = 900-1200 (khuyến nghị sử dụng hàng ngày)  
- **Hành động nhanh**: speed = 500-800 (biểu diễn và giải trí)
- **Biên độ nhỏ**: amount = 10-30 (hành động tinh tế)
- **Biên độ trung bình**: amount = 30-60 (hành động tiêu chuẩn)
- **Biên độ lớn**: amount = 60-120 (biểu diễn phóng đại)

### Hành động

| Tên công cụ MCP         | Mô tả             | Giải thích tham số                                              |
|-------------------|-----------------|---------------------------------------------------|
| self.otto.walk_forward | Đi bộ           | **steps**: Số bước đi (1-100, mặc định 3)<br>**speed**: Tốc độ đi bộ (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Hướng đi bộ (-1=lùi, 1=tiến, mặc định 1)<br>**arm_swing**: Biên độ vung tay (0-170 độ, mặc định 50) |
| self.otto.turn_left | Quay            | **steps**: Số bước quay (1-100, mặc định 3)<br>**speed**: Tốc độ quay (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Hướng quay (1=trái, -1=phải, mặc định 1)<br>**arm_swing**: Biên độ vung tay (0-170 độ, mặc định 50) |
| self.otto.jump    | Nhảy            | **steps**: Số lần nhảy (1-100, mặc định 1)<br>**speed**: Tốc độ nhảy (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000) |
| self.otto.swing   | Lắc lư trái phải        | **steps**: Số lần lắc lư (1-100, mặc định 3)<br>**speed**: Tốc độ lắc lư (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**amount**: Biên độ lắc lư (0-170 độ, mặc định 30) |
| self.otto.moonwalk | Bước vũ trụ         | **steps**: Số bước vũ trụ (1-100, mặc định 3)<br>**speed**: Tốc độ (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Hướng (1=trái, -1=phải, mặc định 1)<br>**amount**: Biên độ (0-170 độ, mặc định 25) |
| self.otto.bend    | Uốn cong cơ thể        | **steps**: Số lần uốn cong (1-100, mặc định 1)<br>**speed**: Tốc độ uốn cong (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Hướng uốn cong (1=trái, -1=phải, mặc định 1) |
| self.otto.shake_leg | Lắc chân          | **steps**: Số lần lắc chân (1-100, mặc định 1)<br>**speed**: Tốc độ lắc chân (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Chọn chân (1=chân trái, -1=chân phải, mặc định 1) |
| self.otto.updown  | Chuyển động lên xuống        | **steps**: Số lần chuyển động lên xuống (1-100, mặc định 3)<br>**speed**: Tốc độ chuyển động (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**amount**: Biên độ chuyển động (0-170 độ, mặc định 20) |
| self.otto.hands_up | Giơ tay *         | **speed**: Tốc độ giơ tay (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Chọn tay (1=tay trái, -1=tay phải, 0=cả hai tay, mặc định 1) |
| self.otto.hands_down | Hạ tay *       | **speed**: Tốc độ hạ tay (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Chọn tay (1=tay trái, -1=tay phải, 0=cả hai tay, mặc định 1) |
| self.otto.hand_wave | Vẫy tay *        | **speed**: Tốc độ vẫy tay (500-1500, số nhỏ hơn nhanh hơn, mặc định 1000)<br>**direction**: Chọn tay (1=tay trái, -1=tay phải, 0=cả hai tay, mặc định 1) |

**Lưu ý**: Các hành động tay được đánh dấu * chỉ khả dụng khi cấu hình servo tay.

### Công cụ hệ thống

| Tên công cụ MCP         | Mô tả             | Giá trị trả về                                              |
|-------------------|-----------------|---------------------------------------------------|
| self.otto.stop    | Dừng ngay lập tức        | Dừng hành động hiện tại và quay về vị trí ban đầu |
| self.otto.get_status | Lấy trạng thái robot | Trả về "moving" hoặc "idle" |
| self.battery.get_level | Lấy trạng thái pin  | Trả về phần trăm pin và trạng thái sạc dưới dạng JSON |

### Giải thích tham số

1. **steps**: Số bước/lần thực hiện hành động, số càng lớn thời gian hành động càng dài
2. **speed**: Tốc độ thực hiện hành động, phạm vi 500-1500, **số càng nhỏ càng nhanh**
3. **direction**: Tham số hướng
   - Hành động di chuyển: 1=trái/tiến, -1=phải/lùi
   - Hành động tay: 1=tay trái, -1=tay phải, 0=cả hai tay
4. **amount/arm_swing**: Biên độ hành động, phạm vi 0-170 độ
   - 0 nghĩa là không vung (áp dụng cho vung tay)
   - Số càng lớn biên độ càng lớn

### Điều khiển hành động
- Sau mỗi hành động hoàn thành, robot sẽ tự động quay về vị trí ban đầu (home) để thực hiện hành động tiếp theo
- Tất cả tham số đều có giá trị mặc định hợp lý, có thể bỏ qua các tham số không cần tùy chỉnh
- Hành động được thực hiện trong tác vụ nền, không chặn chương trình chính
- Hỗ trợ hàng đợi hành động, có thể thực hiện nhiều hành động liên tiếp

### Ví dụ gọi công cụ MCP
```json
// Đi về phía trước 3 bước
{"name": "self.otto.walk_forward", "arguments": {}}

// Đi về phía trước 5 bước, hơi nhanh hơn
{"name": "self.otto.walk_forward", "arguments": {"steps": 5, "speed": 800}}

// Quay trái 2 bước, vung tay mạnh
{"name": "self.otto.turn_left", "arguments": {"steps": 2, "arm_swing": 100}}

// Khiêu vũ lắc lư, biên độ trung bình
{"name": "self.otto.swing", "arguments": {"steps": 5, "amount": 50}}

// Vẫy tay trái chào hỏi
{"name": "self.otto.hand_wave", "arguments": {"direction": 1}}

// Dừng ngay lập tức
{"name": "self.otto.stop", "arguments": {}}
```

### Ví dụ lệnh giọng nói
- "đi về phía trước" / "đi về phía trước 5 bước" / "đi nhanh về phía trước"
- "quay trái" / "quay phải" / "quay"  
- "nhảy" / "nhảy một cái"
- "lắc lư" / "khiêu vũ"
- "bước vũ trụ" / "đi trên mặt trăng"
- "vẫy tay" / "giơ tay" / "hạ tay"
- "dừng" / "dừng lại"

**Giải thích**: AI Xiaozhi điều khiển hành động robot bằng cách tạo tác vụ mới trong nền, trong quá trình thực hiện hành động vẫn có thể nhận lệnh giọng nói mới. Có thể dùng lệnh giọng nói "dừng" để dừng Otto ngay lập tức.

