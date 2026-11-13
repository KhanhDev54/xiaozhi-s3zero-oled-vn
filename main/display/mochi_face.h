#ifndef MOCHI_FACE_H
#define MOCHI_FACE_H

// Small animated "mochi" face for monochrome OLEDs (KenDev). A soft rounded blob
// with cat ears, drawn with LVGL shapes (outline only, low power on OLED). Eyes and
// mouth change with the assistant's emotion, plus idle animations: breathing bob,
// blinking, ear twitch, glancing while listening/thinking, and a mouth that opens
// with the loudness of the voice being played.
//
// Two sizes: small (32 px wide, next to the subtitle) and large (whole content
// area, used while there is no subtitle). Large mode also shows blush, a floating
// heart (loving) and a floating "z" (sleepy).

#include <lvgl.h>
#include <initializer_list>

class MochiFace {
public:
    MochiFace(lv_obj_t* parent, int width, int height);
    ~MochiFace();
    void SetEmotion(const char* emotion);
    // Resize the drawing area. large=true scales the face up by 1.5x.
    void SetLayout(int width, int height, bool large);

private:
    enum Expr { kNeutral, kHappy, kSad, kAngry, kSurprised, kSleepy, kThinking, kLove };
    static constexpr int kLines = 10;
    static constexpr int kMaxPts = 9;

    static void TimerCb(lv_timer_t* t);
    void Animate();
    void Draw();
    int S(float v) const;  // scale a design unit to pixels
    int AddLine(std::initializer_list<lv_point_precise_t> pts);
    void ShowDot(int i, int x, int y, int w, int h);
    void HideAll();

    lv_obj_t* box_;
    lv_obj_t* body_;
    lv_obj_t* dot_[5];  // 0,1 eyes; 2,3 cheeks; 4 mouth
    lv_obj_t* lines_[kLines];
    lv_point_precise_t pts_[kLines][kMaxPts];
    int used_lines_ = 0;
    lv_timer_t* timer_;
    int w_, h_;
    float k_ = 1.0f;
    bool large_ = false;
    Expr expr_ = kNeutral;
    uint32_t tick_ = 0;
    int blink_ = 0;        // >0: eyes closed for N more ticks
    int next_blink_ = 25;
    int look_ = 0;         // -1 left, 0 center, 1 right
    int mouth_ = 0;        // 0 closed, 1..3 how wide open while speaking
    int bob_ = 0;
    int ear_twitch_ = 0;   // >0: one ear bent for N more ticks
    int ear_side_ = 0;
    int float_ = 0;        // phase of the floating heart / z
};

#endif
