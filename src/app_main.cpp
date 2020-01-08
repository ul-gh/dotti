#include <functional>
#include <Arduino.h>
#include <Ticker.h>
#include "esp32-hal-log.h"

//#include "info_debug_error.h"
#include "wifi_setup.hpp"
#include "http_server.hpp"
#include "tannenbaum.hpp"

#define SERIAL_BAUD 115200

// HTTP server provides REST API + HTML5 AJAX web interface on port 80
HTTPServer* http_server;

// Tannenbaum instance
Tannenbaum* tannenbaum;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(100);
    http_server = new HTTPServer{};
    tannenbaum = new Tannenbaum{*http_server, Tannenbaum::LARSON};
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    setup_wifi_station();
}

void loop() {
    static bool wifi_initialised = false;
    if (!wifi_initialised && wifi_handle_state_change() == NEW_CONNECTION) {
        http_server->activate_events_on("/events");
        http_server->begin();
        wifi_initialised = true;
    }
}

// template <typename ObjectT, typename MemberPtr>
// class Timer2 {
// public:
//     Timer2(const uint32_t time_ms, ObjectT* obj, MemberPtr member)
//     {
//         obj = obj;
//         member = member;
//         ticker.attach_ms(time_ms, call_wrapper);
//     }
//     virtual ~Timer2() {
//         ticker.detach();
//     }
//     void detach() {
//         ticker.detach();
//     }
// private:
//     Ticker ticker{};
//     static ObjectT* obj;
//     static MemberPtr member;
//     static void call_wrapper() {
//         (obj->*member)();
//     }
// };
