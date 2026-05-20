# 🌿 Smart Farm IoT Gateway (Local Tuya to MQTT)

Dự án này đóng vai trò là lớp Gateway giao tiếp phần cứng nội bộ (Local) cho hệ thống Nông nghiệp Thông minh (Smart Farm). 

Hệ thống bóc tách dữ liệu thời gian thực từ bộ đo chất lượng nước **Tuya PH-W218 (8-in-1)** thông qua mạng LAN, giải phóng hoàn toàn sự phụ thuộc vào Cloud của Tuya. Dữ liệu sau đó được "bắn" (Publish) lên Mosquitto MQTT Broker thông qua cơ chế hướng sự kiện (Event-driven), tạo nền tảng vững chắc để tích hợp với Core Backend (Golang/Node.js) và các thiết bị hiển thị nhúng (ESP32 + TFT Dashboard).

## 📐 Kiến trúc Hệ thống (Decoupled Architecture)

1. **Hardware Layer:** Bộ đo Tuya PH-W218 kết nối WiFi LAN (Đã gán IP Tĩnh).
2. **Gateway Layer:** Home Assistant Core (Docker) + `tuya-local` integration.
3. **Message Broker Layer:** Mosquitto MQTT (Docker) hứng dữ liệu thông qua `mqtt_statestream`.
4. **Application Layer (Tương lai):** - Core Backend (Node.js/Golang) subscribe MQTT để lưu InfluxDB/MongoDB và chạy AI logic.
   - ESP32 (C/C++ LVGL) subscribe MQTT hiển thị HMI Dashboard.

## 📁 Cấu trúc thư mục

```text
smart-farm/
├── docker-compose.yml       # File cấu hình khởi chạy các container
├── homeassistant/           # Volume chứa cấu hình của HA (configuration.yaml, HACS...)
└── mosquitto/               # Volume chứa cấu hình và dữ liệu của MQTT Broker
    └── config/
        └── mosquitto.conf   # File cấu hình port và xác thực MQTT
```

## 🚀 Hướng dẫn quản lý Docker (Vận hành)

Đảm bảo bạn đang đứng ở thư mục gốc của dự án (`cd ~/smart-farm`) trước khi chạy các lệnh sau:

### 1. Khởi động hệ thống (Start)

Lệnh này sẽ tải image (nếu chưa có) và chạy ngầm toàn bộ các services (HA và MQTT) ở chế độ background.

```bash
docker compose up -d
```

### 2. Tắt hệ thống (Stop)

Lệnh này sẽ dừng an toàn và gỡ bỏ các container đang chạy nhưng **KHÔNG** làm mất dữ liệu (vì dữ liệu đã được lưu ở các thư mục `homeassistant` và `mosquitto`).

```bash
docker compose down
```

*(Mẹo: Nếu chỉ muốn tạm dừng mà không gỡ container, dùng lệnh `docker compose stop`)*

### 3. Khởi động lại hệ thống (Restart)

Rất hữu ích khi bạn vừa chỉnh sửa file `docker-compose.yml` hoặc cần reboot lại tài nguyên.

```bash
docker compose restart
```

*(Nếu chỉ muốn restart riêng Home Assistant: `docker restart homeassistant`)*

### 4. Xem Log hệ thống (View Logs)

Dùng để debug xem MQTT hoặc HA có báo lỗi gì không. Lệnh này sẽ hiển thị log realtime.

```bash
docker compose logs -f
```

*(Bấm `Ctrl + C` để thoát màn hình xem log).*

## 🔌 Cấu trúc Dữ liệu MQTT (Data Structure)

Dữ liệu được publish lên MQTT Broker tại địa chỉ IP của máy Host (Port `1883`).
Chỉ các giá trị **thay đổi (state changed)** mới được publish để tối ưu băng thông.

**Base Topic:** `farm/ha/sensor/<entity_id>/state`

**Các Topic lõi đang khả dụng:**

* **Độ pH:** `farm/ha/sensor/water_quality_ph_w218_ph/state` (Ví dụ: `7.65`)
* **Độ dẫn điện (EC):** `farm/ha/sensor/water_quality_ph_w218_electrical_conductivity/state` (Đơn vị: μS/cm)
* **Tổng chất rắn hòa tan (TDS):** `farm/ha/sensor/water_quality_ph_w218_total_dissolved_solids/state` (Đơn vị: ppm)
* **Nhiệt độ nước:** `farm/ha/sensor/water_quality_ph_w218_nhiet_do/state` (Đơn vị: °C)

## 🔑 Lưu ý bảo mật

* Hiện tại `mosquitto.conf` đang được set `allow_anonymous true` để phục vụ phát triển (Development). Khi deploy lên Production, cần thiết lập User/Password cho MQTT Broker.
* Thông tin **Local Key** của thiết bị Tuya là khóa nội bộ, không chia sẻ công khai. Đăng nhập [Tuya IoT Platform](https://iot.tuya.com/) bằng tài khoản Developer (Singapore Data Center) để trích xuất lại Local Key nếu có sự thay đổi về phần cứng.
