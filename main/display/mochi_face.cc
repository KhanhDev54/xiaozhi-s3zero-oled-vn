#include "mochi_face.h"

#include <cstdlib>
#include <cstring>
#include <initializer_list>

#include "application.h"

// Layout inside a ~32x48 area: body 30x26 centred, features relative to the body.
static const int kBodyW = 30;
static const int kBodyH = 25;

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
    lv_obj_set_size(body_, kBodyW, kBodyH);
    lv_obj_set_style_radius(body_, 12, 0);
    lv_obj_set_style_border_width(body_, 2, 0);
    lv_obj_set_style_border_color(body_, lv_color_white(), 0);
    lv_obj_clear_flag(body_, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));

    auto dot = [this]() {
        lv_obj_t* o = lv_obj_create(body_);
        lv_obj_remove_style_all(o);
        lv_obj_set_style_bg_color(o, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE);
        return o;
    };
    eye_[0] = dot();
    eye_[1] = dot();
    cheek_[0] = dot();
    cheek_[1] = dot();
    mouth_ = dot();
    for (auto& l : lines_) {
        l = lv_line_create(body_);
        lv_obj_set_style_line_width(l, 1, 0);
        lv_obj_set_style_line_color(l, lv_color_white(), 0);
        lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    }
    timer_ = lv_timer_create(TimerCb, 120, this);
    Draw();
}

MochiFace::~MochiFace() {
    if (timer_) lv_timer_delete(timer_);
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
    expr_ = x;
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

    // Breathing bob: up/down by 1 px, faster while listening
    int period = listening ? 8 : 16;
    int new_bob = ((tick_ / period) % 2) ? 1 : 0;
    // Blink every 2.5-5 s (not when sleepy)
    if (blink_ > 0) {
        blink_--;
    } else if (--next_blink_ <= 0 && expr_ != kSleepy) {
        blink_ = 1;
        next_blink_ = 20 + rand() % 22;
    }
    // Glance around while listening / thinking / connecting
    int new_look = 0;
    if (listening || busy || expr_ == kThinking) {
        int phase = (tick_ / 12) % 4;
        new_look = phase == 1 ? -1 : (phase == 3 ? 1 : 0);
    }
    // Talking mouth
    bool new_mouth = speaking ? (rand() % 3 != 0) && (tick_ % 2 == 0 || rand() % 2) : false;

    if (new_bob != bob_ || new_look != look_ || new_mouth != mouth_open_ || blink_ > 0 || tick_ % 10 == 0) {
        bob_ = new_bob;
        look_ = new_look;
        mouth_open_ = new_mouth;
        Draw();
    }
}

void MochiFace::HideAll() {
    for (auto* o : {eye_[0], eye_[1], cheek_[0], cheek_[1], mouth_}) lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
    for (auto* l : lines_) lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
}

void MochiFace::SetLine(int i, std::initializer_list<lv_point_precise_t> pts) {
    int n = 0;
    for (auto& p : pts) {
        if (n < 3) pts_[i][n++] = p;
    }
    lv_line_set_points(lines_[i], pts_[i], n);
    lv_obj_remove_flag(lines_[i], LV_OBJ_FLAG_HIDDEN);
}

void MochiFace::Draw() {
    // Body position (bob)
    int bx = (w_ - kBodyW) / 2;
    int by = (h_ - kBodyH) / 2 + 2 + bob_;
    lv_obj_set_pos(body_, bx, by);
    // Squash a little when bobbing down: wider & shorter = soft mochi feel
    lv_obj_set_size(body_, kBodyW + (bob_ ? 1 : 0), kBodyH - (bob_ ? 1 : 0));

    HideAll();
    const int ex0 = 7 + look_, ex1 = 17 + look_, ey = 8;  // inner coords (border 2 px)
    auto eye = [this](int i, int x, int y, int w, int h) {
        lv_obj_set_pos(eye_[i], x, y);
        lv_obj_set_size(eye_[i], w, h);
        lv_obj_remove_flag(eye_[i], LV_OBJ_FLAG_HIDDEN);
    };
    int li = 0;
    bool closed = blink_ > 0;

    switch (expr_) {
        case kHappy:
        case kLove:
            // ^ ^ eyes
            SetLine(li++, {P(ex0 - 1, ey + 3), P(ex0 + 1, ey), P(ex0 + 3, ey + 3)});
            SetLine(li++, {P(ex1 - 1, ey + 3), P(ex1 + 1, ey), P(ex1 + 3, ey + 3)});
            if (expr_ == kLove) {
                lv_obj_set_pos(cheek_[0], 2, 13);
                lv_obj_set_size(cheek_[0], 3, 2);
                lv_obj_set_pos(cheek_[1], 21, 13);
                lv_obj_set_size(cheek_[1], 3, 2);
                lv_obj_remove_flag(cheek_[0], LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(cheek_[1], LV_OBJ_FLAG_HIDDEN);
            }
            break;
        case kSad:
            SetLine(li++, {P(ex0, ey + 1), P(ex0 + 3, ey + 3)});
            SetLine(li++, {P(ex1 + 3, ey + 1), P(ex1, ey + 3)});
            break;
        case kAngry:
            SetLine(li++, {P(ex0 - 1, ey - 1), P(ex0 + 3, ey + 1)});
            SetLine(li++, {P(ex1 + 4, ey - 1), P(ex1, ey + 1)});
            if (!closed) {
                eye(0, ex0, ey + 2, 3, 3);
                eye(1, ex1 + 1, ey + 2, 3, 3);
            }
            break;
        case kSleepy:
            SetLine(li++, {P(ex0, ey + 3), P(ex0 + 4, ey + 3)});
            SetLine(li++, {P(ex1, ey + 3), P(ex1 + 4, ey + 3)});
            break;
        case kSurprised:
            if (closed) {
                SetLine(li++, {P(ex0, ey + 3), P(ex0 + 4, ey + 3)});
                SetLine(li++, {P(ex1, ey + 3), P(ex1 + 4, ey + 3)});
            } else {
                eye(0, ex0, ey, 5, 6);
                eye(1, ex1, ey, 5, 6);
            }
            break;
        default:  // neutral / thinking
            if (closed) {
                SetLine(li++, {P(ex0, ey + 3), P(ex0 + 3, ey + 3)});
                SetLine(li++, {P(ex1, ey + 3), P(ex1 + 3, ey + 3)});
            } else {
                eye(0, ex0, ey + (expr_ == kThinking ? -1 : 0), 4, 5);
                eye(1, ex1, ey + (expr_ == kThinking ? -1 : 0), 4, 5);
            }
            break;
    }

    // Mouth
    const int mx = 11, my = 15;
    if (mouth_open_) {
        lv_obj_set_pos(mouth_, mx, my - 1);
        lv_obj_set_size(mouth_, 5, 4);
        lv_obj_remove_flag(mouth_, LV_OBJ_FLAG_HIDDEN);
    } else {
        switch (expr_) {
            case kHappy:
            case kLove:
                SetLine(li++, {P(mx - 1, my), P(mx + 2, my + 2), P(mx + 5, my)});
                break;
            case kSad:
            case kAngry:
                SetLine(li++, {P(mx - 1, my + 2), P(mx + 2, my), P(mx + 5, my + 2)});
                break;
            case kSurprised:
                lv_obj_set_pos(mouth_, mx + 1, my - 1);
                lv_obj_set_size(mouth_, 3, 3);
                lv_obj_remove_flag(mouth_, LV_OBJ_FLAG_HIDDEN);
                break;
            case kSleepy:
                SetLine(li++, {P(mx, my + 1), P(mx + 4, my + 1)});
                break;
            default:
                SetLine(li++, {P(mx, my + 1), P(mx + 2, my + 2), P(mx + 4, my + 1)});
                break;
        }
    }
}
