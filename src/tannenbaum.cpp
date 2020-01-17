#include <Arduino.h>
#include <Ticker.h>

#include "info_debug_error.h"
#include "tannenbaum.hpp"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

/////////// public

Tannenbaum::Tannenbaum(APIServer& http_server, enum OP_MODES op_mode)
    // public
    : mplayer{Tannenbaum::audio_gpio, Tannenbaum::audio_pwm_channel}
    // private
    , http_server{http_server}
    , buttons{}
    , pattern_timer{}
    , tone_timer{}
    , op_mode{LARSON}
    , led_state_all_on{false}
    , pattern_interval{100}
{
    debug_print("Configuring Tannenbaum...");
    init_pwm_gpios();
    switch (op_mode) {
        case LARSON: set_mode_larson(); break;
        case SPIN_RIGHT: set_mode_spinning(true); break;
        case SPIN_LEFT: set_mode_spinning(false); break;
        case ARROW_UP: set_mode_arrow(true); break;
        case ARROW_DOWN: set_mode_arrow(false); break;
        case ALL_ON_OFF: set_mode_all_on_off(); break;
    }
    // Remote control interface
    setup_http_interface();
    // Initial state
    http_server.set_template("ON_OFF_BTN_STATE", "btn_off");
    // Local touch buttons interface
    setup_touch_buttons();
    // Start timer for LED pattern updading
    pattern_timer.attach_ms(pattern_interval, on_timer_event, this);
    // Configure melody player
    mplayer.set_tempo(64);
}

Tannenbaum::~Tannenbaum() {
    pattern_timer.detach();
}

void Tannenbaum::set_mode_larson() {
    debug_print("New Operation Mode: Scanning Larson");
    op_mode = LARSON;
    // Attach GPIO pins to PWM channels
    ledcAttachPin(32, 0); // Links unten
    ledcAttachPin(21, 0); // Rechts unten
    ledcAttachPin(33, 1); // Links
    ledcAttachPin(19, 1); // Rechts
    ledcAttachPin(25, 2); // Links
    ledcAttachPin(18, 2); // Rechts
    ledcAttachPin(26, 3); // Links
    ledcAttachPin(17, 3); // Rechts
    ledcAttachPin(27, 4); // Links oberster
    ledcAttachPin(16, 4); // Rechts oberster
    ledcAttachPin(4, 5); // Rechteckige oben
    ledcAttachPin(14, 6); // Baumkrone rund
}

void Tannenbaum::set_mode_spinning(bool direction) {
    if (direction) { 
        debug_print("New Operation Mode: Spinning right");
        op_mode = SPIN_RIGHT;
    } else {
        debug_print("New Operation Mode: Spinning left");
        op_mode = SPIN_LEFT;
    }
    // Attach GPIO pins to PWM channels
    ledcAttachPin(32, 0); // Links unten
    ledcAttachPin(33, 1); // Links
    ledcAttachPin(25, 2); // Links
    ledcAttachPin(26, 3); // Links
    ledcAttachPin(27, 4); // Links oberster
    ledcAttachPin(4, 5); // Rechteckige oben
    ledcAttachPin(14, 6); // Baumkrone rund
    ledcAttachPin(16, 7); // Rechts oberster
    ledcAttachPin(17, 8); // Rechts
    ledcAttachPin(18, 9); // Rechts
    ledcAttachPin(19, 10); // Rechts
    ledcAttachPin(21, 11); // Rechts unten
}

void Tannenbaum::set_mode_arrow(bool direction) {
    if (direction) { 
        debug_print("New Operation Mode: Upwards pointing arrow");
        op_mode = ARROW_UP;
    } else {
        debug_print("New Operation Mode: Downwards pointing arrow");
        op_mode = ARROW_DOWN;
    }
    // Attach GPIO pins to PWM channels
    ledcAttachPin(32, 0); // Links unten
    ledcAttachPin(21, 0); // Rechts unten
    ledcAttachPin(33, 1); // Links
    ledcAttachPin(19, 1); // Rechts
    ledcAttachPin(25, 2); // Links
    ledcAttachPin(18, 2); // Rechts
    ledcAttachPin(26, 3); // Links
    ledcAttachPin(17, 3); // Rechts
    ledcAttachPin(27, 4); // Links oberster
    ledcAttachPin(16, 4); // Rechts oberster
    ledcAttachPin(4, 5); // Rechteckige oben
    ledcAttachPin(14, 6); // Baumkrone rund
}

void Tannenbaum::set_mode_all_on_off() {
    debug_print("New Operation Mode: All on or all off");
    op_mode = ALL_ON_OFF;
    // Attach GPIO pins to PWM channels
    ledcAttachPin(32, 0); // Links unten
    ledcAttachPin(21, 0); // Rechts unten
    ledcAttachPin(33, 0); // Links
    ledcAttachPin(19, 0); // Rechts
    ledcAttachPin(25, 0); // Links
    ledcAttachPin(18, 0); // Rechts
    ledcAttachPin(26, 0); // Links
    ledcAttachPin(17, 0); // Rechts
    ledcAttachPin(27, 0); // Links oberster
    ledcAttachPin(16, 0); // Rechts oberster
    ledcAttachPin(4, 0); // Rechteckige oben
    ledcAttachPin(14, 0); // Baumkrone rund
}

bool Tannenbaum::toggle_on_off_state() {
    set_mode_all_on_off();
    led_state_all_on = !led_state_all_on;
    update_all_on_off();
    if(led_state_all_on) {
        http_server.set_template("ON_OFF_BTN_STATE", "");
        mplayer.play(
            {G, G, L4,E, P, G, F, E,
            L4,F, L4,E, L4,D, P, C, A, C, C, C, E, E, D, C, L4,D, L4,P, L2,P,
            F, A, L4,A, P, A, G, F, G, F, L4,E, L4,P, P, E, D, Fs, L4,A, P, D, D, B,
            L4,A, L4,G, L4,G, L4,P, C, C, A, G, L4,G, L4,F, E, G, G, A, L4,G, L2, P
            },
            196
        );
    } else {
        http_server.set_template("ON_OFF_BTN_STATE", "btn_off");
    }
    debug_print_sv("Setting LED state to: ", led_state_all_on ? "ON" : "OFF");
    return led_state_all_on;
}

void Tannenbaum::increase_speed() {
    debug_print("Faster.");
    if (pattern_interval >= 2) {
        pattern_interval /= 2;
    }
    pattern_timer.attach_ms(pattern_interval, on_timer_event, this);
}

void Tannenbaum::decrease_speed() {
    debug_print("Slower..");
    if (pattern_interval < 4096) {
        pattern_interval *= 2;
    }
    pattern_timer.attach_ms(pattern_interval, on_timer_event, this);
}

///////////// private

void Tannenbaum::setup_http_interface() {
    http_server.register_api_cb("larson", [this](){set_mode_larson();});
    http_server.register_api_cb("spin_right", [this](){set_mode_spinning(true);});
    http_server.register_api_cb("spin_left", [this](){set_mode_spinning(false);});
    http_server.register_api_cb("arrow_up", [this](){set_mode_arrow(true);});
    http_server.register_api_cb("arrow_down", [this](){set_mode_arrow(false);});
    http_server.register_api_cb("on_off", [this](){
        toggle_on_off_state();
        mplayer.play({C, D, E, P, C});
    });
    http_server.register_api_cb("plus", [this](){
        increase_speed();
        mplayer.play({C, D, L2, E});
    });
    http_server.register_api_cb("minus", [this](){
        decrease_speed();
        mplayer.play({E, D, L2, C});
    });
}

void Tannenbaum::setup_touch_buttons() {
    buttons.configure_input(touch_io_left, touch_threshold_percent, [this](){
        if (op_mode == ALL_ON_OFF) {
            toggle_on_off_state();
        } else {
            decrease_speed();
            mplayer.play({E, D, L2, C});
        }
    });
    buttons.configure_input(touch_io_middle, touch_threshold_percent, [this](){
        if (op_mode == ALL_ON_OFF) {
            toggle_on_off_state();
        } else {
            increase_speed();
            mplayer.play({C, D, L2, E});
        }
    });
    buttons.configure_input(touch_io_right, touch_threshold_percent, [this](){
        switch (op_mode) {
            case LARSON: set_mode_spinning(true); break;
            case SPIN_RIGHT: set_mode_spinning(false); break;
            case SPIN_LEFT: set_mode_arrow(true); break;
            case ARROW_UP: set_mode_arrow(false); break;
            case ARROW_DOWN: set_mode_all_on_off(); break;
            case ALL_ON_OFF: set_mode_larson(); break;
        }
        mplayer.play({C, D, E, P, C});
    });
    buttons.begin();
}

void Tannenbaum::init_pwm_gpios() {
    // Setup PWM channels for LEDs
    ledcSetup(0, pwm_freq, 8);
    ledcSetup(1, pwm_freq, 8);
    ledcSetup(2, pwm_freq, 8);
    ledcSetup(3, pwm_freq, 8);
    ledcSetup(4, pwm_freq, 8);
    ledcSetup(5, pwm_freq, 8);
    ledcSetup(6, pwm_freq, 8);
    ledcSetup(7, pwm_freq, 8);
    ledcSetup(8, pwm_freq, 8);
    ledcSetup(9, pwm_freq, 8);
    ledcSetup(10, pwm_freq, 8);
    ledcSetup(11, pwm_freq, 8);
    ledcSetup(12, pwm_freq, 8);
}


void Tannenbaum::update_larson() {
    constexpr int n_leds = 7;
    // Pattern is one uint8 value for setting output PWM duty cycle.
    constexpr uint16_t led_pattern[] = {led_dim, led_on, led_dim};
    // Shift is allowed to be -1...5 for 7 LEDs and width-3 pattern
    constexpr int shift_max = n_leds + 1 - NELEMS(led_pattern);
    static int shift = -1;
    // Current pattern shift direction (left or right)
    static bool is_shifting_right = true;

    // Write shifted pattern to LED PWM channels
    for (int i = 0; i < n_leds; ++i) {
        int pattern_index = i - shift;
        if (pattern_index < 0|| pattern_index >= NELEMS(led_pattern)) {
            ledcWrite(i, led_off);
        } else {
            ledcWrite(i, led_pattern[pattern_index]);
        }
    }

    // Update shift value
    if (is_shifting_right) {
        if (shift < shift_max) {
            ++shift;
        } else { // Change direction
        is_shifting_right = false;
            --shift;
        }
    } else { // Current shift direction left
        if (shift > -1) {
            --shift;
        } else { // Change direction
        is_shifting_right = true;
            ++shift;
        }
    }
}

void Tannenbaum::update_spinning(bool direction) {
    constexpr int n_leds = 12;
    // Pattern is one uint8 value for setting output PWM duty cycle.
    constexpr uint16_t led_pattern[] = {led_dim, led_on, led_dim};
    // Wrap around a cycle of this many shift positions
    constexpr uint8_t wrap_length = n_leds;
    rotate_pattern(led_pattern, NELEMS(led_pattern), n_leds, wrap_length, direction);
}

void Tannenbaum::update_arrow(bool direction) {
    constexpr int n_leds = 7;
    // Pattern is one uint8 value for setting output PWM duty cycle.
    constexpr uint16_t led_pattern[] = {led_dim, led_on, led_dim};
    // Wrap around a cycle of this many shift positions
    constexpr int wrap_length = n_leds + NELEMS(led_pattern);
    rotate_pattern(led_pattern, NELEMS(led_pattern), n_leds, wrap_length, direction);
}

void Tannenbaum::update_all_on_off() {
    uint16_t pwm_value = led_state_all_on ? led_on : led_off;
    ledcWrite(0, pwm_value);
}

void Tannenbaum::rotate_pattern(const uint16_t* pattern, const uint8_t l_pattern,
                                const uint8_t n_leds, const uint8_t wrap_length,
                                const bool direction) {
    static int shift = 0;
    // Write shifted pattern to LED PWM channels
    for (int i = 0; i < n_leds; ++i) {
        int pattern_index = (i - shift + wrap_length) % wrap_length;
        if (pattern_index >= l_pattern) {
            ledcWrite(i, led_off);
        } else {
            ledcWrite(i, pattern[pattern_index]);
        }
    }
    // Update shift value
    if (direction) {
        if (shift < wrap_length - 1) {
            ++shift;
        } else {
            shift = 0;
        }
    } else { // Current shift direction left
        if (shift > 0) {
            --shift;
        } else {
            shift = wrap_length - 1;
        }
    }
}


// Static function
void Tannenbaum::on_timer_event(Tannenbaum* self) {
    // Call LED PWM pattern update
    switch (self->op_mode) {
        case LARSON: self->update_larson(); break;
        case SPIN_RIGHT: self->update_spinning(true); break;
        case SPIN_LEFT: self->update_spinning(false); break;
        case ARROW_UP: self->update_arrow(true); break;
        case ARROW_DOWN: self->update_arrow(false); break;
        case ALL_ON_OFF: self->update_all_on_off(); break;
    }
}