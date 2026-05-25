#include "gui.h"
#include <stdio.h>

// Biến lưu trạng thái hiện tại và trạng thái hiển thị trên màn hình
static UIState current_ui_state = {0.0f, 0.0f, 0.0f, 0.0f, false, false, "00:00"};
static UIState displayed_ui_state = {-1.0f, -1.0f, -1.0f, -1.0f, false, false, ""};

// Các đối tượng widget LVGL
static lv_obj_t *screen_main;
static lv_obj_t *label_ph_val;
static lv_obj_t *label_ec_val;
static lv_obj_t *label_temp_val;
static lv_obj_t *label_tds_val;

static lv_obj_t *led_ph_status;
static lv_obj_t *led_ec_status;

static lv_obj_t *label_mqtt_status;
static lv_obj_t *led_mqtt;
static lv_obj_t *label_time;

// Style cho các thẻ (Cards)
static lv_style_t style_card;

// Hàm bổ trợ tạo một thẻ đo thông số (Card)
static lv_obj_t *create_metric_card(lv_obj_t *parent, const char *title, const char *unit, lv_color_t theme_color, lv_obj_t **value_label, lv_obj_t **status_led)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_style_border_color(card, theme_color, 0);
    lv_obj_set_style_border_width(card, 1, 0); // Viền mỏng 1px sang trọng

    // Tiêu đề của thẻ (ví dụ: "pH", "EC")
    lv_obj_t *lbl_title = lv_label_create(card);
    lv_label_set_text(lbl_title, title);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x8B949E), 0);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_LEFT, 2, -2);

    // Đèn LED chỉ báo trạng thái sinh học (Nếu có)
    if (status_led)
    {
        *status_led = lv_led_create(card);
        lv_obj_set_size(*status_led, 8, 8);
        lv_obj_align(*status_led, LV_ALIGN_TOP_RIGHT, 2, 0);
        lv_led_set_color(*status_led, lv_color_hex(0x00FF87)); // Mặc định xanh lá
        lv_led_on(*status_led);
    }

    // Giá trị số lớn ở trung tâm
    *value_label = lv_label_create(card);
    lv_label_set_text(*value_label, "--.-");
    lv_obj_set_style_text_color(*value_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(*value_label, &lv_font_montserrat_32, 0); // Montserrat 32px
    lv_obj_align(*value_label, LV_ALIGN_CENTER, 0, 5);

    // Đơn vị đo ở phía dưới
    lv_obj_t *lbl_unit = lv_label_create(card);
    lv_label_set_text(lbl_unit, unit);
    lv_obj_set_style_text_color(lbl_unit, theme_color, 0);
    lv_obj_set_style_text_font(lbl_unit, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_unit, LV_ALIGN_BOTTOM_MID, 0, 2);

    return card;
}

void gui_init(void)
{
    // 1. Định nghĩa Style cho Thẻ (Card Style)
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(0x161B22)); // Màu xám tối Obsidian
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12); // Bo góc 12px cực kỳ mượt mà
    lv_style_set_pad_all(&style_card, 6);
    lv_style_set_shadow_width(&style_card, 0); // Tắt bóng để tiết kiệm CPU vẽ

    // 2. Tạo màn hình chính với màu nền Đen sâu
    screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_main, lv_color_hex(0x0D1117), 0);
    lv_scr_load(screen_main);

    // 3. XÂY DỰNG HEADER BAR (Cao 26px)
    lv_obj_t *header = lv_obj_create(screen_main);
    lv_obj_set_size(header, 320, 26);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x0A0D14), 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    // Tiêu đề hệ thống trên Header
    lv_obj_t *logo_title = lv_label_create(header);
    lv_label_set_text(logo_title, "SMART H2O");
    lv_obj_set_style_text_color(logo_title, lv_color_hex(0x8B949E), 0);
    lv_obj_set_style_text_font(logo_title, &lv_font_montserrat_12, 0);
    lv_obj_align(logo_title, LV_ALIGN_LEFT_MID, 8, 0);

    label_mqtt_status = lv_label_create(header);
    lv_label_set_text(label_mqtt_status, "MQTT");
    lv_obj_set_style_text_color(label_mqtt_status, lv_color_hex(0x8B949E), 0);
    lv_obj_set_style_text_font(label_mqtt_status, &lv_font_montserrat_12, 0);
    lv_obj_align(label_mqtt_status, LV_ALIGN_RIGHT_MID, -55, 0);

    // Đèn LED hiển thị trạng thái kết nối MQTT
    led_mqtt = lv_led_create(header);
    lv_obj_set_size(led_mqtt, 6, 6);
    lv_obj_align_to(led_mqtt, label_mqtt_status, LV_ALIGN_OUT_LEFT_MID, -6, 0);
    lv_led_set_color(led_mqtt, lv_color_hex(0xFF4949)); // Đỏ khi chưa kết nối
    lv_led_on(led_mqtt);

    // Đồng hồ hiển thị thời gian
    label_time = lv_label_create(header);
    lv_label_set_text(label_time, "23:10");
    lv_obj_set_style_text_color(label_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_12, 0);
    lv_obj_align(label_time, LV_ALIGN_RIGHT_MID, -10, 0);

    // 4. XÂY DỰNG BỐ CỤC GRID 2x2 CHO DỮ LIỆU CỐT LÕI
    lv_obj_t *grid_container = lv_obj_create(screen_main);
    lv_obj_set_size(grid_container, 320, 214);
    lv_obj_align(grid_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(grid_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid_container, 0, 0);
    lv_obj_set_style_pad_all(grid_container, 6, 0);

    // Khai báo kích thước các cột (2 cột bằng nhau) và hàng (2 hàng bằng nhau)
    static lv_coord_t col_dsc[] = {142, 142, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {94, 94, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(grid_container, col_dsc, row_dsc);
    lv_obj_set_style_pad_row(grid_container, 8, 0);
    lv_obj_set_style_pad_column(grid_container, 8, 0);

    // Tạo 4 thẻ dữ liệu chính
    // Ô [0,0]: pH (Cyan-blue)
    lv_obj_t *card_ph = create_metric_card(grid_container, "pH", "pH Unit", lv_color_hex(0x00F2FE), &label_ph_val, &led_ph_status);
    lv_obj_set_grid_cell(card_ph, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Ô [1,0]: EC (Electric Cyan)
    lv_obj_t *card_ec = create_metric_card(grid_container, "EC (uS/cm)", "uS/cm", lv_color_hex(0x00D2FF), &label_ec_val, &led_ec_status);
    lv_obj_set_grid_cell(card_ec, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Ô [0,1]: Nhiệt độ (Neon Orange)
    lv_obj_t *card_temp = create_metric_card(grid_container, "Temperature", "°C", lv_color_hex(0xFF9F43), &label_temp_val, NULL);
    lv_obj_set_grid_cell(card_temp, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    // Ô [1,1]: TDS (Emerald Green)
    lv_obj_t *card_tds = create_metric_card(grid_container, "TDS", "ppm", lv_color_hex(0x00FF87), &label_tds_val, NULL);
    lv_obj_set_grid_cell(card_tds, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}

void gui_update_data(const UIState &new_state)
{
    // Chỉ cập nhật giá trị vào biến đệm toàn cục (cực nhanh, an toàn cho ngắt)
    current_ui_state = new_state;
}

void gui_handler(void)
{
    // Luôn gọi hàm xử lý nền của LVGL
    lv_timer_handler();

    // KIỂM TRA & CẬP NHẬT GIAO DIỆN CHỈ KHI CÓ THAY ĐỔI (Tối ưu CPU cực lớn)

    // 1. Cập nhật pH
    if (current_ui_state.ph != displayed_ui_state.ph)
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.2f", current_ui_state.ph);
        lv_label_set_text(label_ph_val, buf);

        // Điều khiển màu đèn LED pH dựa theo ngưỡng sinh học thủy canh (Chuẩn 5.5 - 6.5)
        if (current_ui_state.ph >= 5.5f && current_ui_state.ph <= 6.5f)
        {
            lv_led_set_color(led_ph_status, lv_color_hex(0x00FF87)); // Xanh lá: Lý tưởng
        }
        else if ((current_ui_state.ph >= 5.0f && current_ui_state.ph < 5.5f) || (current_ui_state.ph > 6.5f && current_ui_state.ph <= 7.0f))
        {
            lv_led_set_color(led_ph_status, lv_color_hex(0xFF9F43)); // Cam: Hơi lệch
        }
        else
        {
            lv_led_set_color(led_ph_status, lv_color_hex(0xFF4949)); // Đỏ: Nguy hiểm!
        }
        displayed_ui_state.ph = current_ui_state.ph;
    }

    // 2. Cập nhật EC
    if (current_ui_state.ec != displayed_ui_state.ec)
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.0f", current_ui_state.ec);
        lv_label_set_text(label_ec_val, buf);

        // Ngưỡng EC chuẩn xà lách/rau ăn lá (1200 - 1800 uS/cm)
        if (current_ui_state.ec >= 1200.0f && current_ui_state.ec <= 1800.0f)
        {
            lv_led_set_color(led_ec_status, lv_color_hex(0x00FF87)); // Đạt chuẩn
        }
        else
        {
            lv_led_set_color(led_ec_status, lv_color_hex(0xFF9F43)); // Vượt ngưỡng/Thiếu dinh dưỡng
        }
        displayed_ui_state.ec = current_ui_state.ec;
    }

    // 3. Cập nhật Nhiệt độ
    if (current_ui_state.temp != displayed_ui_state.temp)
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.1f", current_ui_state.temp);
        lv_label_set_text(label_temp_val, buf);
        displayed_ui_state.temp = current_ui_state.temp;
    }

    // 4. Cập nhật TDS
    if (current_ui_state.tds != displayed_ui_state.tds)
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.0f", current_ui_state.tds);
        lv_label_set_text(label_tds_val, buf);
        displayed_ui_state.tds = current_ui_state.tds;
    }

    // 5. Cập nhật Trạng thái MQTT trên Header
    if (current_ui_state.mqtt_connected != displayed_ui_state.mqtt_connected)
    {
        if (current_ui_state.mqtt_connected)
        {
            lv_led_set_color(led_mqtt, lv_color_hex(0x00FF87)); // Xanh lá: Đã kết nối MQTT Broker
            lv_label_set_text(label_mqtt_status, "MQTT OK");
        }
        else
        {
            lv_led_set_color(led_mqtt, lv_color_hex(0xFF4949)); // Đỏ: Mất kết nối
            lv_label_set_text(label_mqtt_status, "MQTT ERR");
        }
        displayed_ui_state.mqtt_connected = current_ui_state.mqtt_connected;
    }
}
