#include "retr01_studio/collision.h"

int r01_world_apply_solid_tile(R01World *w, int bank, uint8_t tile_id, int set_solid) {
    int si, cell, touched = 0;
    if (!w || bank < 0 || bank >= R01_BG_BANKS) {
        return 0;
    }
    for (si = 0; si < w->screen_count; si++) {
        R01Screen *s = &w->screens[si];
        if (!s->present) {
            continue;
        }
        for (cell = 0; cell < R01_TILES_PER_SCREEN; cell++) {
            if (s->tiles[cell] != tile_id || r01_attr_bank(s->attrs[cell]) != bank) {
                continue;
            }
            if (set_solid) {
                s->attrs[cell] |= R01_ATTR_SOLID;
            } else {
                s->attrs[cell] = (uint8_t)(s->attrs[cell] & ~R01_ATTR_SOLID);
            }
            touched++;
        }
    }
    return touched;
}
