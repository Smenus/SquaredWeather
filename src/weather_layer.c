#include "pebble_os.h"
#include "pebble_app.h"

#include "weather_layer.h"

#include "config.h"

static AnimationImplementation weather_anim_impl;
static Animation weather_anim;

static int temp_start_digit[TEMP_DIGITS] = {
    11,10,11,10
};

static uint8_t WEATHER_ICONS[] = {
	RESOURCE_ID_ICON_CLEAR_DAY,
	RESOURCE_ID_ICON_CLEAR_NIGHT,
	RESOURCE_ID_ICON_RAIN,
	RESOURCE_ID_ICON_SNOW,
	RESOURCE_ID_ICON_SLEET,
	RESOURCE_ID_ICON_WIND,
	RESOURCE_ID_ICON_FOG,
	RESOURCE_ID_ICON_CLOUDY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT,
	RESOURCE_ID_ICON_ERROR,
};

GRect frame_for_temp_digitslot(int i) {
    int x, y, w, h;
    w = i!=3 ? FONT_WIDTH/2 : 20;
    h = i!=3 ? FONT_HEIGHT/2 : 20;
    x = (FONT_WIDTH + SPACING_X) * i / 2;
    y = 0;
    return GRect(x, y, w, h);
}

void weather_layer_init(WeatherLayer* weather_layer, GPoint top_left) {
    GRect frame = GRect(top_left.x, top_left.y, FONT_WIDTH * 2 + SPACING_X, FONT_HEIGHT / 2);
    layer_init(&weather_layer->layer, frame);

    for (int i=0; i<TEMP_DIGITS; i++) {
        digitslot_layer_init(&weather_layer->slots[i], frame_for_temp_digitslot(i), 2, temp_start_digit[i]);
        layer_add_child(&weather_layer->layer, &weather_layer->slots[i].layer);
    }

	// Note absence of icon layer
	weather_layer->has_weather_icon = false;
	weather_layer->icon = WEATHER_ICON_NO_WEATHER;

	weather_anim_impl.setup = NULL;
    weather_anim_impl.update = weather_layer_animate;
    weather_anim_impl.teardown = NULL;

    animation_init(&weather_anim);
    animation_set_delay(&weather_anim, 0);
    animation_set_duration(&weather_anim, TEMP_DIGIT_CHANGE_ANIM_DURATION);
    animation_set_implementation(&weather_anim, &weather_anim_impl);
    animation_set_handlers(&weather_anim, (AnimationHandlers) {NULL, NULL}, weather_layer);
}

void weather_layer_deinit(WeatherLayer* weather_layer) {
    if (animation_is_scheduled(&weather_anim))
            animation_unschedule(&weather_anim);

    layer_remove_child_layers(&weather_layer->layer);
	if(weather_layer->has_weather_icon) {
		bmp_deinit_container(&weather_layer->icon_layer);
		weather_layer->has_weather_icon = false;
		weather_layer->icon = WEATHER_ICON_NO_WEATHER;
	}
}

void weather_layer_animate(Animation *anim, const uint32_t normTime) {
    WeatherLayer* weather_layer = (WeatherLayer*)animation_get_context(anim);
    for (int i=0; i<TEMP_DIGITS; i++) {
        if (weather_layer->slots[i].curDigit != weather_layer->slots[i].prevDigit) {
            weather_layer->slots[i].normTime = normTime;
            layer_mark_dirty(&weather_layer->slots[i].layer);
        }
    }
}

void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon) {
    if(icon == weather_layer->icon) return;

	if(weather_layer->has_weather_icon) {
		layer_remove_from_parent(&weather_layer->icon_layer.layer.layer);
		bmp_deinit_container(&weather_layer->icon_layer);
		weather_layer->has_weather_icon = false;
		weather_layer->icon = WEATHER_ICON_NO_WEATHER;
	}

	bmp_init_container(WEATHER_ICONS[icon], &weather_layer->icon_layer);
	layer_set_frame(&weather_layer->icon_layer.layer.layer, frame_for_temp_digitslot(3));
	layer_add_child(&weather_layer->layer, &weather_layer->icon_layer.layer.layer);
	weather_layer->has_weather_icon = true;
	weather_layer->icon = icon;
}

void weather_layer_set_temp(WeatherLayer* weather_layer, uint16_t temp, int8_t is_c) {
    if (animation_is_scheduled(&weather_anim))
            animation_unschedule(&weather_anim);

    for (int i=0; i<TEMP_DIGITS; i++) {
        weather_layer->slots[i].prevDigit = weather_layer->slots[i].curDigit;
    }
    
    weather_layer->slots[0].curDigit = temp/10;
    weather_layer->slots[1].curDigit = temp%10;

    if(is_c == 1) {
        weather_layer->slots[2].curDigit = 12;
    } else if(is_c == 0) {
        weather_layer->slots[2].curDigit = 13;
    } else {
        weather_layer->slots[2].curDigit = 14;
    }
    
    if (NO_ZERO && weather_layer->slots[0].curDigit == 0) {
        weather_layer->slots[0].curDigit = 10;
        if (weather_layer->slots[0].prevDigit == 10) {
            weather_layer->slots[0].curDigit++;
        }
    }

    animation_schedule(&weather_anim);
}

void weather_layer_clear_icon(WeatherLayer* weather_layer) {
	if(weather_layer->has_weather_icon) {
		layer_remove_from_parent(&weather_layer->icon_layer.layer.layer);
		bmp_deinit_container(&weather_layer->icon_layer);
		weather_layer->has_weather_icon = false;
		weather_layer->icon = WEATHER_ICON_NO_WEATHER;
	}
}

void weather_layer_clear_temp(WeatherLayer* weather_layer) {
    if (animation_is_scheduled(&weather_anim))
            animation_unschedule(&weather_anim);

    for (int i=0; i<TEMP_DIGITS; i++) {
        weather_layer->slots[i].prevDigit = weather_layer->slots[i].curDigit;
        weather_layer->slots[i].curDigit = temp_start_digit[i];
    }

    animation_schedule(&weather_anim);
}
