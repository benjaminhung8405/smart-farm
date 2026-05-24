# SƠ ĐỒ ĐẤU NỐI MÀN HÌNH TFT 320x240 SPI (ROHS) VỚI ESP32-S3
## DỰ ÁN: SMART FARM HYDROPONICS MONITOR

Chúc mừng bạn đã nạp code (Upload) thành công 100% xuống mạch ESP32-S3! 

Dưới đây là sơ đồ đấu nối chi tiết, chuẩn xác từng chân (Pinout) giữa màn hình **TFT 320x240 SPI (ST7789/ILI9341)** và board **ESP32-S3 DevKitC-1** theo đúng cấu hình cờ biên dịch `platformio.ini` mà anh em mình đã nạp thành công.

---

## I. BẢNG SƠ ĐỒ ĐẤU NỐI (PINOUT MAP)

| Tên chân Màn hình TFT | Ký hiệu khác trên TFT | Chân kết nối ESP32-S3 | Loại tín hiệu | Ghi chú |
| :--- | :--- | :--- | :--- | :--- |
| **VCC** | `VCC / 3V3 / 5V` | **3.3V** | Nguồn dương | Cấp nguồn 3.3V từ mạch ESP32 |
| **GND** | `GND` | **GND** | Nguồn âm | Đất chung |
| **CS** | `CS / Chip Select` | **GPIO 15** | SPI Chip Select | Chọn thiết bị SPI |
| **RESET** | `RST / RESET` | **GPIO 5** | Reset | Khởi tạo lại màn hình |
| **DC** | `D/C / RS / A0` | **GPIO 4** | Data / Command | Phân biệt Lệnh & Dữ liệu |
| **SDI** | `MOSI / SDA` | **GPIO 13** | SPI MOSI | Đường truyền dữ liệu chính |
| **SCK** | `SCLK / CLK` | **GPIO 12** | SPI Clock | Xung nhịp đồng bộ SPI (40MHz) |
| **LED** | `BL / BLK / BACKLIGHT`| **GPIO 2** | Backlight Control| Điều khiển tắt/mở đèn nền |
| **SDO** | `MISO` | **GPIO 14** | SPI MISO | *Không bắt buộc* (Có thể bỏ trống) |

---

## II. SƠ ĐỒ ĐẤU DÂY TRỰC QUAN (VISUAL WIRING SCHEMATIC)

```text
  ┌───────────────────────┐                    ┌────────────────────────┐
  │   MÀN HÌNH TFT SPI    │                    │     ESP32-S3 BOARD     │
  │     (320x240 RoHS)    │                    │      (DevKitC-1)       │
  ├───────────────────────┤                    ├────────────────────────┤
  │ 1. VCC (Nguồn 3.3V)   ├───────────────────>│ 3V3  (Nguồn 3.3V ra)    │
  │ 2. GND (Đất)          ├───────────────────>│ GND  (Đất chung)        │
  │ 3. CS  (Chip Select)  ├───────────────────>│ IO15 (TFT Chip Select)  │
  │ 4. RST (Reset)        ├───────────────────>│ IO5  (Reset màn hình)   │
  │ 5. DC  (Data/Command) ├───────────────────>│ IO4  (Data/Cmd Select)  │
  │ 6. SDI (MOSI)         ├───────────────────>│ IO13 (SPI MOSI Master)  │
  │ 7. SCK (SCLK)         ├───────────────────>│ IO12 (SPI Clock Output) │
  │ 8. LED (Đèn nền)      ├───────────────────>│ IO2  (Bật/Tắt Backlight)│
  │ 9. SDO (MISO)         ├───────────────────>│ IO14 (SPI MISO Input)   │
  └───────────────────────┘                    └────────────────────────┘
```

---

## III. MỘT SỐ LƯU Ý KHI ĐẤU NỐI & VẬN HÀNH

1. **Chân LED (Đèn nền - Backlight):**
   * Trong cấu hình của chúng ta, chân **LED** nối với **GPIO 2**. 
   * Mạch ESP32 sẽ tự động cấu hình GPIO 2 là ngõ ra và kéo lên mức CAO (`HIGH`) để cấp điện cho đèn nền màn hình sáng lên.
   * Nếu bạn đấu nối xong mà màn hình vẫn tối đen, hãy kiểm tra xem dây LED có lỏng không, hoặc thử đấu trực tiếp chân **LED** của màn hình vào chân **3.3V** của ESP32 để kiểm tra xem màn hình có sáng đèn nền không.

2. **Chân Touch (Cảm ứng - Nếu có):**
   * Nếu màn hình của bạn có cụm chân cảm ứng (thường ký hiệu là `T_CLK`, `T_CS`, `T_DIN`, `T_OUT`, `T_IRQ`), bạn **không cần đấu nối** các chân này vì hiện tại giao diện Smart Farm của chúng ta là màn hình dashboard hiển thị theo dõi (Telemetry), không cần thao tác cảm ứng. Hãy để trống các chân này.

3. **Hiện tượng ngược màu (Nếu gặp phải):**
   * Nếu sau khi cắm dây, màn hình hiển thị hình ảnh bình thường nhưng màu sắc bị ngược (ví dụ nền đen biến thành trắng, xanh biến thành đỏ), bạn chỉ cần bật cờ `-D TFT_RGB_ORDER=TFT_RGB` hoặc ngược lại trong file `platformio.ini` để sửa đổi cực kỳ nhanh chóng.
