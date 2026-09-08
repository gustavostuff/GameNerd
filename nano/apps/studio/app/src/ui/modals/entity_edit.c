#include "ui/ui.h"
#include "ui/internal.h"
#include "font/font.h"

#include "retr01_studio/chr_pack.h"
#include "retr01_studio/entities.h"
#include "retr01_studio/palette.h"
#include "retr01_studio/project.h"

#include <string.h>

#define ENTITY_FIELD_NAME 1
#define ENTITY_FIELD_STATE 2

static R01EntityState *edit_state(UiState *ui) {
    return r01_entity_ensure_state(&ui->entity_edit.draft, ui->entity_edit.state);
}

static R01EntityFrame *edit_frame(UiState *ui) {
    R01EntityState *st = edit_state(ui);
    if (!st) {
        return NULL;
    }
    st->frame_count = 1;
    return r01_entity_ensure_frame(&ui->entity_edit.draft, ui->entity_edit.state, 0);
}

static R01EntityPart *nano_entity_part(UiState *ui) {
    R01EntityFrame *fr = edit_frame(ui);
    R01EntityPart pt;
    int bank;
    if (!fr || !ui) {
        return NULL;
    }
    if (fr->part_count >= 1) {
        return &fr->parts[0];
    }
    bank = ui->banks_idx;
    if (bank < 0 || bank >= UI_BANKS_N) {
        bank = 0;
    }
    memset(&pt, 0, sizeof(pt));
    pt.bank = bank;
    pt.tile_id = -1;
    pt.pal = 0;
    if (r01_entity_frame_add_part(fr, &pt) != 0) {
        return NULL;
    }
    return &fr->parts[0];
}

static int state_unlock_count(const UiState *ui) {
    int n = ui->entity_edit.draft.state_count + 1;
    if (n > R01_ENTITY_STATES_MAX) {
        n = R01_ENTITY_STATES_MAX;
    }
    if (n < 1) {
        n = 1;
    }
    return n;
}

static int entity_part_bank(const UiEntityEdit *ed) {
    R01EntityState *st = r01_entity_state((R01EntityType *)&ed->draft, ed->state);
    if (!st || st->frame_count < 1 || st->frames[0].part_count < 1) {
        return 0;
    }
    return st->frames[0].parts[0].bank;
}

static uint8_t *entity_tile_chr(UiState *ui, int allocate, int *out_bank, int *out_tile_id) {
    R01EntityPart *pt;
    R01World *w;
    int bank;
    int tid;

    pt = nano_entity_part(ui);
    w = r01_project_active_world(ui->project);
    if (!pt || !w) {
        return NULL;
    }
    bank = pt->bank;
    if (bank < 0 || bank >= R01_BG_BANKS) {
        bank = 0;
        pt->bank = bank;
    }
    if (out_bank) {
        *out_bank = bank;
    }
    if (pt->tile_id < 0) {
        if (!allocate) {
            return NULL;
        }
        tid = r01_chr_alloc_tile(w, bank);
        if (tid < 0) {
            return NULL;
        }
        pt->tile_id = tid;
    }
    tid = pt->tile_id;
    if (out_tile_id) {
        *out_tile_id = tid;
    }
    if (tid < w->bg_banks[bank].tile_count) {
        return w->bg_banks[bank].chr + (size_t)tid * R01_TILE_BYTES;
    }
    if (!allocate) {
        return NULL;
    }
    if (r01_chr_write_tile(w, bank, tid, (const uint8_t[R01_TILE_BYTES]){0}) != 0) {
        return NULL;
    }
    return w->bg_banks[bank].chr + (size_t)tid * R01_TILE_BYTES;
}

static void entity_normalize_draft(R01EntityType *e) {
    r01_entity_nano_normalize(e);
}

static void entity_paint_pixel(UiState *ui, int sx, int sy, int on) {
    uint8_t *chr;
    int bank, tid;

    chr = entity_tile_chr(ui, 1, &bank, &tid);
    if (!chr) {
        return;
    }
    r01_tile_set_pixel(chr, sx, sy, on ? 1u : 0u);
    (void)r01_chr_write_tile(r01_project_active_world(ui->project), bank, tid, chr);
}

void entity_edit_open_new(UiState *ui) {
    if (!ui) {
        return;
    }
    memset(&ui->entity_edit, 0, sizeof(ui->entity_edit));
    ui->entity_edit.open = 1;
    ui->entity_edit.is_new = 1;
    ui->entity_edit.type_idx = -1;
    r01_entity_type_init(&ui->entity_edit.draft);
    ui->entity_edit.state = 0;
    ui->entity_edit.paint_on = 1;
    (void)r01_entity_ensure_state(&ui->entity_edit.draft, 0);
    ui_focus_set(ui, UI_FOCUS_WORKBENCH);
    ui_text_blur(ui);
}

void entity_edit_open(UiState *ui, int type_idx) {
    R01World *w;
    if (!ui) {
        return;
    }
    w = r01_project_active_world(ui->project);
    if (!w || type_idx < 0 || type_idx >= w->entity_count) {
        return;
    }
    memset(&ui->entity_edit, 0, sizeof(ui->entity_edit));
    ui->entity_edit.open = 1;
    ui->entity_edit.is_new = 0;
    ui->entity_edit.type_idx = type_idx;
    ui->entity_edit.draft = w->entities[type_idx];
    ui->entity_edit.state = 0;
    ui->entity_edit.paint_on = 1;
    entity_normalize_draft(&ui->entity_edit.draft);
    ui_focus_set(ui, UI_FOCUS_WORKBENCH);
    ui_text_blur(ui);
}

static void entity_edit_save(UiState *ui) {
    R01World *w = r01_project_active_world(ui->project);
    int idx;
    if (!w) {
        return;
    }
    entity_normalize_draft(&ui->entity_edit.draft);
    if (ui->entity_edit.draft.state_count < 1) {
        ui->entity_edit.draft.state_count = 1;
    }
    if (ui->entity_edit.is_new || ui->entity_edit.type_idx < 0) {
        idx = r01_world_entity_add(w);
        if (idx < 0) {
            ui_toast(ui, "entity catalog full", 1);
            return;
        }
        w->entities[idx] = ui->entity_edit.draft;
        ui->entity_edit.type_idx = idx;
        ui->entity_edit.is_new = 0;
        ui_toast(ui, "entity created", 0);
    } else {
        if (ui->entity_edit.type_idx >= w->entity_count) {
            ui_toast(ui, "bad entity index", 1);
            return;
        }
        w->entities[ui->entity_edit.type_idx] = ui->entity_edit.draft;
        ui_toast(ui, "entity saved", 0);
    }
    ui->entity_edit.open = 0;
    ui_focus_clear(ui);
    ui_text_blur(ui);
}

void entity_modal_zoom_wheel(UiState *ui, int lx, int ly, int wheel_y) {
    (void)ui;
    (void)lx;
    (void)ly;
    (void)wheel_y;
}

int entity_modal_wheel(UiState *ui, int lx, int ly, int wheel_y, int shift) {
    (void)ui;
    (void)lx;
    (void)ly;
    (void)wheel_y;
    (void)shift;
    return 0;
}

static void draw_entity_tile_canvas(UiState *ui, SDL_Renderer *r, const EntityModalLayout *lo) {
    const uint8_t *chr;
    int sx, sy;
    R01EntityPart *pt;
    uint8_t fr, fg, fb;

    fill_rect(r, lo->canvas_x, lo->canvas_y, UI_TILE_CANVAS, UI_TILE_CANVAS, 0, 0, 0);
    chr = (const uint8_t *)entity_tile_chr(ui, 0, NULL, NULL);
    if (!chr) {
        return;
    }
    pt = nano_entity_part(ui);
    r01_nano_fg_rgb(pt ? (pt->pal & 7) : 0, &fr, &fg, &fb);
    for (sy = 0; sy < 8; sy++) {
        for (sx = 0; sx < 8; sx++) {
            if (r01_tile_pixel_color(chr, sx, sy) != 0) {
                fill_rect(r, lo->canvas_x + sx * 16, lo->canvas_y + sy * 16, 15, 15, fr, fg, fb);
            }
        }
    }
}

void draw_entity_modal(UiState *ui, SDL_Renderer *r) {
    EntityModalLayout lo;
    R01EntityState *st;
    R01EntityPart *pt;
    const char *title = ui->entity_edit.is_new ? "Add entity" : "Edit entity";
    int x;

    entity_modal_layout(ui, &lo);
    x = lo.mx + UI_UNIT;
    ui_modal_scrim(r, ui);
    ui_modal_panel(r, lo.mx, lo.my, lo.mw, lo.mh, title);

    {
        const char *ename = ui->entity_edit.draft.name[0] ? ui->entity_edit.draft.name : "Entity";
        font_draw(r, x, lo.name_y + 4, "Name", 230, 230, 230);
        ui_text_draw(ui, r, lo.name_x, lo.name_y, lo.name_w, ename, ENTITY_FIELD_NAME);
    }

    font_draw(r, x, lo.state_y + 4, "State", 230, 230, 230);
    ui_dot_strip_draw(r, lo.dots_x, lo.dots_y, UI_DOT_STRIP_N, ui->entity_edit.state, state_unlock_count(ui));

    st = edit_state(ui);
    {
        const char *sname = (st && st->name[0]) ? st->name : "Idle";
        font_draw(r, x, lo.state_name_y + 4, "State name", 230, 230, 230);
        ui_text_draw(ui, r, lo.sname_x, lo.state_name_y, lo.sname_w, sname, ENTITY_FIELD_STATE);
    }

    font_draw(r, x, lo.bank_y + 4, "Bank", 230, 230, 230);
    ui_dot_strip_draw(r, lo.bank_dots_x, lo.bank_dots_y, UI_DOT_STRIP_N, entity_part_bank(&ui->entity_edit),
                      UI_DOT_STRIP_N);

    pt = nano_entity_part(ui);
    font_draw(r, x, lo.fg_label_y + 4, "Color", 230, 230, 230);
    ui_fg_strip_draw(r, lo.fg_x, lo.fg_y, pt ? (pt->pal & 7) : 0);

    draw_entity_tile_canvas(ui, r, &lo);

    font_draw(r, x, lo.tip_y + 4, "LMB paint / RMB clear", 160, 160, 170);

    ui_modal_save_cancel(r, x, lo.btn_y, lo.save_w, lo.cancel_w, ui->mouse_x, ui->mouse_y);
}

static int entity_canvas_hit(const EntityModalLayout *lo, int lx, int ly, int *out_sx, int *out_sy) {
    if (!point_in_rect(lx, ly, lo->canvas_x, lo->canvas_y, UI_TILE_CANVAS, UI_TILE_CANVAS)) {
        return 0;
    }
    if (out_sx) {
        *out_sx = (lx - lo->canvas_x) / 16;
    }
    if (out_sy) {
        *out_sy = (ly - lo->canvas_y) / 16;
    }
    return 1;
}

int entity_modal_handle(UiState *ui, int lx, int ly, int down, Uint8 button) {
    EntityModalLayout lo;
    R01EntityState *st;
    R01EntityPart *pt;
    int idx;
    int sx, sy;
    int right = (button == SDL_BUTTON_RIGHT);
    int paint_on;

    entity_modal_layout(ui, &lo);
    st = edit_state(ui);
    pt = nano_entity_part(ui);

    if (!down) {
        ui->entity_edit.dragging = 0;
        ui_text_mouse_up(ui);
        return 1;
    }

    if (ui_modal_overlay_hit(lx, ly, lo.mx, lo.my, lo.mw, lo.mh)) {
        ui->entity_edit.open = 0;
        ui_focus_clear(ui);
        ui_text_blur(ui);
        return 1;
    }

    if (ui_text_mouse_down(ui, lx, ly, lo.name_x, lo.name_y, lo.name_w, ui->entity_edit.draft.name,
                           R01_ENTITY_NAME_MAX, ENTITY_FIELD_NAME)) {
        return 1;
    }
    if (st && ui_text_mouse_down(ui, lx, ly, lo.sname_x, lo.state_name_y, lo.sname_w, st->name, R01_ENTITY_NAME_MAX,
                                 ENTITY_FIELD_STATE)) {
        return 1;
    }
    ui_text_blur(ui);

    if (ui_dot_strip_hit(lx, ly, lo.dots_x, lo.dots_y, UI_DOT_STRIP_N, &idx)) {
        if (idx < state_unlock_count(ui)) {
            if (!r01_entity_ensure_state(&ui->entity_edit.draft, idx)) {
                return 1;
            }
            ui->entity_edit.state = idx;
        }
        return 1;
    }
    if (ui_dot_strip_hit(lx, ly, lo.bank_dots_x, lo.bank_dots_y, UI_DOT_STRIP_N, &idx) && pt) {
        if (idx >= 0 && idx < UI_BANKS_N) {
            pt->bank = idx;
        }
        return 1;
    }
    if (ui_fg_strip_hit(lx, ly, lo.fg_x, lo.fg_y, &idx) && pt) {
        pt->pal = idx & 7;
        return 1;
    }

    if (ui_modal_save_hit(lx, ly, lo.mx + UI_UNIT, lo.btn_y, lo.save_w)) {
        entity_edit_save(ui);
        return 1;
    }
    if (ui_modal_cancel_hit(lx, ly, lo.mx + UI_UNIT, lo.btn_y, lo.save_w, lo.cancel_w)) {
        ui->entity_edit.open = 0;
        ui_focus_clear(ui);
        ui_text_blur(ui);
        return 1;
    }

    if (entity_canvas_hit(&lo, lx, ly, &sx, &sy)) {
        ui_focus_set(ui, UI_FOCUS_WORKBENCH);
        paint_on = right ? 0 : ui->entity_edit.paint_on;
        entity_paint_pixel(ui, sx, sy, paint_on);
        ui->entity_edit.dragging = 1;
        return 1;
    }

    return 1;
}

void entity_modal_drag(UiState *ui, int lx, int ly, Uint32 buttons) {
    EntityModalLayout lo;
    int sx, sy;
    int paint_on;

    if (!ui || !ui->entity_edit.open) {
        return;
    }
    entity_modal_layout(ui, &lo);
    if (ui->text.drag && ui->text.field_id == ENTITY_FIELD_NAME) {
        ui_text_mouse_drag(ui, lx, lo.name_x, lo.name_w);
        return;
    }
    if (ui->text.drag && ui->text.field_id == ENTITY_FIELD_STATE) {
        ui_text_mouse_drag(ui, lx, lo.sname_x, lo.sname_w);
        return;
    }
    if (!ui->entity_edit.dragging) {
        return;
    }
    if (!entity_canvas_hit(&lo, lx, ly, &sx, &sy)) {
        return;
    }
    if (buttons & SDL_BUTTON_LMASK) {
        paint_on = ui->entity_edit.paint_on;
        entity_paint_pixel(ui, sx, sy, paint_on);
    } else if (buttons & SDL_BUTTON_RMASK) {
        entity_paint_pixel(ui, sx, sy, 0);
    }
}

void entity_modal_key(UiState *ui, SDL_Keycode sym) {
    if (!ui || !ui->entity_edit.open) {
        return;
    }
    if (ui->text.field_id > 0) {
        ui_text_key(ui, sym, SDL_GetModState());
        return;
    }
    if (sym == SDLK_1) {
        ui->entity_edit.paint_on = 1;
    } else if (sym == SDLK_0) {
        ui->entity_edit.paint_on = 0;
    }
}
