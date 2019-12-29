#include <Arduino.h>

#include "info_debug_error.h"
#include "tannenbaum.hpp"
#include "http_server.hpp"

/////////// public

Tannenbaum::Tannenbaum(HTTPServer& http_server, enum OP_MODES op_mode)
    // private
    : http_server{http_server}
    , op_mode{LARSON}
    , led_state_all_on{false}
    , pattern_interval{100}
{
    debug_print("Configuring Tannenbaum...");
    init_pwm_gpios();
    if (op_mode == LARSON) {
        set_mode_larson();
    } else if (op_mode == SPINNING) {
        set_mode_spinning();
    } else if (op_mode == ALL_ON_OFF) {
        set_mode_all_on_off();
        update_all_on_off();
    }
    // Remote control interface
    setup_http_interface();
}

void Tannenbaum::set_mode_larson() {
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

void Tannenbaum::set_mode_spinning() {
    op_mode = SPINNING;
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

void Tannenbaum::set_mode_all_on_off() {
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
    return led_state_all_on;
}

void Tannenbaum::increase_speed() {
    debug_print("Faster.");
    if (pattern_interval >= 2) {
        pattern_interval /= 2;
    }
}

void Tannenbaum::decrease_speed() {
    debug_print("Slower..");
    if (pattern_interval < 4096) {
        pattern_interval *= 2;
    }
}

void Tannenbaum::update_timer(const unsigned long curr_time) {        
    static unsigned long pattern_timestamp = millis();
    if (curr_time - pattern_timestamp > pattern_interval) {
        pattern_timestamp = curr_time;
        // Call LED PWM pattern update
        if (op_mode == LARSON) {
            update_larson();
        }  else if (op_mode == SPINNING) {
            update_spinning();
        } else if (op_mode == ALL_ON_OFF) {
            update_all_on_off();
        }
    }
}

///////////// private

void Tannenbaum::setup_http_interface() {
    http_server.register_api_cb("larson", [this](){set_mode_larson();});
    http_server.register_api_cb("spin", [this](){set_mode_spinning();});
    http_server.register_api_cb("on_off", [this](){
        if(toggle_on_off_state()) {
            http_server.set_template("ON_OFF_BTN_STATE", "");
        } else {
            http_server.set_template("ON_OFF_BTN_STATE", "btn_off");
        }
    });
    http_server.register_api_cb("plus", [this](){increase_speed();});
    http_server.register_api_cb("minus", [this](){decrease_speed();});
    
    // Initial state
    http_server.set_template("ON_OFF_BTN_STATE", "btn_off");
}

void Tannenbaum::init_pwm_gpios() {
    // Setup PWM channels
    ledcSetup(0, PWM_FREQ, 8);
    ledcSetup(1, PWM_FREQ, 8);
    ledcSetup(2, PWM_FREQ, 8);
    ledcSetup(3, PWM_FREQ, 8);
    ledcSetup(4, PWM_FREQ, 8);
    ledcSetup(5, PWM_FREQ, 8);
    ledcSetup(6, PWM_FREQ, 8);
    ledcSetup(7, PWM_FREQ, 8);
    ledcSetup(8, PWM_FREQ, 8);
    ledcSetup(9, PWM_FREQ, 8);
    ledcSetup(10, PWM_FREQ, 8);
    ledcSetup(11, PWM_FREQ, 8);
    ledcSetup(12, PWM_FREQ, 8);
}

void Tannenbaum::update_larson() {
    const int N_LEDS = 7;
    // Pins are inverted - 255 => 0%; 0 => 100%
    const uint8_t ON = 0;
    const uint8_t DIM = 220;
    const uint8_t OFF = 255;
    // Pattern is one uint8 value for setting output PWM duty cycle.
    const uint8_t LED_PATTERN[] = {DIM, ON, DIM};
    // Shift is allowed to be -1...5 for 7 LEDs and width-3 pattern
    const int SHIFT_MAX = N_LEDS + 1 - sizeof(LED_PATTERN);
    static int shift = -1;
    // Current pattern shift direction (left or right)
    static bool is_shifting_right = true;

    // Write shifted pattern to LED PWM channels
    for (int i = 0; i < N_LEDS; ++i) {
        int pattern_index = i - shift;
        if (pattern_index < 0 || pattern_index >= sizeof(LED_PATTERN)) {
        ledcWrite(i, OFF);
        } else {
        ledcWrite(i, LED_PATTERN[pattern_index]);
        }
    }

    // Update shift value
    if (is_shifting_right) {
        if (shift < SHIFT_MAX) {
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

void Tannenbaum::update_spinning() {
    const int N_LEDS = 12;
    // Pins are inverted - 255 => 0%; 0 => 100%
    const uint8_t ON = 0;
    const uint8_t DIM = 225;
    const uint8_t OFF = 255;
    // Pattern is one uint8 value for setting output PWM duty cycle.
    const uint8_t LED_PATTERN[] = {DIM, ON, DIM};
    const int SHIFT_MAX = N_LEDS;
    static int shift = 0;

    // Write shifted pattern to LED PWM channels
    for (int i = 0; i < N_LEDS; ++i) {
        int pattern_index = i - shift;
        if (pattern_index < 0) {
        pattern_index += N_LEDS;
        }
        if (pattern_index >= sizeof(LED_PATTERN)) {
        ledcWrite(i, OFF);
        } else {
        ledcWrite(i, LED_PATTERN[pattern_index]);
        }
    }

    // Update shift value
    if (shift < SHIFT_MAX) {
        ++shift;
    } else {
        shift = 1;
    }
}

void Tannenbaum::update_all_on_off() {
    // Pins are inverted - 255 => 0%; 0 => 100%
    const uint8_t ON = 0;
    const uint8_t OFF = 255;
    uint8_t pwm_value = led_state_all_on ? ON : OFF;
    ledcWrite(0, pwm_value);
}
