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

#define WEATHER_HTTP_COOKIE 1138158163
#define SPLASH_COOKIE       1166677173

#define STARTDELAY 2000
	
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
static bool located;

static bool splashEnded = false;


void handle_tick(AppContextRef ctx, PebbleTickEvent *evt) {
    if (splashEnded) {
        if(evt == NULL) {
			PblTm now;
        	get_time(&now);
        	time_layer_set_time(&time_layer, now);
        } else {
        	time_layer_set_time(&time_layer, *(evt->tick_time));
        }
    }
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	if(cookie == SPLASH_COOKIE) {
	    splashEnded = true;
	    handle_tick(ctx, NULL);
	} else if(cookie == WEATHER_HTTP_COOKIE) {

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

	app_timer_send_event(ctx, STARTDELAY /* milliseconds */, SPLASH_COOKIE);
}

void handle_deinit(AppContextRef ctx) {
	animation_unschedule_all();

	time_layer_deinit(&time_layer);
	weather_layer_deinit(&weather_layer);

	layer_remove_child_layers(&window.layer);
}

void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,
		.timer_handler = &handle_timer,

		.tick_info = {
			.tick_handler = &handle_tick,
			.tick_units   = MINUTE_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
