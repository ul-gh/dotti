#include <functional>
#include <Arduino.h>
#include <FreeRTOS.h>
#include <Ticker.h>
#include "esp32-hal-log.h"

//#include "info_debug_error.h"
//include "wifi_setup.hpp"
#include "http_server.hpp"
#include "ESPAsyncWiFiManager.h"
#include "tannenbaum.hpp"

constexpr unsigned long serial_baudrate = 115200;
// TCP socket port number
constexpr uint16_t tcp_port = 80;

// ESPAsyncWebserver must be one single instance
AsyncWebServer http_backend{tcp_port};
// DNS resolution for captive portal page etc.
DNSServer* dns_server;
// Wifi connection manager
AsyncWiFiManager* wifi_manager;

// HTTP server provides REST API + HTML5 AJAX web interface on port 80
APIServer* api_server;
// Der Tannenbaum
Tannenbaum* tannenbaum;

void setup() {
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    Serial.begin(serial_baudrate);
    //setup_wifi_station();
    //setup_wifi_hostap();
    //delay(300);
    dns_server = new DNSServer{};
    wifi_manager = new AsyncWiFiManager{&http_backend, dns_server};
    //wifi_manager->resetSettings();
    //wifi_manager->startConfigPortal("Tannenbaum_Access_Point");
    wifi_manager->autoConnect("Tannenbaum_Access_Point");
    wifi_manager->setConfigPortalTimeout(180);
    api_server = new APIServer{&http_backend};
    tannenbaum = new Tannenbaum{*api_server, Tannenbaum::LARSON};
    api_server->activate_events_on("/events");
    api_server->activate_default_callbacks();
}

void loop() {
    //dns_server->processNextRequest();
}
