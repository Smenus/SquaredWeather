#include "pebble_os.h"
#include "pebble_app.h"

#include "time_layer.h"

#include "config.h"

static AnimationImplementation time_anim_impl;
static Animation time_anim;

static int time_start_digit[TIME_DIGITS] = {
    11,10,10,11
};

GRect frame_for_time_digitslot(int i) {
    int x, y, w, h;
    w = FONT_WIDTH;
    h = FONT_HEIGHT;
    x = i%2 ? FONT_WIDTH + SPACING_X : 0;
    y = i<2 ? 0 : FONT_HEIGHT + SPACING_Y;
    return GRect(x, y, w, h);
}

unsigned short get_display_hour(unsigned short hour) {
    if (clock_is_24h_style()) {
        return hour;
    }

    unsigned short display_hour = hour % 12;
    return display_hour ? display_hour : 12;
}

void time_layer_init(TimeLayer* time_layer, GPoint top_left) {
    GRect frame = GRect(top_left.x, top_left.y, FONT_WIDTH * 2 + SPACING_X, FONT_HEIGHT * 2 + SPACING_Y);
    layer_init(&time_layer->layer, frame);

    for (int i=0; i<TIME_DIGITS; i++) {
        digitslot_layer_init(&time_layer->slots[i], frame_for_time_digitslot(i), 1, time_start_digit[i]);
        layer_add_child(&time_layer->layer, &time_layer->slots[i].layer);
    }

    time_anim_impl.setup = NULL;
    time_anim_impl.update = time_layer_animate;
    time_anim_impl.teardown = NULL;

    animation_init(&time_anim);
    animation_set_delay(&time_anim, 0);
    animation_set_duration(&time_anim, TIME_DIGIT_CHANGE_ANIM_DURATION);
    animation_set_implementation(&time_anim, &time_anim_impl);
    animation_set_handlers(&time_anim, (AnimationHandlers) {
        NULL, NULL
    }, time_layer);
}

void time_layer_deinit(TimeLayer* time_layer) {
    if (animation_is_scheduled(&time_anim)) {
        animation_unschedule(&time_anim);
    }

    layer_remove_child_layers(&time_layer->layer);
}

void time_layer_animate(Animation* anim, const uint32_t normTime) {
    TimeLayer* time_layer = (TimeLayer*)animation_get_context(anim);

    for (int i=0; i<TIME_DIGITS; i++) {
        if (time_layer->slots[i].curDigit != time_layer->slots[i].prevDigit) {
            time_layer->slots[i].normTime = normTime;
            layer_mark_dirty(&time_layer->slots[i].layer);
        }
    }
}

void time_layer_set_time(TimeLayer* time_layer, PblTm time) {
    int ho, mi;
    ho = get_display_hour(time.tm_hour);
    mi = time.tm_min;

    if (animation_is_scheduled(&time_anim)) {
        animation_unschedule(&time_anim);
    }

    for (int i=0; i<TIME_DIGITS; i++) {
        time_layer->slots[i].prevDigit = time_layer->slots[i].curDigit;
    }

    time_layer->slots[0].curDigit = ho/10;
    time_layer->slots[1].curDigit = ho%10;
    time_layer->slots[2].curDigit = mi/10;
    time_layer->slots[3].curDigit = mi%10;

    if (NO_ZERO && time_layer->slots[0].curDigit == 0) {
        time_layer->slots[0].curDigit = 10;

        if (time_layer->slots[0].prevDigit == 10) {
            time_layer->slots[0].curDigit++;
        }
    }

    animation_schedule(&time_anim);
}
