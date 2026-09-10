#include "ui/ui.h"
#include "ui/internal.h"
#include "font/font.h"

#include "retr01_studio/chr_pack.h"
#include "retr01_studio/palette.h"
#include "retr01_studio/project.h"

#include "retr01_nano_emu/machine.h"
#include "retr01_nano_emu/video.h"

#include <string.h>

static void draw_bg_tile_px(SDL_Renderer *r, const R01Project *p, const R01World *w, int bank, int tile_id,
                            int pal, int flip_h, int flip_v, int dx, int dy, int scale) {
    const uint8_t *raw;
    uint8_t tile[R01_TILE_BYTES];
    int sy, sx;
    if (!p || !w || bank < 0 || bank >= R01_BG_BANKS || tile_id < 0) {
        return;
    }
    if (tile_id >= w->bg_banks[bank].tile_count) {
        return;
    }
    raw = w->bg_banks[bank].chr + (size_t)tile_id * R01_TILE_BYTES;
    r01_tile_orient(raw, flip_h, flip_v, tile);
    for (sy = 0; sy < 8; sy++) {
        for (sx = 0; sx < 8; sx++) {
            uint8_t c = r01_tile_pixel_color(tile, sx, sy);
            uint8_t R, G, B;
            int row = w->default_pal_row;
            if (c == 0) {
                continue;
            }
            r01_kit_rgb(p->global_pal_bg[row][pal & 3].idx[c & 3], &R, &G, &B);
            fill_rect(r, dx + sx * scale, dy + sy * scale, scale, scale, R, G, B);
        }
    }
}

void draw_screen_editor(UiState *ui, SDL_Renderer *r, const R01Screen *s) {
    const R01World *w;
    int ox, oy, scale;
    int ty, tx;
    int mark;
    if (!ui || !r || !s || !ui->project) {
        return;
    }
    w = r01_project_active_world_const(ui->project);
    if (!w) {
        return;
    }
    screen_origin(ui, &ox, &oy);
    scale = ui_screen_scale(ui);
    fill_rect(r, ox, oy, ui_screen_w(ui), ui_screen_h(ui), UI_COL_WELL_R, UI_COL_WELL_G, UI_COL_WELL_B);
    for (ty = 0; ty < R01_SCREEN_TILES_Y; ty++) {
        for (tx = 0; tx < R01_SCREEN_TILES_X; tx++) {
            int cell = ty * R01_SCREEN_TILES_X + tx;
            uint8_t attr = s->attrs[cell];
            draw_bg_tile_px(r, ui->project, w, r01_attr_bank(attr), s->tiles[cell], r01_attr_pal(attr),
                            r01_attr_flip_h(attr), r01_attr_flip_v(attr), ox + tx * 8 * scale,
                            oy + ty * 8 * scale, scale);
        }
    }
    if (screen_sel_valid(ui) && ui->screen_mode == UI_SCREEN_MODE_SEL) {
        int x0, y0, x1, y1;
        screen_sel_bounds(ui, &x0, &y0, &x1, &y1);
        draw_rect(r, ox + x0 * 8 * scale, oy + y0 * 8 * scale, (x1 - x0 + 1) * 8 * scale,
                  (y1 - y0 + 1) * 8 * scale, UI_COL_MARK_R, UI_COL_MARK_G, UI_COL_MARK_B);
    }
    mark = ui_play_screen_mark(ui);
    (void)mark;
}

void draw_play_view(UiState *ui, SDL_Renderer *r) {
    int ox, oy, dw, dh;
    SDL_Rect dst;
    if (!ui || !r || !ui->play.active) {
        return;
    }
    screen_origin(ui, &ox, &oy);
    dw = ui_screen_w(ui);
    dh = ui_screen_h(ui);
    fill_rect(r, ox, oy, dw, dh, UI_COL_WELL_R, UI_COL_WELL_G, UI_COL_WELL_B);
    if (ui->play.booting || !ui->play.machine || !ui->play.fb_tex) {
        font_draw_centered(r, ox, oy, dw, dh, "booting...", 200, 200, 210);
        return;
    }
    {
        void *pixels = NULL;
        int pitch = 0;
        if (SDL_LockTexture(ui->play.fb_tex, NULL, &pixels, &pitch) == 0) {
            const uint8_t *fb = ui->play.machine->video.fb;
            int y;
            for (y = 0; y < R01NE_VISIBLE_H; y++) {
                memcpy((uint8_t *)pixels + y * pitch, fb + (size_t)y * R01NE_VISIBLE_W * 3,
                       (size_t)R01NE_VISIBLE_W * 3);
            }
            SDL_UnlockTexture(ui->play.fb_tex);
        }
    }
    dst.x = ox;
    dst.y = oy;
    dst.w = dw;
    dst.h = dh;
    SDL_RenderCopy(r, ui->play.fb_tex, NULL, &dst);
}

void draw_catalog_drag_ghost(UiState *ui, SDL_Renderer *r) {
    (void)ui;
    (void)r;
}
