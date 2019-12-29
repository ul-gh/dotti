#ifndef __TANNENBAUM_HPP__
#define __TANNENBAUM_HPP__

#include "http_server.hpp"

// LED PWM frequency
#define PWM_FREQ 1000

// Operation modes for the application
enum OP_MODES{LARSON, SPINNING, ALL_ON_OFF};

class Tannenbaum
{
public:
    Tannenbaum(HTTPServer& http_server, enum OP_MODES op_mode);

    void set_mode_larson();
    void set_mode_spinning();
    void set_mode_all_on_off();

    bool toggle_on_off_state();

    void increase_speed();
    void decrease_speed();

    void update_timer(const unsigned long curr_time);


private:
    HTTPServer& http_server;

    // Operation mode
    enum OP_MODES op_mode;
    // LED state for static on/off; true => ON
    bool led_state_all_on;
    // Time interval in ms for updating LED pattern state
    unsigned long pattern_interval;

    void setup_http_interface();

    void init_pwm_gpios();

    void update_larson();
    void update_spinning();
    void update_all_on_off();

}; // class Tannenbaum

#endif