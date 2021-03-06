// When set to yes, mount SPIFFS filesystem and serve static content
// from files contained in data/www at the "/" endpoint.
constexpr bool mount_spiffs_requested = false;
// Default filename served from SPIFFS when "/" without filename is requested
constexpr const char* index_html_filename = "index.html";

// Activate template processing when defined
constexpr bool template_processing_activated = true;

// Common API endpoint for AJAX GET requests registered via regster_ajax_cb()
constexpr const char* api_endpoint = "/cmd";
// For AJAX, reply with an plain string, default is empty string.
// When not using AJAX, reply with content from string API_HTML as
// defined in separate header http_content.hpp
constexpr bool api_is_ajax = false;
constexpr const char* ajax_return_text = "";

// Send heartbeat message via SSE event source in regular intervals when
// set to true. This needs regular calling of update_timer().
constexpr bool sending_heartbeats = true;
// Interval time in milliseconds
constexpr unsigned long heartbeat_interval = 1000;

// Activate HTTP Basic Authentication, set to true when user/password is given
constexpr bool http_auth_requested = false;
// HTTP Basic Authentication username and password
constexpr const char* http_user = "";
constexpr const char* http_pass = "";
