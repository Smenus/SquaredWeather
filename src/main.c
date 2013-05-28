#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "http.h"

#include "config.h"
#include "time_layer.h"
#include "weather_layer.h"

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }
PBL_APP_INFO(MY_UUID,
             "Squared Weather", "Smenus",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

// POST variables
#define WEATHER_KEY_LATITUDE 1
#define WEATHER_KEY_LONGITUDE 2
#define WEATHER_KEY_UNIT_SYSTEM 3
// Received variables
#define WEATHER_KEY_ICON 1
#define WEATHER_KEY_TEMPERATURE 2
#define WEATHER_KEY_IS_C 3

#define WEATHER_HTTP_COOKIE  1290352054
#define WEATHER_TIMER_COOKIE 1138158163
#define SPLASH_TIMER_COOKIE  1166677173

//#define WEATHER_API_ENDPOINT "http://pwdb.kathar.in/pebble/weather2.php"
#define WEATHER_API_ENDPOINT "http://91.121.105.51/~smenusco/weather.php"

#define STARTDELAY 2000
#define WEATHER_FETCH_FREQUENCY 1800000

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

#define TIME_WIDTH (FONT_WIDTH  + SPACING_X + FONT_WIDTH)
#define TIME_HEIGHT (FONT_HEIGHT + SPACING_Y + FONT_HEIGHT)

#define WEATHER_HEIGHT 20

#define TOTAL_HEIGHT (TIME_HEIGHT + SPACING_Y + WEATHER_HEIGHT)

#define ORIGIN_X ((SCREEN_WIDTH - TIME_WIDTH)/2)

#define TIME_ORIGIN_Y ((SCREEN_HEIGHT - TOTAL_HEIGHT)/2)
#define WEATHER_ORIGIN_Y (TIME_ORIGIN_Y + TIME_HEIGHT + SPACING_Y)


static Window window;
static TimeLayer time_layer;
static WeatherLayer weather_layer;

static int our_latitude, our_longitude;
static bool located = false;

static bool splashEnded = false;

static AppTimerHandle weather_timer = 0;


void request_weather() {
    if(!located) {
        http_location_request();
        return;
    }

    // Build the HTTP request
    DictionaryIterator* body;
    HTTPResult result = http_out_get(WEATHER_API_ENDPOINT, WEATHER_HTTP_COOKIE, &body);

    if(result != HTTP_OK) {
        weather_layer_clear_icon(&weather_layer);
        weather_layer_clear_temp(&weather_layer);
        return;
    }

    dict_write_int32(body, WEATHER_KEY_LATITUDE, our_latitude);
    dict_write_int32(body, WEATHER_KEY_LONGITUDE, our_longitude);
    dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM);

    // Send it.
    if(http_out_send() != HTTP_OK) {
        weather_layer_clear_icon(&weather_layer);
        weather_layer_clear_temp(&weather_layer);
        return;
    }
}

void failed(int32_t cookie, int http_status, void* ctx) {
    if(cookie == 0 || cookie == WEATHER_HTTP_COOKIE) {
        weather_layer_clear_icon(&weather_layer);
        weather_layer_clear_temp(&weather_layer);
    }

    located = false;
    http_location_request();
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* ctx) {
    if(cookie != WEATHER_HTTP_COOKIE) {
        return;
    }

    Tuple* icon_tuple = dict_find(received, WEATHER_KEY_ICON);

    if(icon_tuple) {
        int icon = icon_tuple->value->int8;

        if(icon >= 0 && icon < 10) {
            weather_layer_set_icon(&weather_layer, icon);
        } else {
            weather_layer_clear_icon(&weather_layer);
        }
    }

    Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE);

    if(temperature_tuple) {
        Tuple* is_c_tuple = dict_find(received, WEATHER_KEY_IS_C);

        if(is_c_tuple) {
            weather_layer_set_temp(&weather_layer, temperature_tuple->value->int16, is_c_tuple->value->int8);
        } else {
            weather_layer_set_temp(&weather_layer, temperature_tuple->value->int16, -1);
        }
    }
}

void reconnect(void* ctx) {
    located = false;
    http_location_request();
}

void location(float latitude, float longitude, float altitude, float accuracy, void* ctx) {
    // Fix the floats
    our_latitude = latitude * 10000;
    our_longitude = longitude * 10000;
    located = true;
    request_weather();

    if(weather_timer != 0) {
        app_timer_cancel_event(ctx, weather_timer);
    }

    weather_timer = app_timer_send_event(ctx, WEATHER_FETCH_FREQUENCY, WEATHER_TIMER_COOKIE);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent* evt) {
    if (splashEnded) {
        if(evt == NULL) {
            PblTm now;
            get_time(&now);
            time_layer_set_time(&time_layer, now);
        } else {
            time_layer_set_time(&time_layer, *(evt->tick_time));
        }

        // If there's been an error, reaquire a location and weather
        if(!located) {
            http_location_request();
        }
    }
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
    if(cookie == SPLASH_TIMER_COOKIE) {
        splashEnded = true;
        handle_tick(ctx, NULL);
    } else if(cookie == WEATHER_TIMER_COOKIE) {
        located = false;
        http_location_request();
    }
}

void handle_init(AppContextRef ctx) {
    resource_init_current_app(&APP_RESOURCES);
    window_init(&window, "Squared Weather");
    window_stack_push(&window, true /* Animated */);
    window_set_background_color(&window, GColorBlack);

    // Add time layer
    time_layer_init(&time_layer, GPoint(ORIGIN_X, TIME_ORIGIN_Y));
    layer_add_child(&window.layer, &time_layer.layer);

    // Add weather layer
    weather_layer_init(&weather_layer, GPoint(ORIGIN_X, WEATHER_ORIGIN_Y));
    layer_add_child(&window.layer, &weather_layer.layer);

    app_timer_send_event(ctx, STARTDELAY, SPLASH_TIMER_COOKIE);

    http_set_app_id(WEATHER_HTTP_COOKIE);
    HTTPCallbacks callbacks = {
        .failure   = failed,
        .success   = success,
        .reconnect = reconnect,
        .location  = location
    };
    http_register_callbacks(callbacks, (void*)ctx);
}

void handle_deinit(AppContextRef ctx) {
    animation_unschedule_all();

    time_layer_deinit(&time_layer);
    weather_layer_deinit(&weather_layer);

    layer_remove_child_layers(&window.layer);
}

void pbl_main(void* params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .deinit_handler = &handle_deinit,
        .timer_handler = &handle_timer,

        .tick_info = {
            .tick_handler = &handle_tick,
            .tick_units   = MINUTE_UNIT
        },

        .messaging_info = {
            .buffer_sizes = {
                .inbound = 124,
                .outbound = 256,
            }
        }
    };
    app_event_loop(params, &handlers);
}
