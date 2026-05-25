#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include <time.h>
#include "gui.h"

// ==========================================
// 0. FORWARD DECLARATIONS
// ==========================================
void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void update_ui_state();
void touch_read(lv_indev_drv_t *indev, lv_indev_data_t *data);

// ==========================================
// 1. CẤU HÌNH HỆ THỐNG
// ==========================================
const char *ssid = "FPT Telecom-Phihung";
const char *password = "phihung2005";
const char *mqtt_server = "192.168.1.165";
const int mqtt_port = 1883;

// ==========================================
// 2. KHỞI TẠO ĐỐI TƯỢNG
// ==========================================
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();

// Biến toàn cục lưu trữ 4 thông số cốt lõi
static float water_ph = 0.0f;
static float water_ec = 0.0f;
static float water_temp = 0.0f;
static float water_tds = 0.0f;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *lv_buf1 = nullptr;
static lv_color_t *lv_buf2 = nullptr;
static lv_indev_t *touch_indev = nullptr;

// ==========================================
// 3. HÀM KẾT NỐI WIFI
// ==========================================
void setup_wifi()
{
    delay(10);
    Serial.println("\n------------------------------------");
    Serial.printf("Kết nối vào mạng WiFi: %s\n", ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n➔ WiFi Connected!");
    Serial.print("➔ ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

// ==========================================
// 4. HÀM CALLBACK (Hứng và xử lý dữ liệu MQTT)
// ==========================================
void callback(char *topic, byte *payload, unsigned int length)
{
    String messageBuff;
    messageBuff.reserve(length + 1);
    for (unsigned int i = 0; i < length; i++)
    {
        messageBuff += (char)payload[i];
    }

    String topicStr = String(topic);
    float value = messageBuff.toFloat();

    Serial.printf("[MQTT RECV] Topic: %s | Raw: %s\n", topic, messageBuff.c_str());

    if (topicStr.endsWith("water_quality_ph_w218_ph/state"))
    {
        water_ph = value;
    }
    else if (topicStr.endsWith("water_quality_ph_w218_electrical_conductivity/state"))
    {
        water_ec = value;
    }
    else if (topicStr.endsWith("water_quality_ph_w218_nhiet_do/state"))
    {
        water_temp = value;
    }
    else if (topicStr.endsWith("water_quality_ph_w218_total_dissolved_solids/state"))
    {
        water_tds = value;
    }

    update_ui_state();

    Serial.println("------------------------------------");
}

void update_ui_state()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    static char time_buffer[10] = "--:--";
    
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > 70)
    {
        strftime(time_buffer, sizeof(time_buffer), "%H:%M", &timeinfo);
    }
    else
    {
        strcpy(time_buffer, "--:--");
    }

    UIState new_state;
    new_state.ph = water_ph;
    new_state.ec = water_ec;
    new_state.temp = water_temp;
    new_state.tds = water_tds;
    new_state.wifi_connected = (WiFi.status() == WL_CONNECTED);
    new_state.mqtt_connected = client.connected();
    new_state.time_str = time_buffer;
    gui_update_data(new_state);
}

// ==========================================
// 5. HÀM KẾT NỐI / KẾT NỐI LẠI MQTT BROKER
// ==========================================
void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Đang kết nối MQTT Broker...");

        String clientId = "ESP32-Hydroponics-";
        clientId += String(random(0, 0xffff), HEX);

        if (client.connect(clientId.c_str()))
        {
            Serial.println("Đã kết nối thành công!");
            client.subscribe("farm/ha/sensor/+/state");
            Serial.println("➔ Đã subscribe thành công topic: farm/ha/sensor/+/state");
        }
        else
        {
            Serial.print("Lỗi kết nối, rc=");
            Serial.print(client.state());
            Serial.println(" -> Thử lại sau 5 giây");
            delay(5000);
        }
    }
}

// ==========================================
// 6. LVGL DISP FLUSH
// ==========================================
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(reinterpret_cast<uint16_t *>(color_p), w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void touch_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    LV_UNUSED(indev);

    uint16_t x = 0;
    uint16_t y = 0;
    bool touched = tft.getTouch(&x, &y);

    if (touched)
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ==========================================
// 7. SETUP & LOOP
// ==========================================
void setup()
{
    Serial.begin(115200);

    uint32_t timeout = millis();
    while (!Serial && (millis() - timeout < 4000))
    {
        delay(10);
    }

    setup_wifi();

    // Cấu hình giờ Việt Nam thời gian thực qua NTP (GMT+7: 7 * 3600 = 25200 giây)
    configTime(25200, 0, "pool.ntp.org", "time.nist.gov");

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    tft.begin();
    tft.invertDisplay(false); // Tắt đảo màu để hiển thị đúng theme thiết kế
    tft.setRotation(1);
    tft.setSwapBytes(true);

// Bật đèn nền (backlight) màn hình
#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif

    lv_init();

    const uint32_t buf_pixels = tft.width() * 20;
    const uint32_t buf_size = buf_pixels * sizeof(lv_color_t);
    lv_buf1 = static_cast<lv_color_t *>(heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT));
    lv_buf2 = static_cast<lv_color_t *>(heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT));

    if (!lv_buf1 || !lv_buf2)
    {
        Serial.println("[LVGL] DMA buffer alloc failed, trying PSRAM single buffer");
        if (lv_buf1)
        {
            heap_caps_free(lv_buf1);
            lv_buf1 = nullptr;
        }
        if (lv_buf2)
        {
            heap_caps_free(lv_buf2);
            lv_buf2 = nullptr;
        }
        lv_buf1 = static_cast<lv_color_t *>(heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    }

    if (!lv_buf1)
    {
        Serial.println("[LVGL] Buffer allocation failed");
        while (true)
        {
            delay(1000);
        }
    }

    lv_disp_draw_buf_init(&draw_buf, lv_buf1, lv_buf2, buf_pixels);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = tft.width();
    disp_drv.ver_res = tft.height();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // TODO: Run TFT_eSPI TouchCal and set calibration with tft.setTouch(...) for accurate touch coords.
#ifdef TOUCH_CS
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    touch_indev = lv_indev_drv_register(&indev_drv);
#endif

    gui_init();

    update_ui_state();
}

void loop()
{
    static bool last_mqtt_connected = false;

    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    static uint32_t last_tick = 0;
    uint32_t now = millis();
    uint32_t delta = now - last_tick;
    if (delta > 0)
    {
        lv_tick_inc(delta);
        last_tick = now;
    }

    bool mqtt_connected = client.connected();
    if (mqtt_connected != last_mqtt_connected)
    {
        last_mqtt_connected = mqtt_connected;
        update_ui_state();
    }

    // Cập nhật thời gian thực lên giao diện định kỳ (mỗi 15 giây)
    static uint32_t last_time_update = 0;
    if (now - last_time_update >= 15000)
    {
        last_time_update = now;
        update_ui_state();
    }

    gui_handler();
    delay(5);
}
