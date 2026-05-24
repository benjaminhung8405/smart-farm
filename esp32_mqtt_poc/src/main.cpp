/**
 * DIAGNOSTIC v5 - CHỚP MÀU THỬ NGHIỆM (ILI9341 RAW SPI)
 * ===================================================
 * Vừa chớp đèn nền (GPIO 2) vừa đổi màu màn hình qua SPI.
 * 
 * CS=15, DC=4, RST=5, MOSI=13, SCK=12, BL=2
 */
#include <Arduino.h>
#include <SPI.h>

#define PIN_CS    15
#define PIN_DC    4
#define PIN_RST   5
#define PIN_MOSI  13
#define PIN_SCLK  12
#define PIN_BL    2   // Backlight làm đèn báo hiệu

#define CS_LOW()   digitalWrite(PIN_CS,  LOW)
#define CS_HIGH()  digitalWrite(PIN_CS,  HIGH)
#define DC_CMD()   digitalWrite(PIN_DC,  LOW)
#define DC_DATA()  digitalWrite(PIN_DC,  HIGH)

// RGB565 Colors
#define RED   0xF800
#define GREEN 0x07E0
#define BLUE  0x001F
#define BLACK 0x0000

void spi_write_cmd(uint8_t cmd) {
    DC_CMD(); CS_LOW();
    SPI.transfer(cmd);
    CS_HIGH();
}

void spi_write_data(uint8_t data) {
    DC_DATA(); CS_LOW();
    SPI.transfer(data);
    CS_HIGH();
}

void ili9341_init() {
    // Hardware reset
    digitalWrite(PIN_RST, HIGH); delay(10);
    digitalWrite(PIN_RST, LOW);  delay(20);
    digitalWrite(PIN_RST, HIGH); delay(150);

    spi_write_cmd(0x01); delay(100); // Soft reset
    spi_write_cmd(0x11); delay(100); // Exit Sleep

    // Init sequence đơn giản nhất
    spi_write_cmd(0x3A); spi_write_data(0x55); // 16-bit pixel
    spi_write_cmd(0x36); spi_write_data(0x28); // Xoay ngang màn hình (Landscape BGR)
     spi_write_cmd(0x11); delay(120);
     spi_write_cmd(0x29); delay(50);  // Display ON
    spi_write_cmd(0x11); delay(120);
    spi_write_cmd(0x29); delay(50);  // Display ON
}

void fill_screen(uint16_t color) {
    // Set Address Window
    spi_write_cmd(0x2A);
    spi_write_data(0); spi_write_data(0);
    spi_write_data(319 >> 8); spi_write_data(319 & 0xFF);

    spi_write_cmd(0x2B);
    spi_write_data(0); spi_write_data(0);
    spi_write_data(239 >> 8); spi_write_data(239 & 0xFF);

    spi_write_cmd(0x2C); // Write memory start
    
    DC_DATA(); CS_LOW();
    for (uint32_t i = 0; i < 76800UL; i++) {
        SPI.transfer(color >> 8);
        SPI.transfer(color & 0xFF);
    }
    CS_HIGH();
}

void setup() {
    Serial.begin(115200);
    uint32_t start = millis();
    while (!Serial && (millis() - start < 3000));

    Serial.println("\n--- Starting Diagnostic v5 ---");

    pinMode(PIN_CS,  OUTPUT); CS_HIGH();
    pinMode(PIN_DC,  OUTPUT); DC_CMD();
    pinMode(PIN_RST, OUTPUT); digitalWrite(PIN_RST, HIGH);
    pinMode(PIN_BL,  OUTPUT); digitalWrite(PIN_BL, LOW); // Tắt đèn nền lúc đầu

    // Khởi tạo SPI
    SPI.begin(PIN_SCLK, -1, PIN_MOSI, -1);
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

    // Khởi tạo màn hình
    ili9341_init();
    Serial.println("ILI9341 SPI Init commands sent.");
}

void loop() {
    static int color_step = 0;
    uint16_t colors[] = {RED, GREEN, BLUE};
    uint16_t currentColor = colors[color_step % 3];

    // 1. Gửi lệnh SPI đổi màu màn hình trước khi bật đèn nền
    fill_screen(currentColor);
    Serial.printf("[SPI] Filled screen with color index %d\n", color_step % 3);

    // 2. Bật đèn nền lên để hiển thị màu sắc đó
    digitalWrite(PIN_BL, HIGH);
    Serial.println("[LIGHT] BLINK ON");
    delay(1000);

    // 3. Tắt đèn nền để chuyển sang chu kỳ tiếp theo
    digitalWrite(PIN_BL, LOW);
    Serial.println("[LIGHT] BLINK OFF");
    delay(500);

    color_step++;
}
