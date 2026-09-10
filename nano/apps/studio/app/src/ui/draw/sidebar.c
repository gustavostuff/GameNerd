#include "ui/ui.h"
#include "ui/internal.h"
#include "font/font.h"

#include "retr01_studio/cart.h"
#include "retr01_studio/chr_pack.h"
#include "retr01_studio/json_io.h"
#include "retr01_studio/palette.h"
#include "retr01_studio/project.h"
#include "retr01_studio/metatiles.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void accordion_body_clip(SDL_Renderer *r, int body_y, int body_h, UiClipStack *stack) {
    ui_clip_push(r, 0, body_y, UI_SIDEBAR_W, body_h, stack);
}

static void accordion_body_clip_pop(SDL_Renderer *r, const UiClipStack *stack) {
    ui_clip_pop(r, stack);
}

static void draw_worlds_body(UiState *ui, SDL_Renderer *r, const AccordionLayout *lo) {
    R01World *w = r01_project_active_world(ui->project);
    int col, row;
    int lx = ui->mouse_x;
    int ly = ui->mouse_y;
    UiTabsLayout tabs;

    worlds_tabs_prepare(ui, &tabs);
    ui_tabs_draw(r, &tabs, ui->project->active_world, lx, ly);

    draw_chess_grid(r, UI_WORLDS_X, lo->worlds_grid_y, R01_GRID_MAX, R01_GRID_MAX, UI_WORLD_CELL);

    if (!w || !w->present) {
        return;
    }
    {
        int mark_idx = ui->play.active ? ui_play_screen_mark(ui) : w->default_screen;
        if (mark_idx < 0 || mark_idx >= w->screen_count || !w->screens[mark_idx].present) {
            mark_idx = r01_world_default_screen(w);
        }
        for (row = 0; row < R01_GRID_MAX; row++) {
            for (col = 0; col < R01_GRID_MAX; col++) {
                int idx = r01_world_screen_index(w, col, row);
                int x = UI_WORLDS_X + col * UI_WORLD_CELL;
                int y = lo->worlds_grid_y + row * UI_WORLD_CELL;
                int present = (idx >= 0 && w->screens[idx].present);
                int marked = present && idx == mark_idx;
                int hover = point_in_rect(lx, ly, x, y, UI_WORLD_CELL, UI_WORLD_CELL);
                if (present && !marked) {
                    fill_rect_alpha(r, x, y, UI_WORLD_CELL, UI_WORLD_CELL, UI_COL_PRESENT_R, UI_COL_PRESENT_G,
                                    UI_COL_PRESENT_B, 204);
                }
                if (marked) {
                    fill_rect_alpha(r, x, y, UI_WORLD_CELL, UI_WORLD_CELL, UI_COL_MARK_R, UI_COL_MARK_G,
                                    UI_COL_MARK_B, 204);
                }
                if (hover) {
                    hover_overlay(r, x, y, UI_WORLD_CELL, UI_WORLD_CELL);
                }
            }
        }
        if (!ui->play.active && ui->world_sel_col >= 0 && ui->world_sel_row >= 0 &&
            ui->world_sel_col < R01_GRID_MAX && ui->world_sel_row < R01_GRID_MAX) {
            int x = UI_WORLDS_X + ui->world_sel_col * UI_WORLD_CELL;
            int y = lo->worlds_grid_y + ui->world_sel_row * UI_WORLD_CELL;
            draw_rect(r, x, y, UI_WORLD_CELL, UI_WORLD_CELL, 255, 255, 255);
        }
    }
}


static void draw_banks_body(UiState *ui, SDL_Renderer *r, const AccordionLayout *lo) {
    const R01World *w = r01_project_active_world_const(ui->project);
    int lx = ui->mouse_x;
    int ly = ui->mouse_y;
    UiTabsLayout tabs;
    int bank = ui->banks_idx;
    int grid_y = lo->banks_body_y + UI_BTN_H;
    int tx, ty;

    fill_rect(r, 0, lo->banks_body_y, UI_SIDEBAR_W, UI_BANKS_BODY_H, UI_COL_PANEL_R, UI_COL_PANEL_G,
              UI_COL_PANEL_B);

    if (bank < 0) {
        bank = 0;
    }
    if (bank >= UI_BANKS_N) {
        bank = UI_BANKS_N - 1;
    }
    banks_tabs_prepare(ui, &tabs);
    ui_tabs_draw(r, &tabs, bank, lx, ly);

    fill_rect(r, UI_WORLDS_X, grid_y, UI_BANKS_GRID, UI_BANKS_GRID, UI_COL_WELL_R, UI_COL_WELL_G, UI_COL_WELL_B);
    if (!w) {
        return;
    }
    for (ty = 0; ty < 16; ty++) {
        for (tx = 0; tx < 16; tx++) {
            int tile_id = ty * 16 + tx;
            int dx = UI_WORLDS_X + tx * 8;
            int dy = grid_y + ty * 8;
            int sx, sy;
            const uint8_t *tile = NULL;
            if (tile_id < w->bg_banks[bank].tile_count) {
                tile = w->bg_banks[bank].chr + (size_t)tile_id * R01_TILE_BYTES;
            }
            if (!tile) {
                continue;
            }
            for (sy = 0; sy < 8; sy++) {
                for (sx = 0; sx < 8; sx++) {
                    uint8_t col = r01_tile_pixel_color(tile, sx, sy);
                    uint8_t cr, cg, cb;
                    if (col == 0) {
                        continue;
                    }
                    r01_nano_fg_rgb(0, &cr, &cg, &cb);
                    fill_rect(r, dx + sx, dy + sy, 1, 1, cr, cg, cb);
                }
            }
        }
    }
    if (banks_cell_hit(ui, lx, ly, NULL)) {
        int tid;
        int hx, hy;
        banks_cell_hit(ui, lx, ly, &tid);
        hx = UI_WORLDS_X + (tid % 16) * 8;
        hy = grid_y + (tid / 16) * 8;
        hover_overlay(r, hx, hy, 8, 8);
    }
    /* Selected paint brush tile (from Banks pick or tile editor). */
    if (ui->paint_stamp_valid && r01_attr_bank(ui->paint_stamp_attr) == bank) {
        int tid = ui->paint_stamp_tile;
        int hx = UI_WORLDS_X + (tid % 16) * 8;
        int hy = grid_y + (tid / 16) * 8;
        draw_rect(r, hx, hy, 8, 8, 255, 255, 255);
    }
}


static void draw_metatiles_body(UiState *ui, SDL_Renderer *r, const AccordionLayout *lo) {
    const R01World *w = r01_project_active_world_const(ui->project);
    int lx = ui->mouse_x;
    int ly = ui->mouse_y;
    int add_y = lo->metatiles_body_y + UI_METATILES_BODY_H - UI_BTN_H;
    int add_w = label_width("Add");
    int add_hover = point_in_rect(lx, ly, UI_WORLDS_X + UI_UNIT, add_y, add_w, UI_BTN_H);
    int vis = (UI_METATILES_BODY_H - UI_BTN_H) / UI_SPRITE_ROW_H;
    int i;

    fill_rect(r, 0, lo->metatiles_body_y, UI_SIDEBAR_W, UI_METATILES_BODY_H, UI_COL_PANEL_R, UI_COL_PANEL_G,
              UI_COL_PANEL_B);

    if (!w || w->metatile_count < 1) {
        font_draw_centered(r, 0, lo->metatiles_body_y, UI_SIDEBAR_W, UI_BTN_H * 2, "empty", 160, 160, 170);
    } else {
        int max_scroll = w->metatile_count - vis;
        if (max_scroll < 0) {
            max_scroll = 0;
        }
        if (ui->metatiles_scroll > max_scroll) {
            ui->metatiles_scroll = max_scroll;
        }
        if (ui->metatiles_scroll < 0) {
            ui->metatiles_scroll = 0;
        }
        for (i = 0; i < vis; i++) {
            int idx = ui->metatiles_scroll + i;
            int y = lo->metatiles_body_y + i * UI_SPRITE_ROW_H;
            const char *label;
            int hover;
            if (idx >= w->metatile_count) {
                break;
            }
            hover = point_in_rect(lx, ly, 0, y, UI_SIDEBAR_W, UI_SPRITE_ROW_H);
            if (hover) {
                fill_rect(r, 0, y, UI_SIDEBAR_W, UI_SPRITE_ROW_H, UI_COL_WELL_R, UI_COL_WELL_G, UI_COL_WELL_B);
            }
            fill_rect(r, 0, y, UI_PREVIEW_ICON, UI_PREVIEW_ICON, UI_COL_WELL_R, UI_COL_WELL_G, UI_COL_WELL_B);
            label = r01_metatile_display_name(&w->metatiles[idx]);
            font_draw_clipped(r, UI_PREVIEW_ICON + 2, y + 4, UI_PREVIEW_ICON + 2, y,
                              UI_SIDEBAR_W - (UI_PREVIEW_ICON + 2), UI_SPRITE_ROW_H, label, 230, 230, 230);
            if (hover) {
                char id[96];
                int wi = ui->project ? ui->project->active_world : 0;
                r01_metatile_id(id, sizeof(id), wi, &w->metatiles[idx]);
                ui_tooltip_hover(ui, lx, ly, label, id);
            }
        }
    }

    draw_button(r, UI_WORLDS_X + UI_UNIT, add_y, add_w, "Add", 1, add_hover);
}



void draw_sidebar(UiState *ui, SDL_Renderer *r) {
    AccordionLayout lo;
    UiClipStack clip;
    int lx = ui->mouse_x;
    int ly = ui->mouse_y;

    accordion_layout(ui, &lo);
    fill_rect(r, 0, UI_APP_CHROME_H, UI_SIDEBAR_W, ui_logic_h(ui) - UI_APP_CHROME_H, UI_COL_PANEL_R, UI_COL_PANEL_G,
              UI_COL_PANEL_B);

    if (lo.worlds_body_h > 0) {
        accordion_body_clip(r, lo.worlds_btns_y, lo.worlds_body_h, &clip);
        draw_worlds_body(ui, r, &lo);
        accordion_body_clip_pop(r, &clip);
    }
    if (lo.banks_body_h > 0) {
        accordion_body_clip(r, lo.banks_body_y, lo.banks_body_h, &clip);
        draw_banks_body(ui, r, &lo);
        accordion_body_clip_pop(r, &clip);
    }
    if (lo.metatiles_body_h > 0) {
        accordion_body_clip(r, lo.metatiles_body_y, lo.metatiles_body_h, &clip);
        draw_metatiles_body(ui, r, &lo);
        accordion_body_clip_pop(r, &clip);
    }

    draw_accordion_header(r, lo.worlds_hdr_y, "Worlds", lo.worlds_open,
                          point_in_rect(lx, ly, 0, lo.worlds_hdr_y, UI_SIDEBAR_W, UI_BTN_H));
    draw_accordion_header(r, lo.banks_hdr_y, "Banks", lo.banks_open,
                          point_in_rect(lx, ly, 0, lo.banks_hdr_y, UI_SIDEBAR_W, UI_BTN_H));
    draw_accordion_header(r, lo.metatiles_hdr_y, "Metatiles", lo.metatiles_open,
                          point_in_rect(lx, ly, 0, lo.metatiles_hdr_y, UI_SIDEBAR_W, UI_BTN_H));
}
