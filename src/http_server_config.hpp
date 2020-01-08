// TCP socket port number
constexpr uint16_t PORT = 80;

// When set to yes, mount SPIFFS filesystem and serve static content
// from files contained in data/www at the "/" endpoint.
constexpr bool MOUNT_SPIFFS = false;
// Default filename served from SPIFFS when "/" without filename is requested
constexpr const char* INDEX_HTML_FILENAME = "index.html";

// Activate template processing when defined
constexpr bool USE_TEMPLATES = true;

// Common API endpoint for AJAX GET requests registered via regster_ajax_cb()
constexpr const char* API_ENDPOINT = "/cmd";
// For AJAX, reply with an plain string, default is empty string.
// When not using AJAX, reply with content from string API_HTML as
// defined in separate header http_content.hpp
constexpr bool API_IS_AJAX = false;
constexpr const char* AJAX_RETURN_TEXT = "";

// Send heartbeat message via SSE event source in regular intervals when
// set to true. This needs regular calling of update_timer().
constexpr bool SEND_HEARTBEATS = true;
// Interval time in milliseconds
constexpr unsigned long HEARTBEAT_INTERVAL = 1000;

// Activate HTTP Basic Authentication, set to true when user/password is given
constexpr bool USE_AUTH = false;
// HTTP Basic Authentication username and password
constexpr const char* HTTP_USER = "";
constexpr const char* HTTP_PASS = "";