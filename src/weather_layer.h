#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

#include "digitslot_layer.h"

#define TEMP_DIGITS 4
#define TEMP_DIGIT_CHANGE_ANIM_DURATION 1700

typedef enum {
	WEATHER_ICON_CLEAR_DAY = 0,
	WEATHER_ICON_CLEAR_NIGHT,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_SLEET,
	WEATHER_ICON_WIND,
	WEATHER_ICON_FOG,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_PARTLY_CLOUDY_DAY,
	WEATHER_ICON_PARTLY_CLOUDY_NIGHT,
	WEATHER_ICON_NO_WEATHER,
	WEATHER_ICON_COUNT
} WeatherIcon;

typedef struct {
	Layer layer;
	BmpContainer icon_layer;
	bool has_weather_icon;
	WeatherIcon icon;
    DigitSlot slots[TEMP_DIGITS];
} WeatherLayer;


void weather_layer_init(WeatherLayer* weather_layer, GPoint top_left);
void weather_layer_deinit(WeatherLayer* weather_layer);
void weather_layer_animate(Animation *anim, const uint32_t normTime);
void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon);
void weather_layer_set_temp(WeatherLayer* weather_layer, uint16_t temp, int8_t is_c);
void weather_layer_clear_icon(WeatherLayer* weather_layer);
void weather_layer_clear_temp(WeatherLayer* weather_layer);

#endif
