#ifndef TIME_LAYER_H
#define TIME_LAYER_H

#include "digitslot_layer.h"

#define TIME_DIGITS 4
#define TIME_DIGIT_CHANGE_ANIM_DURATION 1700

typedef struct {
    Layer layer;
    DigitSlot slots[TIME_DIGITS];
} TimeLayer;

void time_layer_init(TimeLayer* time_layer, GPoint top_left);
void time_layer_deinit(TimeLayer* time_layer);
void time_layer_animate(Animation* anim, const uint32_t normTime);
void time_layer_set_time(TimeLayer* time_layer, PblTm time);

#endif
