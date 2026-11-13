#include "mochi_face.h"

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "application.h"

// Peak of the PCM played most recently, written by AudioCodec::OutputData (KenDev).
extern std::atomic<int> g_kendev_audio_peak;

// Design units (scale 1.0): body 30x25, features placed relative to its centre/top.
static const float kBodyW = 30.0f;
static const float kBodyH = 25.0f;
static const float kLargeScale = 1.5f;

static lv_point_precise_t P(int x, int y) {
    lv_point_precise_t p;
    p.x = (lv_value_precise_t)x;
    p.y = (lv_value_precise_t)y;
    return p;
}

MochiFace::MochiFace(lv_obj_t* parent, int width, int height) : w_(width), h_(height) {
    box_ = lv_obj_create(parent);
    lv_obj_remove_style_all(box_);
    lv_obj_set_size(box_, width, height);
    lv_obj_clear_flag(box_, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    lv_obj_center(box_);

    body_ = lv_obj_create(box_);
    lv_obj_remove_style_all(body_);
    lv_obj_set_style_border_width(body_, 2, 0);
    lv_obj_set_style_border_color(body_, lv_color_black(), 0);
    lv_obj_clear_flag(body_, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));

    // Features are siblings of the body (drawn on top of it) so every coordinate is
    // relative to box_, which keeps the maths simple when scaling.
    for (auto& o : dot_) {
        o = lv_obj_create(box_);
        lv_obj_remove_style_all(o);
        lv_obj_set_style_bg_color(o, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(o, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    }
    for (auto& l : lines_) {
        l = lv_line_create(box_);
        lv_obj_set_style_line_width(l, 1, 0);
        lv_obj_set_style_line_color(l, lv_color_black(), 0);
        lv_obj_set_style_line_rounded(l, false, 0);
        lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    }
    timer_ = lv_timer_create(TimerCb, 100, this);
    Draw();
}

MochiFace::~MochiFace() {
    if (timer_) lv_timer_delete(timer_);
}

void MochiFace::SetLayout(int width, int height, bool large) {
    if (width == w_ && height == h_ && large == large_) return;
    w_ = width;
    h_ = height;
    large_ = large;
    k_ = large ? kLargeScale : 1.0f;
    lv_obj_set_size(box_, width, height);
    lv_obj_center(box_);
    int lw = 2;  // 1 px diagonal lines lose pixels when thresholded to mono
    for (auto* l : lines_) {
        lv_obj_set_style_line_width(l, lw, 0);
        // Rounded caps on 1 px lines leave gaps on the OLED; only round the thick ones
        lv_obj_set_style_line_rounded(l, large, 0);
    }
    Draw();
}

void MochiFace::SetEmotion(const char* e) {
    if (e == nullptr) return;
    auto is = [e](const char* s) { return strcmp(e, s) == 0; };
    Expr x = kNeutral;
    if (is("happy") || is("laughing") || is("funny") || is("confident") || is("relaxed") ||
        is("delicious") || is("cool") || is("silly") || is("winking")) x = kHappy;
    else if (is("loving") || is("kissy")) x = kLove;
    else if (is("sad") || is("crying")) x = kSad;
    else if (is("angry")) x = kAngry;
    else if (is("surprised") || is("shocked") || is("embarrassed") || is("confused")) x = kSurprised;
    else if (is("sleepy")) x = kSleepy;
    else if (is("thinking")) x = kThinking;
    if (x != expr_) {
        expr_ = x;
        // A little ear wiggle whenever the mood changes
        ear_twitch_ = 2;
        ear_side_ = rand() % 2;
    }
    Draw();
}

void MochiFace::TimerCb(lv_timer_t* t) {
    static_cast<MochiFace*>(lv_timer_get_user_data(t))->Animate();
}

void MochiFace::Animate() {
    tick_++;
    auto state = Application::GetInstance().GetDeviceState();
    bool speaking = state == kDeviceStateSpeaking;
    bool listening = state == kDeviceStateListening;
    bool busy = state == kDeviceStateConnecting || state == kDeviceStateActivating;
    bool dirty = false;

    // Mouth follows the loudness of what is being played (peak since last tick).
    int peak = g_kendev_audio_peak.exchange(0);
    int target = 0;
    if (speaking) {
        if (peak > 9000) target = 3;
        else if (peak > 3500) target = 2;
        else if (peak > 900) target = 1;
    }
    int new_mouth = target >= mouth_ ? target : mouth_ - 1;  // open fast, close gently

    // Breathing bob: 1 px, faster while listening; a loud syllable also squashes it.
    int period = listening ? 10 : 20;
    int new_bob = ((tick_ / period) % 2) ? 1 : 0;
    if (speaking && new_mouth >= 2) new_bob = 1;

    // Blink every 2.5-5 s (not when sleepy)
    if (blink_ > 0) {
        blink_--;
        dirty = true;
    } else if (--next_blink_ <= 0 && expr_ != kSleepy) {
        blink_ = 1;
        next_blink_ = 25 + rand() % 25;
        dirty = true;
    }

    // Ear twitch every now and then
    if (ear_twitch_ > 0) {
        ear_twitch_--;
        dirty = true;
    } else if (rand() % 70 == 0) {
        ear_twitch_ = 2;
        ear_side_ = rand() % 2;
        dirty = true;
    }

    // Glance around while listening / thinking / connecting
    int new_look = 0;
    if (listening || busy || expr_ == kThinking) {
        int phase = (tick_ / 14) % 4;
        new_look = phase == 1 ? -1 : (phase == 3 ? 1 : 0);
    }

    // Floating heart / z in large mode
    if (large_ && (expr_ == kLove || expr_ == kSleepy) && tick_ % 2 == 0) {
        float_ = (float_ + 1) % 16;
        dirty = true;
    }

    if (dirty || new_bob != bob_ || new_look != look_ || new_mouth != mouth_) {
        bob_ = new_bob;
        look_ = new_look;
        mouth_ = new_mouth;
        Draw();
    }
}

int MochiFace::S(float v) const { return (int)lroundf(v * k_); }

void MochiFace::HideAll() {
    for (auto* o : dot_) lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
    for (auto* l : lines_) lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    used_lines_ = 0;
}

void MochiFace::ShowDot(int i, int x, int y, int w, int h) {
    lv_obj_set_pos(dot_[i], x, y);
    lv_obj_set_size(dot_[i], w < 1 ? 1 : w, h < 1 ? 1 : h);
    lv_obj_remove_flag(dot_[i], LV_OBJ_FLAG_HIDDEN);
}

int MochiFace::AddLine(std::initializer_list<lv_point_precise_t> pts) {
    if (used_lines_ >= kLines) return -1;
    int i = used_lines_++;
    int n = 0;
    for (auto& p : pts) {
        if (n < kMaxPts) pts_[i][n++] = p;
    }
    lv_line_set_points(lines_[i], pts_[i], n);
    lv_obj_remove_flag(lines_[i], LV_OBJ_FLAG_HIDDEN);
    return i;
}

void MochiFace::Draw() {
    HideAll();

    // Body: soft rounded blob, squashed a little (wider & shorter) when bobbing down
    int bw = S(kBodyW) + bob_;
    int bh = S(kBodyH) - bob_;
    int bx = (w_ - bw) / 2;
    int by = large_ ? (h_ - S(kBodyH) - 2) : ((h_ - S(kBodyH)) / 2 + 2);
    by += bob_ * 2;  // the top drops by 2 when squashed so the bottom stays on the "floor"
    lv_obj_set_pos(body_, bx, by);
    lv_obj_set_size(body_, bw, bh);
    lv_obj_set_style_radius(body_, S(12), 0);
    int cx = bx + bw / 2;

    // Cat ears on top of the body. Droop when sad/sleepy, one bends during a twitch.
    bool droop = expr_ == kSad || expr_ == kSleepy;
    for (int side = 0; side < 2; side++) {
        int d = side == 0 ? -1 : 1;
        float tip_x = droop ? 14.0f : 10.0f;
        float tip_y = droop ? -2.0f : -5.0f;
        if (ear_twitch_ > 0 && ear_side_ == side) {
            tip_x += 2.5f;
            tip_y += 2.0f;
        }
        AddLine({P(cx + d * S(12), by + S(4)), P(cx + d * S(tip_x), by + S(tip_y)),
                 P(cx + d * S(4), by + S(0.5f))});
    }

    // Eyes
    bool closed = blink_ > 0;
    int ew = S(4), eh = S(5);
    int ey = by + S(9.5f);  // top of the eye dots
    int exl = cx - S(5) + look_ * S(1);
    int exr = cx + S(5) + look_ * S(1);
    auto closed_eyes = [&]() {
        AddLine({P(exl - S(2), ey + S(3)), P(exl + S(2), ey + S(3))});
        AddLine({P(exr - S(2), ey + S(3)), P(exr + S(2), ey + S(3))});
    };
    switch (expr_) {
        case kHappy:
        case kLove:  // ^ ^
            AddLine({P(exl - S(2), ey + S(3.5f)), P(exl, ey + S(0.5f)), P(exl + S(2), ey + S(3.5f))});
            AddLine({P(exr - S(2), ey + S(3.5f)), P(exr, ey + S(0.5f)), P(exr + S(2), ey + S(3.5f))});
            break;
        case kSad:  // eyes with brows sloping down to the sides
            AddLine({P(exl + S(2), ey), P(exl - S(2), ey + S(1.5f))});
            AddLine({P(exr - S(2), ey), P(exr + S(2), ey + S(1.5f))});
            if (closed) {
                closed_eyes();
            } else {
                ShowDot(0, exl - S(1.5f), ey + S(2.5f), S(3), S(3));
                ShowDot(1, exr - S(1.5f), ey + S(2.5f), S(3), S(3));
            }
            break;
        case kAngry:  // brows sloping down to the middle
            AddLine({P(exl - S(2.5f), ey - S(1)), P(exl + S(2), ey + S(1))});
            AddLine({P(exr + S(2.5f), ey - S(1)), P(exr - S(2), ey + S(1))});
            if (!closed) {
                ShowDot(0, exl - S(1), ey + S(2), S(3), S(3));
                ShowDot(1, exr - S(2), ey + S(2), S(3), S(3));
            }
            break;
        case kSleepy:
            closed_eyes();
            break;
        case kSurprised:
            if (closed) {
                closed_eyes();
            } else {
                ShowDot(0, exl - S(2.5f), ey - S(0.5f), S(5), S(6));
                ShowDot(1, exr - S(2.5f), ey - S(0.5f), S(5), S(6));
            }
            break;
        default:  // neutral / thinking (thinking looks a bit up)
            if (closed) {
                closed_eyes();
            } else {
                int up = expr_ == kThinking ? S(1) : 0;
                ShowDot(0, exl - ew / 2, ey - up, ew, eh);
                ShowDot(1, exr - ew / 2, ey - up, ew, eh);
            }
            break;
    }

    // Blush: always in large mode (except sad/angry), only when loving in small mode
    int my = by + S(18);
    bool blush = large_ ? (expr_ != kSad && expr_ != kAngry && expr_ != kSleepy) : expr_ == kLove;
    if (blush) {
        int bo = large_ ? S(11.5f) : 10;  // keep it off the body outline in small mode
        int bwid = large_ ? S(4) : 3;
        ShowDot(2, cx - bo, my - S(2), bwid, S(2));
        ShowDot(3, cx + bo - bwid, my - S(2), bwid, S(2));
    }

    // Mouth: opens with the voice while speaking
    if (mouth_ > 0) {
        int mh = S(1.0f + mouth_ * 1.3f);
        int mw = S(4.0f + mouth_ * 0.5f);
        ShowDot(4, cx - mw / 2, my - mh / 2, mw, mh);
    } else {
        switch (expr_) {
            case kHappy:
            case kLove:
                AddLine({P(cx - S(3), my - S(1)), P(cx, my + S(1.5f)), P(cx + S(3), my - S(1))});
                break;
            case kSad:
            case kAngry:
                AddLine({P(cx - S(3), my + S(1.5f)), P(cx, my - S(0.5f)), P(cx + S(3), my + S(1.5f))});
                break;
            case kSurprised:
                ShowDot(4, cx - S(1.5f), my - S(1.5f), S(3), S(3));
                break;
            case kSleepy:
                AddLine({P(cx - S(2), my), P(cx + S(2), my)});
                break;
            default:  // small cat mouth "w"
                AddLine({P(cx - S(3), my - S(0.5f)), P(cx - S(1.5f), my + S(1)), P(cx, my - S(0.5f)),
                         P(cx + S(1.5f), my + S(1)), P(cx + S(3), my - S(0.5f))});
                break;
        }
    }

    // Floating heart / z beside the face (large mode only, there is no room otherwise)
    if (large_) {
        int off = float_ < 8 ? float_ : 16 - float_;  // 0..8..0, slow up and down
        if (expr_ == kLove) {
            int hx = cx + S(24);
            int hy = by + S(9) - off;
            int u = (float_ % 4 < 2) ? 2 : 3;  // gentle heartbeat
            AddLine({P(hx, hy + 2 * u), P(hx - 2 * u, hy), P(hx - 2 * u, hy - u), P(hx - u, hy - 2 * u),
                     P(hx, hy - u), P(hx + u, hy - 2 * u), P(hx + 2 * u, hy - u), P(hx + 2 * u, hy),
                     P(hx, hy + 2 * u)});
        } else if (expr_ == kSleepy) {
            int zx = cx + S(21);
            int zy = by + S(4) - off / 2;
            AddLine({P(zx, zy), P(zx + 5, zy), P(zx, zy + 5), P(zx + 5, zy + 5)});
            if (float_ >= 6 && float_ <= 13) {
                int zx2 = zx + 8, zy2 = zy - 7;
                AddLine({P(zx2, zy2), P(zx2 + 3, zy2), P(zx2, zy2 + 3), P(zx2 + 3, zy2 + 3)});
            }
        }
    }
}
