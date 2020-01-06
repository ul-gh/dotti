#include <functional>
#include <Arduino.h>
#include <Ticker.h>
#include "esp32-hal-log.h"

//#include "info_debug_error.h"
#include "wifi_setup.hpp"
#include "http_server.hpp"
#include "tannenbaum.hpp"
//#include "touch_buttons.hpp"

#define SERIAL_BAUD 115200

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


// HTTP server provides REST API + HTML5 AJAX web interface on port 80
HTTPServer http_server{};
//using ObjT = HTTPServer;
//using MemberPtr = void (HTTPServer::*)();


// Tannenbaum instance
Tannenbaum tannenbaum{http_server, LARSON};
//auto lambda = [](){Serial.println("Fooo_STARTER__!");};


void setup() {
    Serial.begin(SERIAL_BAUD);
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    setup_wifi_station();
    //touch_buttons_init();
}

void loop() {
    static bool wifi_initialised = false;
    if (!wifi_initialised && wifi_handle_state_change() == NEW_CONNECTION) {
        http_server.activate_events_on("/events");
        http_server.begin();
        wifi_initialised = true;
    }

    //touch_buttons_dispatch();
}