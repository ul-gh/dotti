#ifndef __TANNENBAUM_HPP__
#define __TANNENBAUM_HPP__

#include "http_server.hpp"
#include "touch_buttons.hpp"

class Tannenbaum
{
public:
    // LED PWM frequency
    static constexpr double PWM_FREQ = 1000;
    static constexpr uint8_t TOUCH_THRESHOLD_PERCENT = 94;

    static constexpr int TOUCH_IO_RIGHT = 3; // GPIO 15
    static constexpr int TOUCH_IO_MIDDLE = 5; // GPIO 12
    static constexpr int TOUCH_IO_LEFT = 4; // GPIO 13

    // Operation modes for the application
    enum OP_MODES{LARSON, SPINNING, ALL_ON_OFF};

    Tannenbaum(HTTPServer& http_server, enum OP_MODES op_mode);
    ~Tannenbaum();

    void set_mode_larson();
    void set_mode_spinning();
    void set_mode_all_on_off();

    void set_next_mode();

    bool toggle_on_off_state();

    void increase_speed();
    void decrease_speed();

private:
    // HTTP API server
    HTTPServer& http_server;

    // Touch button interface
    ReactiveTouch buttons;

    // Async event timers
    Ticker pattern_timer;

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
    void update_spinning();
    void update_all_on_off();

    static void on_timer_event(Tannenbaum* self);

}; // class Tannenbaum

#endif