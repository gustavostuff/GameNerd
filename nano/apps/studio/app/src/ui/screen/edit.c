#include "ui/ui.h"
#include "ui/internal.h"
#include "font/font.h"

#include "retr01_studio/cart.h"
#include "retr01_studio/chr_pack.h"
#include "retr01_studio/collision.h"
#include "retr01_studio/json_io.h"
#include "retr01_studio/palette.h"
#include "retr01_studio/project.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void screen_refresh_tile(R01World *w, R01Screen *s) {
    if (w && s && s->present) {
        r01_screen_fill_pixels_from_bank(w, s);
    }
}

void screen_refresh_sel(UiState *ui) {
    R01World *w = r01_project_active_world(ui->project);
    R01Screen *s = ui_edit_map_screen(ui);
    screen_refresh_tile(w, s);
}

static void screen_set_sel_attr(UiState *ui, int bank, int pal, int flip_h, int flip_v) {
    R01World *w = r01_project_active_world(ui->project);
    R01Screen *s = ui_edit_map_screen(ui);
    int min_x, min_y, max_x, max_y, ty, tx;
    if (!w || !s || !screen_sel_valid(ui)) {
        return;
    }
    screen_sel_bounds(ui, &min_x, &min_y, &max_x, &max_y);
    for (ty = min_y; ty <= max_y; ty++) {
        for (tx = min_x; tx <= max_x; tx++) {
            int cell = ty * R01_SCREEN_TILES_X + tx;
            uint8_t old = s->attrs[cell];
            s->attrs[cell] = r01_attr_merge(old, bank, pal, flip_h, flip_v);
        }
    }
    screen_refresh_sel(ui);
}

void screen_set_sel_bank(UiState *ui, int bank) {
    R01World *w = r01_project_active_world(ui->project);
    R01Screen *s = ui_edit_map_screen(ui);
    int min_x, min_y, max_x, max_y, ty, tx;
    if (!w || !s || !screen_sel_valid(ui)) {
        return;
    }
    screen_sel_bounds(ui, &min_x, &min_y, &max_x, &max_y);
    for (ty = min_y; ty <= max_y; ty++) {
        for (tx = min_x; tx <= max_x; tx++) {
            int cell = ty * R01_SCREEN_TILES_X + tx;
            uint8_t old = s->attrs[cell];
            s->attrs[cell] =
                r01_attr_merge(old, bank, r01_attr_pal(old), r01_attr_flip_h(old), r01_attr_flip_v(old));
        }
    }
    screen_refresh_sel(ui);
}

void screen_set_sel_pal(UiState *ui, int pal) {
    R01World *w = r01_project_active_world(ui->project);
    R01Screen *s = ui_edit_map_screen(ui);
    int min_x, min_y, max_x, max_y, ty, tx;
    if (!w || !s || !screen_sel_valid(ui)) {
        return;
    }
    screen_sel_bounds(ui, &min_x, &min_y, &max_x, &max_y);
    for (ty = min_y; ty <= max_y; ty++) {
        for (tx = min_x; tx <= max_x; tx++) {
            int cell = ty * R01_SCREEN_TILES_X + tx;
            uint8_t old = s->attrs[cell];
            s->attrs[cell] =
                r01_attr_merge(old, r01_attr_bank(old), pal, r01_attr_flip_h(old), r01_attr_flip_v(old));
        }
    }
    screen_refresh_sel(ui);
}

void screen_toggle_sel_flag(UiState *ui, uint8_t flag) {
    R01Screen *s = ui_edit_map_screen(ui);
    int min_x, min_y, max_x, max_y, ty, tx;
    if (!s || !screen_sel_valid(ui)) {
        return;
    }
    screen_sel_bounds(ui, &min_x, &min_y, &max_x, &max_y);
    for (ty = min_y; ty <= max_y; ty++) {
        for (tx = min_x; tx <= max_x; tx++) {
            int cell = ty * R01_SCREEN_TILES_X + tx;
            s->attrs[cell] ^= flag;
        }
    }
}

void screen_set_solid_by_hw(UiState *ui, int ref_tx, int ref_ty) {
    R01World *w = r01_project_active_world(ui->project);
    R01Screen *s = ui_edit_map_screen(ui);
    int cell;
    uint8_t ref_attr;
    uint8_t tile_id;
    int bank;
    int set_solid;
    int touched;

    if (!w || !s || ref_tx < 0 || ref_tx >= R01_SCREEN_TILES_X || ref_ty < 0 || ref_ty >= R01_SCREEN_TILES_Y) {
        return;
    }
    cell = ref_ty * R01_SCREEN_TILES_X + ref_tx;
    ref_attr = s->attrs[cell];
    tile_id = s->tiles[cell];
    bank = r01_attr_bank(ref_attr);
    set_solid = !r01_attr_solid(ref_attr);
    touched = r01_world_apply_solid_tile(w, bank, tile_id, set_solid);
    /* Keep paint brush solid bit in sync when it targets this CHR. */
    if (ui->paint_stamp_valid && ui->paint_stamp_tile == tile_id &&
        r01_attr_bank(ui->paint_stamp_attr) == bank) {
        if (set_solid) {
            ui->paint_stamp_attr |= R01_ATTR_SOLID;
        } else {
            ui->paint_stamp_attr &= (uint8_t)~R01_ATTR_SOLID;
        }
    }
    screen_refresh_sel(ui);
    if (touched > 0) {
        ui_toast(ui, set_solid ? "solid set (same bank+tile)" : "solid cleared (same bank+tile)", 0);
    }
}
