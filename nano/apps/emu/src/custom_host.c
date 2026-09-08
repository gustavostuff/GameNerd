/*
 * Nano Emu host for output/nano/C/custom_logic.c.
 * Implements map/soft hooks + the small R01GameCtx surface custom_logic needs.
 */
#include "retr01_nano_emu/custom_host.h"
#include "retr01_nano_emu/machine.h"

#include "r01_engine.h"

#include <string.h>

#define R01_EVENT_SLOTS 8

static R01neMachine *s_m;
static R01GameCtx s_ctx;

typedef struct {
    uint8_t btn;
    R01EventFn fn;
} HostEvent;

static HostEvent s_events[R01_EVENT_SLOTS];

void r01ne_custom_bind(R01neMachine *m) {
    s_m = m;
}

/* --- map / soft (called from custom_logic) --- */

int r01_map_solid_at(int wx, int wy) {
    if (!s_m || wx < 0 || wy < 0) {
        return 0;
    }
    return r01ne_cart_solid_at(&s_m->cart, s_m->world_idx, wx, wy);
}

void r01_map_set_empty(int tx, int ty) {
    int wx, wy, col, row, di, lx, ly, cell;
    uint8_t *pay;
    if (!s_m || tx < 0 || ty < 0) {
        return;
    }
    wx = tx * 8 + 4;
    wy = ty * 8 + 4;
    col = wx / R01NE_SCREEN_PX_W;
    row = wy / R01NE_SCREEN_PX_H;
    di = r01ne_world_find_screen(&s_m->cart, &s_m->world, col, row);
    if (di < 0) {
        return;
    }
    pay = r01ne_world_screen_payload_mut(&s_m->cart, &s_m->world, di);
    if (!pay) {
        return;
    }
    lx = (wx % R01NE_SCREEN_PX_W) / 8;
    ly = (wy % R01NE_SCREEN_PX_H) / 8;
    cell = ly * R01NE_SCREEN_TILES_X + lx;
    pay[cell] = 0;
    pay[R01NE_TILES_PER_SCREEN + cell] = 0;
    if (s_m->video.map_loaded && s_m->video.screen_col == col && s_m->video.screen_row == row) {
        s_m->video.map[cell] = 0;
        s_m->video.map[R01NE_TILES_PER_SCREEN + cell] = 0;
    }
}

int r01_map_tile_in_play(int tx, int ty) {
    int col, row;
    if (!s_m || tx < 0 || ty < 0 || !s_m->video.map_loaded) {
        return 0;
    }
    col = (tx * 8) / R01NE_SCREEN_PX_W;
    row = (ty * 8) / R01NE_SCREEN_PX_H;
    if (col != s_m->video.screen_col || row != s_m->video.screen_row) {
        return 0;
    }
    return r01ne_cart_has_screen(&s_m->cart, s_m->world_idx, col, row);
}

void r01_soft_laser_clear(void) {
    if (!s_m) {
        return;
    }
    memset(s_m->play.lasers, 0, sizeof(s_m->play.lasers));
}

void r01_soft_laser_add(int state, int tx, int ty, int flip_h, int fg) {
    int i;
    if (!s_m) {
        return;
    }
    for (i = 0; i < R01NE_LASERS_MAX; i++) {
        R01neLaser *L = &s_m->play.lasers[i];
        if (L->active) {
            continue;
        }
        L->active = 1;
        L->tx = tx;
        L->ty = ty;
        L->state = state;
        L->flip_h = flip_h;
        L->fg = fg;
        L->dtx = 0;
        L->dty = 0;
        return;
    }
}

/* --- pad / events --- */

uint8_t r01_pad_pressed(const R01GameCtx *ctx, uint8_t btn) {
    if (!ctx) {
        return 0;
    }
    return (uint8_t)((ctx->pad >> btn) & 1u);
}

uint8_t r01_pad_just_pressed(R01GameCtx *ctx, uint8_t btn) {
    if (!ctx) {
        return 0;
    }
    return (uint8_t)(((ctx->pad ^ ctx->pad_prev) & ctx->pad) >> btn) & 1u;
}

int r01_event_on_button(uint8_t btn, R01EventFn fn) {
    int i;
    if (!fn) {
        return -1;
    }
    for (i = 0; i < R01_EVENT_SLOTS; i++) {
        if (!s_events[i].fn) {
            s_events[i].btn = btn;
            s_events[i].fn = fn;
            return i;
        }
    }
    return -1;
}

void r01_runtime_dispatch_buttons(R01GameCtx *ctx) {
    int i;
    if (!ctx) {
        return;
    }
    for (i = 0; i < R01_EVENT_SLOTS; i++) {
        if (s_events[i].fn && r01_pad_just_pressed(ctx, s_events[i].btn)) {
            s_events[i].fn(ctx);
        }
    }
}

/* --- anim policy setters (write R01GameCtx; Host Play copies on start) --- */

static void pa_apply_idle_facing(R01GameCtx *ctx) {
    int dir = R01_PLAYER_DIR_RIGHT;
    switch (ctx->player_default_face) {
    case R01_PLAYER_FACE_DOWN:
        dir = R01_PLAYER_DIR_DOWN;
        break;
    case R01_PLAYER_FACE_LEFT:
        dir = R01_PLAYER_DIR_LEFT;
        break;
    case R01_PLAYER_FACE_UP:
        dir = R01_PLAYER_DIR_UP;
        break;
    default:
        dir = R01_PLAYER_DIR_RIGHT;
        break;
    }
    ctx->player_anim_dir = dir;
    ctx->player_anim_flip_h =
        (dir == R01_PLAYER_DIR_LEFT || dir == R01_PLAYER_DIR_UP_LEFT || dir == R01_PLAYER_DIR_DOWN_LEFT);
}

void r01_player_anim_set_idle_state(R01GameCtx *ctx, int entity_state_idx) {
    if (!ctx || entity_state_idx < 0 || entity_state_idx >= 4) {
        return;
    }
    ctx->player_idle_state = entity_state_idx;
    if (!ctx->player_anim_moving) {
        ctx->player_anim_state = entity_state_idx;
        ctx->player_anim_frame = 0;
        ctx->player_anim_ctr = 0;
    }
}

void r01_player_anim_set_walk_state(R01GameCtx *ctx, int dir8, int entity_state_idx) {
    if (!ctx || dir8 < 0 || dir8 > 7 || entity_state_idx < 0 || entity_state_idx >= 4) {
        return;
    }
    ctx->player_walk_state[dir8] = entity_state_idx;
}

void r01_player_anim_set_walk_all(R01GameCtx *ctx, int entity_state_idx) {
    int i;
    if (!ctx || entity_state_idx < 0 || entity_state_idx >= 4) {
        return;
    }
    for (i = 0; i < 8; i++) {
        ctx->player_walk_state[i] = entity_state_idx;
    }
}

void r01_player_anim_set_release_to_idle(R01GameCtx *ctx, int entity_state_idx, int enable) {
    if (!ctx || entity_state_idx < 0 || entity_state_idx >= 4) {
        return;
    }
    ctx->player_release_to_idle[entity_state_idx] = enable ? 1 : 0;
}

void r01_player_default_face_set(R01GameCtx *ctx, int face) {
    if (!ctx) {
        return;
    }
    if (face < R01_PLAYER_FACE_RIGHT || face > R01_PLAYER_FACE_UP) {
        face = R01_PLAYER_FACE_RIGHT;
    }
    ctx->player_default_face = face;
    if (!ctx->player_anim_moving) {
        pa_apply_idle_facing(ctx);
    }
}

void r01_entity_state_frame_delay_set(R01GameCtx *ctx, int entity_state_idx, int ticks) {
    if (!ctx || entity_state_idx < 0 || entity_state_idx >= 4) {
        return;
    }
    if (ticks < 1) {
        ticks = 1;
    }
    ctx->player_state_delay[entity_state_idx] = ticks;
}

static void apply_pose_to_play(R01nePlay *pl, const R01GameCtx *ctx) {
    int i;
    if (!pl || !ctx) {
        return;
    }
    r01_play_anim_set_idle_state(&pl->anim, ctx->player_idle_state);
    for (i = 0; i < 8; i++) {
        r01_play_anim_set_walk_state(&pl->anim, i, ctx->player_walk_state[i]);
    }
    for (i = 0; i < 4; i++) {
        r01_play_state_frame_delay_set(&pl->anim, i, ctx->player_state_delay[i]);
        r01_play_anim_set_release_to_idle(&pl->anim, i, ctx->player_release_to_idle[i]);
    }
    r01_play_default_face_set(&pl->anim, ctx->player_default_face);
}

static uint8_t face_pad_from_hw(uint8_t hw) {
    uint8_t p = 0;
    /* Export API: R01_BTN_X/Y are bit indices 0/1, not hardware pad bits. */
    if (hw & R01NE_PAD_X) {
        p |= (uint8_t)(1u << R01_BTN_X);
    }
    if (hw & R01NE_PAD_Y) {
        p |= (uint8_t)(1u << R01_BTN_Y);
    }
    return p;
}

static void sync_ctx_from_play(R01neMachine *m) {
    R01nePlay *pl = &m->play;
    s_ctx.player_x = pl->player_tx * 8;
    s_ctx.player_y = pl->player_ty * 8;
    s_ctx.player_anim_dir = pl->anim.player_anim_dir;
    s_ctx.player_anim_flip_h = pl->anim.player_anim_flip_h;
    s_ctx.player_anim_state = pl->anim.player_anim_state;
    s_ctx.player_anim_moving = pl->anim.player_anim_moving;
    s_ctx.pad = face_pad_from_hw(pl->pad0);
}

void r01ne_custom_start(R01neMachine *m) {
    int i;
    if (!m) {
        return;
    }
    r01ne_custom_bind(m);
    memset(&s_ctx, 0, sizeof(s_ctx));
    memset(s_events, 0, sizeof(s_events));
    s_ctx.player_idle_state = 0;
    for (i = 0; i < 8; i++) {
        s_ctx.player_walk_state[i] = 1;
    }
    for (i = 0; i < 4; i++) {
        s_ctx.player_state_delay[i] = 6;
    }
    r01_custom_on_init(&s_ctx);
    apply_pose_to_play(&m->play, &s_ctx);
    s_ctx.pad_prev = 0;
}

void r01ne_custom_frame(R01neMachine *m) {
    if (!m || !m->play.enabled) {
        return;
    }
    r01ne_custom_bind(m);
    sync_ctx_from_play(m);
    r01_runtime_dispatch_buttons(&s_ctx);
    r01_custom_on_tick(&s_ctx);
    s_ctx.pad_prev = s_ctx.pad;
}
