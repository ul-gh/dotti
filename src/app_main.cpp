#include <functional>
#include <Arduino.h>
#include <Ticker.h>
#include "esp32-hal-log.h"

//#include "info_debug_error.h"
#include "wifi_setup.hpp"
#include "http_server.hpp"
#include "tannenbaum.hpp"

constexpr unsigned long serial_baudrate = 115200;

// HTTP server provides REST API + HTML5 AJAX web interface on port 80
HTTPServer* http_server;

Tannenbaum* tannenbaum;

void setup() {
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    Serial.begin(serial_baudrate);
    //setup_wifi_station();
    setup_wifi_hostap();
    delay(300);
    http_server = new HTTPServer{};
    tannenbaum = new Tannenbaum{*http_server, Tannenbaum::LARSON};
    http_server->activate_events_on("/events");
    http_server->begin();
}

void loop() {
    //static bool wifi_initialised = false;
    //if (!wifi_initialised && wifi_handle_state_change() == NEW_CONNECTION) {
    //    http_server->activate_events_on("/events");
    //    http_server->begin();
    //    wifi_initialised = true;
    //}
}
