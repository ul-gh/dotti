#include <Arduino.h>

// Display the HTML web page
const char INDEX_HTML[] =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<link rel=\"icon\" href=\"data:,\">"
    // CSS to style the on/off buttons
    "<style>"
        "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
        "button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;"
        "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
        ".btn_off {background-color: #555555;}"
    "</style>"
    "</head>"
    
    // Web Page Heading
    "<body><h1>Karlottas Tannenbaum!</h1>"
    "<p>Teilweise selbst gebastelt</p>"
    "<p><a href=\"/cmd?larson\"><button>Glen A. Larson</button></a></p>"
    "<p><a href=\"/cmd?spin\"><button>DURCHDREHEN!!!</button></a></p>"

    "<p><a href=\"/cmd?on_off\">"
    "<button class=\"%ON_OFF_BTN_STATE%\">ON/OFF</button>"
    "</a></p>"

    "<p><a href=\"/cmd?plus\"><button>SCHNELLER</button></a></p>"
    "<p><a href=\"/cmd?minus\"><button>LANGSAMER</button></a></p>"
    "</body>"
    "</html>"
    "\n";

// Return HTML for API homepage
//const char* const API_RETURN_HTML = INDEX_HTML;
//const char (&API_RETURN_HTML)[] = INDEX_HTML;
#define API_RETURN_HTML INDEX_HTML