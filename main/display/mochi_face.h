#ifndef MOCHI_FACE_H
#define MOCHI_FACE_H

// Small animated "mochi" face for monochrome OLEDs (KenDev). A soft rounded blob
// drawn with LVGL shapes (outline only, low power on OLED) with eyes and mouth that
// change with the assistant's emotion, plus idle animations: breathing bob, blinking,
// glancing while listening/thinking, and a talking mouth while speaking.

#include <lvgl.h>
#include <initializer_list>
#include <string>

class MochiFace {
public:
    MochiFace(lv_obj_t* parent, int width, int height);
    ~MochiFace();
    void SetEmotion(const char* emotion);

private:
    enum Expr { kNeutral, kHappy, kSad, kAngry, kSurprised, kSleepy, kThinking, kLove };
    static void TimerCb(lv_timer_t* t);
    void Animate();
    void Draw();
    void SetLine(int i, std::initializer_list<lv_point_precise_t> pts);
    void HideAll();

    lv_obj_t* box_;
    lv_obj_t* body_;
    lv_obj_t* eye_[2];
    lv_obj_t* cheek_[2];
    lv_obj_t* mouth_;
    lv_obj_t* lines_[5];
    lv_point_precise_t pts_[5][3];
    lv_timer_t* timer_;
    int w_, h_;
    Expr expr_ = kNeutral;
    uint32_t tick_ = 0;
    int blink_ = 0;        // >0: eyes closed for N more ticks
    int next_blink_ = 25;
    int look_ = 0;         // -1 left, 0 center, 1 right
    bool mouth_open_ = false;
    int bob_ = 0;
};

#endif
