/**
 * TFT DIAGNOSTIC v3 - VISUAL + SERIAL
 * Không cần Serial Monitor mở đúng thời điểm.
 * LED GPIO48 sẽ nhấp nháy để xác nhận code đang chạy.
 * Serial in liên tục trong loop() để kết nối bất cứ lúc nào.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

// Onboard RGB LED trên ESP32-S3-DevKitC-1
#define LED_PIN   48

TFT_eSPI tft = TFT_eSPI();

// Blink LED GPIO 48 n lần để báo hiệu
void blink_led(int times, int ms = 200) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(ms);
        digitalWrite(LED_PIN, LOW);
        delay(ms);
    }
}

void setup() {
    // Bật LED ngay lập tức để biết setup() bắt đầu chạy
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED sáng liên tục = đang trong setup()

    Serial.begin(115200);
    delay(500); // Chờ ngắn để USB CDC ổn định

    Serial.println("\n========== TFT DIAGNOSTIC v3 ==========");
    Serial.println("Pins: MOSI=13 SCLK=12 CS=15 DC=4 RST=5");
    Serial.println("LED48 blinks: 1x=tft.begin, 2x=fillRed, 3x=loop");

    // Bật backlight nếu cần (LED đã nối vào 3.3V thì bỏ qua)
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    Serial.println("[BL] Backlight pin 2 set HIGH");

    // --- TFT INIT ---
    Serial.println("[TFT] Calling tft.begin()...");
    tft.begin();
    Serial.println("[TFT] tft.begin() returned!");

    blink_led(1, 300); // 1 blink = tft.begin() đã xong

    tft.setRotation(1);
    Serial.printf("[TFT] Resolution: %d x %d\n", tft.width(), tft.height());

    // --- TEST FILL RED ---
    Serial.println("[TEST] fillScreen(RED)...");
    tft.fillScreen(TFT_RED);
    Serial.println("[TEST] RED sent. Is screen red?");

    blink_led(2, 300); // 2 blinks = fillScreen(RED) đã xong

    delay(2000);
    tft.fillScreen(TFT_GREEN);
    Serial.println("[TEST] GREEN sent.");
    delay(2000);

    tft.fillScreen(TFT_BLUE);
    Serial.println("[TEST] BLUE sent.");
    delay(2000);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("SMART FARM", 60, 80);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("pH: 6.20", 80, 120);
    Serial.println("[TEST] Text drawn.");

    blink_led(3, 300); // 3 blinks = setup() hoàn thành, vào loop()

    digitalWrite(LED_PIN, LOW); // Tắt LED khi vào loop()
    Serial.println("[OK] Entering loop()...");
}

void loop() {
    // In trạng thái mỗi 2 giây để monitor bắt được bất cứ lúc nào
    static uint32_t t = 0;
    static int state = 0;
    if (millis() - t > 2000) {
        t = millis();
        state++;

        Serial.printf("[LOOP #%d] uptime=%lus heap=%d PSRAM=%d\n",
            state, millis()/1000, ESP.getFreeHeap(), ESP.getFreePsram());

        // Nhấp nháy LED nhẹ để biết board còn sống
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);

        // Đổi màu góc nhỏ để biết TFT_eSPI có flush được không
        uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN};
        tft.fillCircle(290, 10, 10, colors[state % 5]);
    }
}
