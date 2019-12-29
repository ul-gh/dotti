#include <Arduino.h>
#include "esp32-hal-log.h"

//#include "info_debug_error.h"
#include "wifi_setup.hpp"
#include "http_server.hpp"
#include "tannenbaum.hpp"

#define SERIAL_BAUD 115200


// HTTP server provides REST API + HTML5 AJAX web interface on port 80
HTTPServer http_server{};
// Tannenbaum instance
Tannenbaum tannenbaum{http_server, LARSON};

void setup() {
    Serial.begin(SERIAL_BAUD);
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    setup_wifi_station();
}

void loop() {
    static bool wifi_initialised = false;
    if (!wifi_initialised && wifi_handle_state_change() == NEW_CONNECTION) {
        http_server.activate_events_on("/events");
        http_server.begin();
        wifi_initialised = true;
    }

    unsigned long curr_time = millis();
    // Pattern update task
    tannenbaum.update_timer(curr_time);
    // Send heartbeat every sec
    http_server.update_timer(curr_time);
}