#include "idle_screen.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include <cstdio>
#include <ctime>
#include <string>

#include "alarm_manager.h"
#include "application.h"
#include "display.h"
#include "info_services.h"
#include "lunar_calendar.h"

#define TAG "IdleScreen"

LV_FONT_DECLARE(lv_font_montserrat_32);

static const int kShowAfterIdleSeconds = 5;
static const int kLineRotateSeconds = 5;

void IdleScreen::Start(Display* display) {
    display_ = display;
    xTaskCreate(TaskEntry, "idle_screen", 6144, this, 2, nullptr);
}

void IdleScreen::Bump(int seconds) {
    bump_until_us_ = esp_timer_get_time() + (int64_t)seconds * 1000000;
}

void IdleScreen::TaskEntry(void* arg) {
    auto* self = static_cast<IdleScreen*>(arg);
    // Let the application finish booting (SetupUI, assets/fonts) before drawing
    vTaskDelay(pdMS_TO_TICKS(8000));
    while (true) {
        self->Tick();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void IdleScreen::Create() {
    auto* parent = lv_layer_top();
    lv_obj_t* c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_set_size(c, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(c, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* t = lv_label_create(c);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_label_set_text(t, "--:--");
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 4);

    lv_obj_t* l = lv_label_create(c);
    lv_obj_set_style_text_color(l, lv_color_white(), 0);
    lv_obj_set_width(l, LV_HOR_RES);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(l, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(l, "");
    lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, -4);

    container_ = c;
    time_label_ = t;
    line_label_ = l;
}

void IdleScreen::SetVisible(bool visible) {
    if (visible == visible_ || container_ == nullptr) return;
    auto* c = static_cast<lv_obj_t*>(container_);
    if (visible) {
        lv_obj_remove_flag(c, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(c);
    } else {
        lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
    }
    visible_ = visible;
}

void IdleScreen::Tick() {
    tick_++;
    auto& app = Application::GetInstance();
    bool idle = app.GetDeviceState() == kDeviceStateIdle && !AlarmManager::GetInstance().IsRinging();
    idle_seconds_ = idle ? idle_seconds_ + 1 : 0;

    // Weather in the background: retry every minute until the first success,
    // then refresh every 30 minutes (only while idle)
    if (idle) {
        bool have = InfoServices::GetInstance().GetWeather().valid;
        if ((!have && tick_ % 60 == 5) || (have && tick_ % 1800 == 0)) {
            InfoServices::GetInstance().RefreshWeather(1800);
        }
    }

    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    bool time_ok = tmv.tm_year + 1900 >= 2024;
    bool want = time_ok && idle_seconds_ >= kShowAfterIdleSeconds && esp_timer_get_time() >= bump_until_us_;

    if (display_ == nullptr) return;
    DisplayLockGuard lock(display_);
    if (container_ == nullptr) {
        Create();
    }
    if (!want) {
        SetVisible(false);
        return;
    }

    char hhmm[32];
    snprintf(hhmm, sizeof(hhmm), "%02d:%02d", tmv.tm_hour, tmv.tm_min);
    lv_label_set_text(static_cast<lv_obj_t*>(time_label_), hhmm);

    // Use the theme's text font (supports Vietnamese once assets are loaded)
    auto* line = static_cast<lv_obj_t*>(line_label_);
    lv_obj_set_style_text_font(line, lv_obj_get_style_text_font(lv_screen_active(), 0), 0);

    std::string text;
    WeatherNow w = InfoServices::GetInstance().GetWeather();
    bool show_weather = w.valid && ((tick_ / kLineRotateSeconds) % 2 == 1);
    if (show_weather) {
        text = std::to_string(w.temp) + "° " + w.desc;
        if (w.aqi >= 0) text += " · AQI " + std::to_string(w.aqi);
    } else {
        static const char* wd[] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};
        char date[48];
        snprintf(date, sizeof(date), "%s %02d/%02d", wd[tmv.tm_wday % 7], tmv.tm_mday, tmv.tm_mon + 1);
        auto ld = lunar::SolarToLunar(tmv.tm_mday, tmv.tm_mon + 1, tmv.tm_year + 1900);
        char lunar_buf[48];
        snprintf(lunar_buf, sizeof(lunar_buf), "%d/%d%s ÂL", ld.day, ld.month, ld.leap ? "N" : "");
        text = std::string(date) + " · " + lunar_buf;
    }
    if (text != lv_label_get_text(line)) {
        lv_label_set_text(line, text.c_str());
    }
    SetVisible(true);
}
