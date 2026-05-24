#ifndef GUI_H
#define GUI_H

#include <lvgl.h>

// Struct lưu trữ trạng thái hiển thị của 9 thông số nước và kết nối
struct UIState {
    float ph;
    float ec;
    float temp;
    float tds;
    bool wifi_connected;
    bool mqtt_connected;
    const char* time_str;
};

// Khởi tạo hệ thống giao diện
void gui_init(void);

// Cập nhật định kỳ giao diện (Gọi trong main loop)
void gui_handler(void);

// Hàm cập nhật dữ liệu an toàn (được gọi từ MQTT callback)
void gui_update_data(const UIState& new_state);

#endif // GUI_H
