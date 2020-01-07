//#include "esp_log.h"
#include "esp32-hal-log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/touch_pad.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

#include "info_debug_error.h"
#include "touch_buttons.hpp"

static constexpr int DISPATCH_CYCLE_TIME_MS = 100;
static constexpr int THRESHOLD_INACTIVE = 0;
static constexpr int FILTER_PERIOD = 10;

//////// ReactiveTouch public:

ReactiveTouch::ReactiveTouch()
    : event_timer{}
{   
    // Initialize touch pad peripheral, it will start a timer to run a filter
    info_print("Initializing touch pad");
    touch_pad_init();
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Set reference voltage for charging/discharging
    // For most usage scenarios, we recommend using the following combination:
    // the high reference valtage will be 2.7V - 1V = 1.7V, The low reference voltage will be 0.5V.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    //init RTC IO and mode for touch pad.
    for (int i=0; i<TOUCH_PAD_MAX; ++i) {
        s_pad_activated[i] = false;
        s_pad_threshold[i] = THRESHOLD_INACTIVE;
        // Inizialization using the default constructor of std::function
        s_pad_callback[i] = {};
    }
}

ReactiveTouch::~ReactiveTouch() {
    event_timer.detach();
}

void ReactiveTouch::configure_input(const int input_number,
                                    const uint8_t threshold_percent,
                                    CallbackT callback) {
    s_pad_threshold_percent[input_number] = threshold_percent;
    s_pad_callback[input_number] = callback;
}

void ReactiveTouch::calibrate_thresholds() {
    uint16_t touch_value;
    for (int i=0; i<TOUCH_PAD_MAX; ++i) {
        if (s_pad_activated[i]) {
            //read filtered value
            touch_pad_read_filtered(static_cast<touch_pad_t>(i), &touch_value);
            debug_print_sv("Current touch input: ", i);
            debug_print_sv("touch pad val is: ", touch_value);
            s_pad_threshold[i] = static_cast<uint32_t>(touch_value)
                    * s_pad_threshold_percent[i] / 100;
            debug_print_sv("threshold value is: ", s_pad_threshold[i]);
        }
    }
}

void ReactiveTouch::begin() {
    for (int i=0; i<TOUCH_PAD_MAX; ++i) {
        if (s_pad_activated[i]) {
            touch_pad_config(static_cast<touch_pad_t>(i), THRESHOLD_INACTIVE);
        }
    }
    // Initialize and start a software filter to detect slight change of capacitance.
    touch_pad_filter_start(FILTER_PERIOD);
    // Set thresh hold
    calibrate_thresholds();

    event_timer.attach_ms(DISPATCH_CYCLE_TIME_MS, dispatch_callbacks, this);
}

void ReactiveTouch::diagnostics() {
    static int show_message;
    static int change_mode = 0;
    static int filter_mode = 0;
    if (filter_mode == 0) {
        //interrupt mode, enable touch interrupt
        touch_pad_intr_enable();
        for (int i=0; i<TOUCH_PAD_MAX; ++i) {
            if (s_pad_activated[i] == true) {
                info_print_sv("Pad activated!", i);
                // Wait a while for the pad being released
                vTaskDelay(200 / portTICK_PERIOD_MS);
                // Clear information on pad activation
                s_pad_activated[i] = false;
                // Reset the counter triggering a message
                // that application is running
                show_message = 1;
            }
        }
    } else {
        //filter mode, disable touch interrupt
        touch_pad_intr_disable();
        touch_pad_clear_status();
        for (int i=0; i<TOUCH_PAD_MAX; ++i) {
            if (s_pad_activated[i]) {
                uint16_t value;
                touch_pad_read_filtered(static_cast<touch_pad_t>(i), &value);
                if (value < s_pad_threshold[i]) {
                    info_print_sv("T%d activated!", i);
                    info_print_sv("value: ", value);
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                    // Reset the counter to stop changing mode.
                    change_mode = 1;
                    show_message = 1;
                }
            }
        }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

    // If no pad is touched, every couple of seconds, show a message
    // that application is running
    if (show_message++ % 300 == 0) {
        uint16_t value;
        for (int i=0; i<TOUCH_PAD_MAX; ++i) {
            if (s_pad_activated[i]) {
                touch_pad_read_filtered(static_cast<touch_pad_t>(i), &value);
                ESP_LOGI(TAG, "T%d value: %d", i, value);
            }
        }
        ESP_LOGI(TAG, "Waiting for any pad being touched...");
    }
    // Change mode if no pad is touched for a long time.
    // We can compare the two different mode.
    if (change_mode++ % 1000 == 0) {
        filter_mode = !filter_mode;
        uint16_t value;
        for (int i=0; i<TOUCH_PAD_MAX; ++i) {
            if (s_pad_activated[i]) {
                touch_pad_read_filtered(static_cast<touch_pad_t>(i), &value);
                ESP_LOGI(TAG, "T%d value: %d", i, value);
            }
        }
        ESP_LOGW(TAG, "Change mode...%s", filter_mode == 0? "interrupt mode": "filter mode");
    }
} // void touch_buttons_diagnostics()

//////// ReactiveTouch private:

uint8_t ReactiveTouch::s_pad_threshold_percent[TOUCH_PAD_MAX];
bool ReactiveTouch::s_pad_activated[TOUCH_PAD_MAX];
uint16_t ReactiveTouch::s_pad_filtered_value[TOUCH_PAD_MAX];
uint16_t ReactiveTouch::s_pad_threshold[TOUCH_PAD_MAX];
CallbackT ReactiveTouch::s_pad_callback[TOUCH_PAD_MAX];

void ReactiveTouch::filter_read_cb(uint16_t *raw_value, uint16_t *filtered_value) {
    for (int i=0; i<TOUCH_PAD_MAX; ++i) {
        s_pad_filtered_value[i] = *(filtered_value + i);
    }
}

void ReactiveTouch::dispatch_callbacks(ReactiveTouch* self) {
    for (int i=0; i<TOUCH_PAD_MAX; ++i) {
        if (s_pad_activated[i] && s_pad_filtered_value[i] < s_pad_threshold[i]) {
            CallbackT cb = s_pad_callback[i];
            if (cb) {
                cb();
            }
        }
    }
}
