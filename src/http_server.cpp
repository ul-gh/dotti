/* HTTP server
 *
 * Based on ESPAsyncWebServer, see:
 * https://github.com/me-no-dev/ESPAsyncWebServer
 */
#include <Update.h>
#include <SPIFFS.h>
#include <FS.h>

// Activate incomplete features..
//#define __WORK_IN_PROGRESS__

#include "info_debug_error.h"
#include "http_server.hpp"
#include "http_server_config.hpp"
#include "http_content.hpp"

///////////// HTTPServer:: public

HTTPServer::HTTPServer()
    // public
    : backend{PORT}
    , event_source{nullptr}
    , reboot_requested{false}
    , event_timer{}
{   
    if (MOUNT_SPIFFS) {
        if (!SPIFFS.begin(true)) {
            error_print("Error mounting SPI Flash File System");
        }
    }
    event_timer.attach_ms(HEARTBEAT_INTERVAL, on_timer_event, this);
}

HTTPServer::~HTTPServer() {
    event_timer.detach();
}

// Set an entry in the template processor string <=> string mapping 
void HTTPServer::set_template(const char* placeholder, const char* replacement) {
    if (!USE_TEMPLATES) {
        error_print("ERROR: template processing must be activated!");
        return;
    };
    template_map[String(placeholder)] = String(replacement);
}

void HTTPServer::activate_events_on(const char* endpoint) {
    event_source = new AsyncEventSource(endpoint);
    register_sse_callbacks();
}

void HTTPServer::register_api_cb(const char* cmd_name,
                                 CbStringT cmd_callback) {
    cmd_map[cmd_name] = cmd_callback;
    debug_print_sv("Registered String command: ", cmd_name);
}

void HTTPServer::register_api_cb(const char* cmd_name,
                                 CbFloatT cmd_callback) {
    cmd_map[cmd_name] = [cmd_callback](const String& value) {
        // Arduino String.toFloat() defaults to zero for invalid string, hmm...
        cmd_callback(value.toFloat());
    };
    debug_print_sv("Registered float command: ", cmd_name);
}

void HTTPServer::register_api_cb(const char* cmd_name,
                                 CbIntT cmd_callback) {
    cmd_map[cmd_name] = [cmd_callback](const String& value) {
        // Arduino String.toFloat() defaults to zero for invalid string, hmm...
        cmd_callback(value.toInt());
    };
    debug_print_sv("Registered int command: ", cmd_name);
}

void HTTPServer::register_api_cb(const char* cmd_name,
                                 CbVoidT cmd_callback) {
    cmd_map[cmd_name] = [cmd_callback](const String& value) {
        cmd_callback();
    };
    debug_print_sv("Registered void command:", cmd_name);
}

void HTTPServer::begin() {
    register_default_callbacks();
    backend.begin();
}


///////// HTTPServer:: private

// Timer update for heartbeats, reboot etc
// Static function wraps member function to obtain C API callback
void HTTPServer::on_timer_event(HTTPServer* self) {
    if (SEND_HEARTBEATS && self->event_source != nullptr) {
        self->event_source->send("OK", "heartbeat");
    }
    if (self->reboot_requested) {
        debug_print("Rebooting...");
        delay(100);
        ESP.restart();
    }
}

// Sever-Sent Event Source
void HTTPServer::register_sse_callbacks() {
    event_source->onConnect([](AsyncEventSourceClient *client) {
        if(client->lastId()){
            info_print_sv("Client connected! Last msg ID:", client->lastId());
        }
        //send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("Hello Message from ESP32!", NULL, millis(), 1000);
    });
    // HTTP Basic Authentication
    //if (USE_AUTH) {
    //    event_source.setAuthentication(http_user, http_pass);
    //}
    backend.addHandler(event_source);
}

// Normal HTTP request handlers
void HTTPServer::register_default_callbacks() {
    // Serve static HTML and related files content
    if (MOUNT_SPIFFS) {
        auto handler = backend.serveStatic("/", SPIFFS, "/www/")
                              .setDefaultFile(INDEX_HTML_FILENAME);
        if (USE_TEMPLATES) {
            handler = handler.setTemplateProcessor(
                [this](const String &placeholder) {
                    return templateProcessor(placeholder);
                }
            );
        }
        if (USE_AUTH) {
            handler.setAuthentication(HTTP_USER, HTTP_PASS);
        }
    } else {
        // Route for main application home page
        backend.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
                onRootRequest(request);
            }
        );
    }
    // Route for REST API
    //using namespace std::placeholders;
    //on("/cmd", HTTP_GET, std::bind(&HTTPServer::onCmdRequest, this, _1));
    backend.on(API_ENDPOINT, HTTP_GET, [this](AsyncWebServerRequest *request) {
            onCmdRequest(request);
        }
    );
    // respond to GET requests on URL /heap
    backend.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", String(ESP.getFreeHeap()));
       }
    );
    // OTA Firmware Upgrade, see form method in data/www/upload.html
    backend.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
            onUpdateRequest(request);
       },
       onUpdateUploadBody
    );
    backend.onNotFound(onRequest);
    backend.onFileUpload(onUpload);
    backend.onRequestBody(onBody);
    // Handler called when any DNS query is made via access point
    // addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    debug_print("Default callbacks set up");
}


// Template processor
String HTTPServer::templateProcessor(const String& placeholder)
{
    auto template_iterator = template_map.find(placeholder);
        if (template_iterator == template_map.end()) {
            error_print_sv("Error: Entry not registered in template mapping:",
                           placeholder);
            return placeholder;
        } else {
            return template_map[placeholder];
        }
}

// on("/")
void HTTPServer::onRootRequest(AsyncWebServerRequest *request) {
    if (!MOUNT_SPIFFS) {
        // Static content is handled by default handler for static content
        if (USE_TEMPLATES) {
            request->send_P(200, "text/html", INDEX_HTML,
                            [this](const String& placeholder) {
                                return templateProcessor(placeholder);
                            }
            );
        } else {
            request->send_P(200, "text/html", INDEX_HTML);
        }
    }
}

// on("/cmd")
void HTTPServer::onCmdRequest(AsyncWebServerRequest *request) {
    int n_params = request->params();
    debug_print_sv("Number of parameters received:", n_params);
    for (int i = 0; i < n_params; ++i) {
        AsyncWebParameter *p = request->getParam(i);
        const String name = p->name();
        const String value_str = p->value();
        debug_print_sv("-----\nParam name:", name);
        debug_print_sv("Param value:", value_str);
        auto cb_iterator = cmd_map.find(name);
        if (cb_iterator == cmd_map.end()) {
            error_print_sv("Error: Not registered in command mapping:", name);
            continue;
        }
        CbStringT cmd_callback = (*cb_iterator).second; // = cmd_map[name];
        if (!cmd_callback) {
            error_print_sv("Error: Not a callable object!", name);
            continue;
        }
        // Finally call callback
        cmd_callback(value_str);
    }
    if (API_IS_AJAX) {
        // For AJAX interface: Return a plain string, default is empty string.
        request->send(200, "text/plain", AJAX_RETURN_TEXT);
    } else if (!MOUNT_SPIFFS) {
        // Static content is handled by default handler for static content
        if (USE_TEMPLATES) {
            request->send_P(200, "text/html", API_RETURN_HTML,
                            [this](const String& placeholder) {
                                return templateProcessor(placeholder);
                            }
            );
        } else {
            request->send_P(200, "text/html", API_RETURN_HTML);
        }
    }
}

// on("/update")
// When update is initiated via GET
void HTTPServer::onUpdateRequest(AsyncWebServerRequest *request) {
    reboot_requested = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(
        200, "text/plain", reboot_requested ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
}
// When file is uploaded via POST request
void HTTPServer::onUpdateUploadBody(
        AsyncWebServerRequest *request, const String& filename,
        size_t index, uint8_t *data, size_t len, bool final) {
    if(!index) {
        info_print_sv("Update Start:", filename);
        //Update.runAsync(true);
        if(!Update.begin((ESP.getFreeSketchSpace()-0x1000) & 0xFFFFF000)) {
            Update.printError(Serial);
        }
    }
    if(!Update.hasError()) {
        if(Update.write(data, len) != len){
            Update.printError(Serial);
        }
    }
    if(final) {
        if(Update.end(true)) {
            info_print_sv("Update Success:", index+len);
        }
        else {
            Update.printError(Serial);
        }
    }
} // on("/update")

/* Catch-All-Handlers
 */
void HTTPServer::onRequest(AsyncWebServerRequest *request) {
    //Handle Unknown Request
    request->send(404);
}

void HTTPServer::onBody(AsyncWebServerRequest *request,
        uint8_t *data, size_t len, size_t index, size_t total) {
    //Handle body
}

void HTTPServer::onUpload(AsyncWebServerRequest *request, const String& filename,
        size_t index, uint8_t *data, size_t len, bool final) {
    //Handle upload
}

#ifdef __WORK_IN_PROGRESS__
/* Handler for captive portal page, only active when in access point mode
*/
CaptiveRequestHandler::CaptiveRequestHandler() {}
// virtual CaptiveRequestHandler::~CaptiveRequestHandler() {}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request) {
    //request->addInterestingHeader("ANY");
    return true;
}

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf(
            "<!DOCTYPE html><html><head><title>Captive Portal</title></head>"
            "<body><p>Captive portal front page.</p>"
            "<p>You were trying to reach: http://%s%s</p>"
            "<p>Opening <a href='http://%s'>this link</a> instead</p>"
            "</body></html>",
            request->host(), request->url(),
            WiFi.softAPIP().toString()
    );
    request->send(response);
}
#endif