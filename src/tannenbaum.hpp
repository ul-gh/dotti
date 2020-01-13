#ifndef TANNENBAUM_HPP
#define TANNENBAUM_HPP

#include "http_server.hpp"
#include "touch_buttons.hpp"

class Tannenbaum
{
public:
    // LED PWM frequency
    static constexpr double PWM_FREQ = 1000;
    // Pre-Defined LED brightness levels.
    // Pins are inverted - 255 => 0%; 0 => 100%
    static constexpr uint8_t led_on = 0;
    static constexpr uint8_t led_dim = 220;
    static constexpr uint8_t led_off = 255;

    // Touch button touch detection threshold
    static constexpr uint8_t TOUCH_THRESHOLD_PERCENT = 94;
    // Touch button GPIO pins
    static constexpr int TOUCH_IO_RIGHT = 3; // GPIO 15
    static constexpr int TOUCH_IO_MIDDLE = 5; // GPIO 12
    static constexpr int TOUCH_IO_LEFT = 4; // GPIO 13

    // Operation modes for the application
    enum OP_MODES{LARSON, SPIN_RIGHT, SPIN_LEFT, ARROW_UP, ARROW_DOWN, ALL_ON_OFF};

    Tannenbaum(HTTPServer& http_server, enum OP_MODES op_mode);
    ~Tannenbaum();

    void set_mode_larson();
    void set_mode_spinning(bool direction);
    void set_mode_arrow(bool direction);
    void set_mode_all_on_off();

    void set_next_mode();

    bool toggle_on_off_state();

    void increase_speed();
    void decrease_speed();

    void play(note_t note, uint32_t duration, uint8_t octave=4);
    static void play_stop();

private:
    // HTTP API server
    HTTPServer& http_server;

    // Touch button interface
    ReactiveTouch buttons;

    // Async event timers
    Ticker pattern_timer;
    Ticker tone_timer;

    // Operation mode
    enum OP_MODES op_mode;
    // LED state for static on/off; true => ON
    bool led_state_all_on;
    // Time interval in ms for updating LED pattern state
    unsigned long pattern_interval;

    void setup_http_interface();
    void setup_touch_buttons();
    void init_pwm_gpios();

    void update_larson();
    void update_spinning(bool direction);
    void update_arrow(bool direction);
    void update_all_on_off();

    void rotate_pattern(const uint8_t* pattern, const uint8_t l_pattern,
                        const uint8_t n_leds, const uint8_t wrap_length,
                        const bool direction);

    static void on_timer_event(Tannenbaum* self);

}; // class Tannenbaum

#endif