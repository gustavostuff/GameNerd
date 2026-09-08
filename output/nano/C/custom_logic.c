/* User game logic — created once by Studio export; never overwritten. */
#include "include/r01_engine.h"
#include "include/r01_map.h"

/* Player entity states (Studio names): idle, slide_x, slide_up, slide_down */
enum {
    PLAYER_STATE_IDLE = 0,
    PLAYER_STATE_SLIDE_X = 1,
    PLAYER_STATE_SLIDE_UP = 2,
    PLAYER_STATE_SLIDE_DOWN = 3
};

/* Laser entity (Studio name "laser"): horizontal=0, vertical=1 */
enum {
    ENTITY_LASER = 1,
    LASER_STATE_H = 0,
    LASER_STATE_V = 1,
    LASERS_MAX = 8
};

typedef struct {
    int active;
    int tx, ty;
    int dtx, dty;
    int state;
    int flip_h;
    int just_spawned; /* skip move on the spawn frame so Host can show the beam */
} Laser;

static Laser s_lasers[LASERS_MAX];

/* Cardinal facing from anim dir (diagonals follow slide_x → left/right). */
static void face_from_anim(const R01GameCtx *ctx, int *out_dtx, int *out_dty, int *out_horiz,
                           int *out_flip_h) {
    int dir = ctx ? ctx->player_anim_dir : R01_PLAYER_DIR_RIGHT;
    int dtx = 1;
    int dty = 0;
    int horiz = 1;
    int flip = 0;
    switch (dir) {
    case R01_PLAYER_DIR_LEFT:
    case R01_PLAYER_DIR_UP_LEFT:
    case R01_PLAYER_DIR_DOWN_LEFT:
        dtx = -1;
        dty = 0;
        horiz = 1;
        flip = 1;
        break;
    case R01_PLAYER_DIR_UP:
        dtx = 0;
        dty = -1;
        horiz = 0;
        flip = 0;
        break;
    case R01_PLAYER_DIR_DOWN:
        dtx = 0;
        dty = 1;
        horiz = 0;
        flip = 0;
        break;
    case R01_PLAYER_DIR_RIGHT:
    case R01_PLAYER_DIR_UP_RIGHT:
    case R01_PLAYER_DIR_DOWN_RIGHT:
    default:
        dtx = 1;
        dty = 0;
        horiz = 1;
        flip = 0;
        break;
    }
    if (out_dtx) {
        *out_dtx = dtx;
    }
    if (out_dty) {
        *out_dty = dty;
    }
    if (out_horiz) {
        *out_horiz = horiz;
    }
    if (out_flip_h) {
        *out_flip_h = flip;
    }
}

static int destroy_solid_at_tile(int tx, int ty) {
    int wx = tx * 8 + 4;
    int wy = ty * 8 + 4;
    if (tx < 0 || ty < 0) {
        return 0;
    }
    if (!r01_map_solid_at(wx, wy)) {
        return 0;
    }
    r01_map_set_empty(tx, ty);
    return 1;
}

static void laser_fire(R01GameCtx *ctx) {
    int slot = -1;
    int i;
    int dtx, dty, horiz, flip;
    int ntx, nty;
    Laser *L;
    int ptx, pty;
    if (!ctx) {
        return;
    }
    for (i = 0; i < LASERS_MAX; i++) {
        if (!s_lasers[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        return;
    }
    face_from_anim(ctx, &dtx, &dty, &horiz, &flip);
    ptx = ctx->player_x / 8;
    pty = ctx->player_y / 8;
    ntx = ptx + dtx;
    nty = pty + dty;
    if (ntx < 0 || nty < 0 || !r01_map_tile_in_play(ntx, nty)) {
        return;
    }
    L = &s_lasers[slot];
    L->active = 1;
    L->tx = ntx;
    L->ty = nty;
    L->dtx = dtx;
    L->dty = dty;
    L->state = horiz ? LASER_STATE_H : LASER_STATE_V;
    L->flip_h = flip;
    L->just_spawned = 1;
    if (destroy_solid_at_tile(L->tx, L->ty)) {
        L->active = 0;
    }
    (void)ENTITY_LASER;
}

static void lasers_tick(R01GameCtx *ctx) {
    int i;
    (void)ctx;
    r01_soft_laser_clear();
    for (i = 0; i < LASERS_MAX; i++) {
        Laser *L = &s_lasers[i];
        int ntx, nty;
        if (!L->active) {
            continue;
        }
        if (L->just_spawned) {
            L->just_spawned = 0;
        } else {
            if (destroy_solid_at_tile(L->tx, L->ty)) {
                L->active = 0;
                continue;
            }
            ntx = L->tx + L->dtx;
            nty = L->ty + L->dty;
            if (ntx < 0 || nty < 0 || !r01_map_tile_in_play(ntx, nty)) {
                L->active = 0;
                continue;
            }
            L->tx = ntx;
            L->ty = nty;
            if (destroy_solid_at_tile(L->tx, L->ty)) {
                L->active = 0;
                continue;
            }
        }
        r01_soft_laser_add(L->state, L->tx, L->ty, L->flip_h, 1);
    }
}

static void on_fire(R01GameCtx *ctx) {
    laser_fire(ctx);
}

void r01_custom_on_init(R01GameCtx *ctx) {
    int i;
    /*
     * Pose policy lives here. Runtime/base_game only draw the active state index
     * (player_anim_state) — they do not decide idle vs slide themselves.
     */
    for (i = 0; i < LASERS_MAX; i++) {
        s_lasers[i].active = 0;
    }
    r01_player_anim_set_idle_state(ctx, PLAYER_STATE_IDLE);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP, PLAYER_STATE_SLIDE_UP);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN, PLAYER_STATE_SLIDE_DOWN);
    /* slide_x → idle on release; slide_up / slide_down keep their pose. */
    r01_player_anim_set_release_to_idle(ctx, PLAYER_STATE_SLIDE_X, 1);
    r01_player_default_face_set(ctx, R01_PLAYER_FACE_RIGHT);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_IDLE, 10);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_X, 4);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_UP, 4);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_DOWN, 4);
    /* Nano: no camera scroll — screen switches are instant. */
    r01_event_on_button(R01_BTN_X, on_fire);
}

void r01_custom_on_tick(R01GameCtx *ctx) {
    lasers_tick(ctx);
}

void r01_custom_on_vblank(R01GameCtx *ctx) {
    (void)ctx;
}
