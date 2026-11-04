#ifndef ZERO_IDLE_SCREEN_H
#define ZERO_IDLE_SCREEN_H

// Full-screen idle overlay for the 128x64 OLED: big clock + rotating line
// (weekday/date/lunar <-> temperature/condition/AQI). Shown when the device is idle.

#include <cstdint>

class Display;

class IdleScreen {
public:
    static IdleScreen& GetInstance() {
        static IdleScreen instance;
        return instance;
    }

    // Start the 1 Hz update task. Safe to call from a board constructor.
    void Start(Display* display);

    // Hide the overlay for a few seconds (e.g. a button was pressed and a
    // notification such as "Âm lượng 70" should be visible).
    void Bump(int seconds = 4);

private:
    IdleScreen() = default;
    static void TaskEntry(void* arg);
    void Tick();
    void Create();
    void SetVisible(bool visible);

    Display* display_ = nullptr;
    void* container_ = nullptr;  // lv_obj_t*
    void* time_label_ = nullptr;
    void* line_label_ = nullptr;
    bool visible_ = false;
    int idle_seconds_ = 0;
    int tick_ = 0;
    int64_t bump_until_us_ = 0;
};

#endif  // ZERO_IDLE_SCREEN_H
