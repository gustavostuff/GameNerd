#include "retr01_nano_sim/ui.h"

#include "retr01_sim/bus.h"
#include "video_sink.h"

#include "r01_pad_keys.h"

#include <stdio.h>
#include <string.h>

/* Tiny 5x7 glyphs for HUD (ASCII 32..90 subset). */
static const uint8_t FONT5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* space */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, /* G */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* : */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* - */
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* . */
    {0x14, 0x08, 0x3E, 0x08, 0x14}, /* * */
};

static const uint8_t *glyph_for(char ch) {
    if (ch >= '0' && ch <= '9') {
        return FONT5x7[(ch - '0') + 1];
    }
    if (ch >= 'A' && ch <= 'Z') {
        return FONT5x7[(ch - 'A') + 11];
    }
    if (ch >= 'a' && ch <= 'z') {
        return FONT5x7[(ch - 'a') + 11];
    }
    if (ch == ':') {
        return FONT5x7[37];
    }
    if (ch == '-') {
        return FONT5x7[38];
    }
    if (ch == '.') {
        return FONT5x7[39];
    }
    if (ch == '*' || ch == '#') {
        return FONT5x7[40];
    }
    return FONT5x7[0];
}

static void draw_text(SDL_Renderer *r, int x, int y, const char *text, Uint8 R, Uint8 G, Uint8 B) {
    int cx = x;
    const char *p;
    if (!text) {
        return;
    }
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for (p = text; *p; p++) {
        const uint8_t *g = glyph_for(*p);
        int col, row;
        for (col = 0; col < 5; col++) {
            for (row = 0; row < 7; row++) {
                if (g[col] & (1u << row)) {
                    SDL_RenderDrawPoint(r, cx + col, y + row);
                }
            }
        }
        cx += 6;
    }
}

static void fill_rect(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B) {
    SDL_Rect rc = {x, y, w, h};
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderFillRect(r, &rc);
}

static void draw_rect(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B) {
    SDL_Rect rc = {x, y, w, h};
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderDrawRect(r, &rc);
}

static void pin_rgb(R01sLevel lvl, R01sPinDir dir, Uint8 *pr, Uint8 *pg, Uint8 *pb) {
    if (dir == R01S_PIN_PWR) {
        *pr = 220;
        *pg = 70;
        *pb = 70;
        return;
    }
    switch (lvl) {
    case R01S_LVL_H:
        *pr = 70;
        *pg = 210;
        *pb = 90;
        break;
    case R01S_LVL_L:
        *pr = 28;
        *pg = 32;
        *pb = 30;
        break;
    case R01S_LVL_X:
        *pr = 220;
        *pg = 80;
        *pb = 200;
        break;
    default:
        *pr = 120;
        *pg = 125;
        *pb = 110;
        break;
    }
}

static int board_sx(const R01nsUi *ui, int bx) {
    return bx - ui->pan_x;
}
static int board_sy(const R01nsUi *ui, int by) {
    return by - ui->pan_y + R01NS_UI_VIEW_Y;
}

int r01ns_ui_init(R01nsUi *ui, R01nsBoard *board) {
    if (!ui) {
        return -1;
    }
    memset(ui, 0, sizeof(*ui));
    ui->board = board;
    ui->selected = -1;
    ui->drag_chip = -1;
    snprintf(ui->status, sizeof(ui->status),
             "SPACE pause. WASD/ARROWS pads. G=X. Esc quit. Drag pan. Click IC.");
    return 0;
}

void r01ns_ui_shutdown(R01nsUi *ui) {
    if (!ui) {
        return;
    }
    if (ui->lcd_tex) {
        SDL_DestroyTexture(ui->lcd_tex);
        ui->lcd_tex = NULL;
    }
    memset(ui, 0, sizeof(*ui));
}

void r01ns_ui_mount_builder(R01nsUi *ui) {
    R01sIslandBuilder *b;
    int i;
    if (!ui || !ui->board) {
        return;
    }
    b = &ui->board->builder;
    ui->chip_count = 0;
    for (i = 0; i < b->mount_count && ui->chip_count < R01NS_UI_MAX_CHIPS; i++) {
        ui->chips[ui->chip_count] = b->mounts[i].entity;
        ui->chip_island[ui->chip_count] = (uint8_t)b->mounts[i].island_index;
        ui->chip_count++;
    }
}

void r01ns_ui_sync_gamepads(R01nsUi *ui) {
    const Uint8 *keys;
    uint8_t bits;
    if (!ui) {
        return;
    }
    keys = SDL_GetKeyboardState(NULL);
    bits = r01_pad_bits_p1(keys);
    r01s_gamepad_input_clear(&ui->gamepad[0]);
    if (bits & R01_PAD_RIGHT) {
        ui->gamepad[0].stick_x = R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_LEFT) {
        ui->gamepad[0].stick_x = -R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_DOWN) {
        ui->gamepad[0].stick_y = R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_UP) {
        ui->gamepad[0].stick_y = -R01S_GAMEPAD_STICK_RADIUS;
    }
    ui->gamepad[0].btn_x = (bits & R01_PAD_X) ? 1 : 0;
    ui->gamepad[0].btn_y = (bits & R01_PAD_Y) ? 1 : 0;
    ui->gamepad[0].btn_coin = (bits & R01_PAD_COIN) ? 1 : 0;
    ui->gamepad[0].btn_start = (bits & R01_PAD_START) ? 1 : 0;

    bits = r01_pad_bits_p2(keys);
    r01s_gamepad_input_clear(&ui->gamepad[1]);
    if (bits & R01_PAD_RIGHT) {
        ui->gamepad[1].stick_x = R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_LEFT) {
        ui->gamepad[1].stick_x = -R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_DOWN) {
        ui->gamepad[1].stick_y = R01S_GAMEPAD_STICK_RADIUS;
    }
    if (bits & R01_PAD_UP) {
        ui->gamepad[1].stick_y = -R01S_GAMEPAD_STICK_RADIUS;
    }
    ui->gamepad[1].btn_x = (bits & R01_PAD_X) ? 1 : 0;
    ui->gamepad[1].btn_y = (bits & R01_PAD_Y) ? 1 : 0;
    ui->gamepad[1].btn_coin = (bits & R01_PAD_COIN) ? 1 : 0;
    ui->gamepad[1].btn_start = (bits & R01_PAD_START) ? 1 : 0;
}

uint8_t r01ns_ui_pad_byte(const R01nsUi *ui) {
    return ui ? r01s_gamepad_encode(&ui->gamepad[0]) : 0;
}

static int hit_chip(const R01nsUi *ui, int lx, int ly) {
    int i;
    int bx = lx + ui->pan_x;
    int by = ly - R01NS_UI_VIEW_Y + ui->pan_y;
    for (i = ui->chip_count - 1; i >= 0; i--) {
        const R01sEntity *e = ui->chips[i];
        if (!e) {
            continue;
        }
        if (bx >= e->board_x && bx < e->board_x + e->body_w && by >= e->board_y &&
            by < e->board_y + e->body_h) {
            return i;
        }
    }
    return -1;
}

void r01ns_ui_handle_event(R01nsUi *ui, const SDL_Event *e, int logic_x, int logic_y) {
    R01sIslandGroup *group;
    if (!ui || !e || !ui->board) {
        return;
    }
    group = r01ns_board_group(ui->board);
    ui->mouse_lx = logic_x;
    ui->mouse_ly = logic_y;

    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_SPACE && group) {
            group->running = !group->running;
            snprintf(ui->status, sizeof(ui->status), "%s", group->running ? "RUN" : "PAUSE");
        } else if (e->key.keysym.sym == SDLK_r && ui->board->mcu.booted) {
            (void)r01ne_machine_reset(&ui->board->mcu.machine);
            r01ns_video_sink_blit_rgb(&ui->board->sink, ui->board->mcu.machine.video.fb,
                                      sizeof(ui->board->mcu.machine.video.fb));
            snprintf(ui->status, sizeof(ui->status), "reset");
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        ui->selected = hit_chip(ui, logic_x, logic_y);
        ui->drag_chip = ui->selected;
    } else if (e->type == SDL_MOUSEBUTTONDOWN &&
               (e->button.button == SDL_BUTTON_MIDDLE || e->button.button == SDL_BUTTON_RIGHT)) {
        ui->drag_pan = 1;
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        ui->drag_pan = 0;
        ui->drag_chip = -1;
    } else if (e->type == SDL_MOUSEMOTION) {
        if (ui->drag_pan) {
            ui->pan_x -= e->motion.xrel;
            ui->pan_y -= e->motion.yrel;
        } else if (ui->drag_chip >= 0 && ui->drag_chip < ui->chip_count && ui->chips[ui->drag_chip]) {
            ui->chips[ui->drag_chip]->board_x += e->motion.xrel;
            ui->chips[ui->drag_chip]->board_y += e->motion.yrel;
        }
    } else if (e->type == SDL_MOUSEWHEEL) {
        ui->pan_y -= e->wheel.y * 24;
    }
}

static void draw_dip(SDL_Renderer *r, R01nsUi *ui, const R01sEntity *e, int selected) {
    int x = board_sx(ui, e->board_x);
    int y = board_sy(ui, e->board_y);
    int dip = e->dip_pins > 0 ? e->dip_pins : e->pin_count;
    int half = dip / 2;
    int i;
    int pitch = R01S_DIP_PIN_PITCH_PX;
    int margin;
    int row_span;

    if (half < 1) {
        half = 1;
    }
    row_span = (half > 1) ? (half - 1) * pitch : 0;
    margin = (e->body_w - row_span) / 2;
    if (margin < 1) {
        margin = 1;
    }

    for (i = 0; i < e->pin_count; i++) {
        int num = e->pins[i].number;
        int idx;
        int along;
        Uint8 pr, pg, pb;
        int side_pin1;
        if (num < 1 || num > dip) {
            continue;
        }
        side_pin1 = num <= half;
        idx = side_pin1 ? (num - 1) : (dip - num);
        along = margin + idx * pitch;
        pin_rgb(e->pins[i].level, e->pins[i].dir, &pr, &pg, &pb);
        if (side_pin1) {
            fill_rect(r, x + along - 1, y + e->body_h, 3, 4, pr, pg, pb);
        } else {
            fill_rect(r, x + along - 1, y - 4, 3, 4, pr, pg, pb);
        }
    }
    fill_rect(r, x, y, e->body_w, e->body_h, 45, 70, 50);
    if (selected) {
        draw_rect(r, x, y, e->body_w, e->body_h, 255, 220, 80);
    } else {
        draw_rect(r, x, y, e->body_w, e->body_h, 20, 30, 22);
    }
    fill_rect(r, x - 1, y + e->body_h / 2 - 2, 2, 4, 20, 22, 20);
    if (e->part) {
        draw_text(r, x + 2, y + e->body_h / 2 - 3, e->part, 220, 230, 220);
    }
}

static void draw_pwr(SDL_Renderer *r, R01nsUi *ui, const R01sEntity *e, int selected) {
    int x = board_sx(ui, e->board_x);
    int y = board_sy(ui, e->board_y);
    fill_rect(r, x, y, e->body_w, e->body_h, 60, 40, 30);
    draw_rect(r, x, y, e->body_w, e->body_h, selected ? 255 : 140, selected ? 220 : 90, selected ? 80 : 50);
    draw_text(r, x + 2, y + 2, e->part ? e->part : "PWR", 230, 200, 160);
}

static void draw_osc(SDL_Renderer *r, R01nsUi *ui, const R01sEntity *e, int selected) {
    int x = board_sx(ui, e->board_x);
    int y = board_sy(ui, e->board_y);
    fill_rect(r, x, y, e->body_w, e->body_h, 50, 55, 70);
    draw_rect(r, x, y, e->body_w, e->body_h, selected ? 255 : 120, selected ? 220 : 130, selected ? 80 : 180);
    draw_text(r, x + 2, y + 2, "OSC", 180, 200, 255);
}

static void draw_display(SDL_Renderer *r, R01nsUi *ui, R01sEntity *e, int selected) {
    R01nsVideoSink *sink = (R01nsVideoSink *)e;
    int x = board_sx(ui, e->board_x);
    int y = board_sy(ui, e->board_y);
    const uint8_t *rgb = r01ns_video_sink_rgb(sink);
    int box_w = e->body_w > 0 ? e->body_w : R01NS_VIDEO_W / 2;
    int box_h = e->body_h > 0 ? e->body_h : R01NS_VIDEO_H / 2;
    int scale;
    int dw;
    int dh;
    SDL_Rect dst;

    /* Integer fit of 256x192 into the glyph box (1:1 or exact 2:1 downscale, etc.). */
    scale = box_w / R01NS_VIDEO_W;
    if (box_h / R01NS_VIDEO_H < scale) {
        scale = box_h / R01NS_VIDEO_H;
    }
    if (scale >= 1) {
        dw = R01NS_VIDEO_W * scale;
        dh = R01NS_VIDEO_H * scale;
    } else if (R01NS_VIDEO_W % box_w == 0 && R01NS_VIDEO_H % box_h == 0 &&
               (R01NS_VIDEO_W / box_w) == (R01NS_VIDEO_H / box_h)) {
        dw = box_w;
        dh = box_h;
    } else {
        dw = R01NS_VIDEO_W / 2;
        dh = R01NS_VIDEO_H / 2;
    }
    dst.x = x + (box_w - dw) / 2;
    dst.y = y + (box_h - dh) / 2;
    dst.w = dw;
    dst.h = dh;

    if (!ui->lcd_tex) {
        ui->lcd_tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                                        R01NS_VIDEO_W, R01NS_VIDEO_H);
        if (ui->lcd_tex) {
            SDL_SetTextureScaleMode(ui->lcd_tex, SDL_ScaleModeNearest);
        }
    }
    if (ui->lcd_tex && rgb) {
        SDL_UpdateTexture(ui->lcd_tex, NULL, rgb, R01NS_VIDEO_W * 3);
        SDL_RenderCopy(r, ui->lcd_tex, NULL, &dst);
    } else {
        fill_rect(r, x, y, box_w, box_h, 10, 10, 12);
    }
    draw_rect(r, x, y, box_w, box_h, selected ? 255 : 80, selected ? 220 : 90, selected ? 80 : 100);
    draw_text(r, x + 2, y + 2, "SCR1", 200, 210, 220);
}

static void draw_chip(SDL_Renderer *r, R01nsUi *ui, R01sEntity *e, int selected) {
    if (!e) {
        return;
    }
    switch (e->visual) {
    case R01S_ENTITY_VIS_PWR:
        draw_pwr(r, ui, e, selected);
        break;
    case R01S_ENTITY_VIS_OSC:
        draw_osc(r, ui, e, selected);
        break;
    case R01S_ENTITY_VIS_DISPLAY:
        draw_display(r, ui, e, selected);
        break;
    case R01S_ENTITY_VIS_IC:
    default:
        draw_dip(r, ui, e, selected);
        break;
    }
}

static void draw_arcade(SDL_Renderer *r, R01nsUi *ui) {
    int ox = R01S_LOGIC_W - 120;
    int oy = R01S_LOGIC_H - 70;
    uint8_t bits = r01ns_ui_pad_byte(ui);
    fill_rect(r, ox - 4, oy - 4, 116, 62, 18, 22, 20);
    draw_rect(r, ox - 4, oy - 4, 116, 62, 60, 70, 65);
    draw_text(r, ox, oy, "ARCADE P1", 160, 180, 160);
    fill_rect(r, ox + 10, oy + 20, 8, 8, (bits & R01S_PAD_UP) ? 70 : 30, (bits & R01S_PAD_UP) ? 210 : 30,
              90);
    fill_rect(r, ox + 10, oy + 36, 8, 8, (bits & R01S_PAD_DOWN) ? 70 : 30, (bits & R01S_PAD_DOWN) ? 210 : 30,
              90);
    fill_rect(r, ox + 2, oy + 28, 8, 8, (bits & R01S_PAD_LEFT) ? 70 : 30, (bits & R01S_PAD_LEFT) ? 210 : 30,
              90);
    fill_rect(r, ox + 18, oy + 28, 8, 8, (bits & R01S_PAD_RIGHT) ? 70 : 30, (bits & R01S_PAD_RIGHT) ? 210 : 30,
              90);
    fill_rect(r, ox + 50, oy + 24, 12, 12, (bits & R01S_PAD_X) ? 220 : 40, (bits & R01S_PAD_X) ? 80 : 40,
              (bits & R01S_PAD_X) ? 80 : 40);
    draw_text(r, ox + 66, oy + 26, "X/G", 180, 160, 160);
}

void r01ns_ui_draw(R01nsUi *ui, SDL_Renderer *ren) {
    R01sIslandGroup *group;
    int i;

    if (!ui || !ren || !ui->board) {
        return;
    }
    group = r01ns_board_group(ui->board);

    fill_rect(ren, 0, 0, R01S_LOGIC_W, R01S_LOGIC_H, 12, 14, 12);
    fill_rect(ren, 0, 0, R01S_LOGIC_W, R01NS_UI_VIEW_Y, 20, 24, 18);
    draw_text(ren, 8, 8, "RETR01 NANO SIM", 200, 220, 180);
    {
        char hud[96];
        snprintf(hud, sizeof(hud), "FPS:%d STEPS:%d VDD:%s PHI2:%s", ui->fps, ui->sim_steps,
                 ui->probe_vdd ? "H" : "L", ui->probe_phi2 ? "H" : "L");
        draw_text(ren, 200, 8, hud, 140, 160, 140);
    }

    /* Clip-ish: draw islands then chips. */
    if (group) {
        for (i = 0; i < group->island_count; i++) {
            const R01sIsland *isl = group->islands[i];
            int x, y;
            if (!isl) {
                continue;
            }
            x = board_sx(ui, isl->board_x);
            y = board_sy(ui, isl->board_y);
            fill_rect(ren, x, y, isl->board_w, isl->board_h, 22, 28, 24);
            draw_rect(ren, x, y, isl->board_w, isl->board_h, 70, 90, 75);
            draw_text(ren, x + 4, y + 4, isl->title ? isl->title : "?", 160, 190, 150);
        }
    }

    for (i = 0; i < ui->chip_count; i++) {
        draw_chip(ren, ui, ui->chips[i], i == ui->selected);
    }

    fill_rect(ren, 0, R01S_LOGIC_H - 24, R01S_LOGIC_W, 24, 20, 24, 18);
    draw_text(ren, 8, R01S_LOGIC_H - 16, ui->status, 180, 190, 170);
    draw_arcade(ren, ui);
}
