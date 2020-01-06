#ifndef __TANNENBAUM_HPP__
#define __TANNENBAUM_HPP__

#include "http_server.hpp"

// LED PWM frequency
#define PWM_FREQ 1000
#define SENSOR_INTERVAL 1000

#define TOUCH_RIGHT_PIN 15
#define TOUCH_MIDDLE_PIN 12
#define TOUCH_LEFT_PIN 13

// Operation modes for the application
enum OP_MODES{LARSON, SPINNING, ALL_ON_OFF};

class Tannenbaum
{
public:
    Tannenbaum(HTTPServer& http_server, enum OP_MODES op_mode);
    ~Tannenbaum();

    void set_mode_larson();
    void set_mode_spinning();
    void set_mode_all_on_off();

    bool toggle_on_off_state();

    void increase_speed();
    void decrease_speed();

private:
    HTTPServer& http_server;

    // Async event timers
    Ticker pattern_timer;
    Ticker sensor_timer;

    // Operation mode
    enum OP_MODES op_mode;
    // LED state for static on/off; true => ON
    bool led_state_all_on;
    // Time interval in ms for updating LED pattern state
    unsigned long pattern_interval;

    void setup_http_interface();
    static void read_touch_interface();
    void init_pwm_gpios();

    void update_larson();
    void update_spinning();
    void update_all_on_off();

    static void on_timer_event(Tannenbaum* self);

}; // class Tannenbaum

#endif