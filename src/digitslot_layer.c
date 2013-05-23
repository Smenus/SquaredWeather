#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "digitslot_layer.h"
#include "config.h"


void digitslot_layer_init(DigitSlot *slot, GRect rect, int divider, int startDigit) {
    slot->normTime = ANIMATION_NORMALIZED_MAX;
    slot->prevDigit = 0;
    slot->curDigit = startDigit;
    slot->divider = divider;
    layer_init(&slot->layer, rect);
    slot->layer.update_proc = (LayerUpdateProc)digitslot_layer_update;
}

void digitslot_layer_update(DigitSlot *slot, GContext *ctx) {
    int tx, ty, w, offs, shift, tilesize, widthadjust;
    widthadjust = 0;
    if (slot->divider == 2) {
        widthadjust = 1;
    }
    tilesize = TILE_SIZE/slot->divider;
    uint32_t skewedNormTime = slot->normTime*3;
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, slot->layer.bounds.size.w, slot->layer.bounds.size.h), 0, GCornerNone);
    for (uint t=0; t<TOTALBLOCKS; t++) {
        w = 0;
        offs = 0;
        tx = t % FONT_WIDTH_BLOCKS;
        ty = t / FONT_HEIGHT_BLOCKS;
        shift = 0-(t-ty);
        if (FONT[slot->curDigit][ty][tx] == FONT[slot->prevDigit][ty][tx]) {
            if(FONT[slot->curDigit][ty][tx]) {
                w = tilesize;
            }
        } else {
            if (slot->normTime == ANIMATION_NORMALIZED_MAX && FONT[slot->curDigit][ty][tx]) {
                w = tilesize;
            } else {
                if(FONT[slot->curDigit][ty][tx] || FONT[slot->prevDigit][ty][tx]) {
                    offs = (skewedNormTime*TILE_SIZE/ANIMATION_NORMALIZED_MAX)+shift;
                    if(FONT[slot->curDigit][ty][tx]) {
                        w = offs;
                        offs = 0;
                    } else {
                        w = tilesize-offs;
                    }
                }
            }
        }
        if (w < 0) {
            w = 0;
        } else if (w > tilesize) {
            w = tilesize;
        }
        if (offs < 0) {
            offs = 0;
        } else if (offs > tilesize) {
            offs = tilesize;
        }
        if (w > 0) {
            graphics_context_set_fill_color(ctx, GColorWhite);
            graphics_fill_rect(ctx, GRect((tx*tilesize)+offs-(tx*widthadjust), ty*tilesize-(ty*widthadjust), w-widthadjust, tilesize-widthadjust), 0, GCornerNone);
        }
    }
}
