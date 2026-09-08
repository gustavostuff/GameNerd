#include "retr01_nano_emu/play.h"
#include "retr01_nano_emu/machine.h"

#include <string.h>

void r01ne_play_reset(R01nePlay *pl) {
    if (!pl) {
        return;
    }
    memset(pl, 0, sizeof(*pl));
    pl->player_type = -1;
    pl->player_fg = 0;
    r01_play_anim_init(&pl->anim);
}

static int read_u16(const uint8_t *p) {
    return (int)p[0] | ((int)p[1] << 8);
}

static void sync_tiles_from_pixels(R01nePlay *pl) {
    if (!pl) {
        return;
    }
    pl->player_tx = pl->player_px / 8;
    pl->player_ty = pl->player_py / 8;
}

static int player_instance_spawn(R01neMachine *m, int *out_type, int *out_fg, int *out_x, int *out_y) {
    const R01neWorldView *w = &m->world;
    const uint8_t *insts;
    int pe;
    int i;
    if (!w->present || w->entity_inst_count < 1) {
        return 0;
    }
    insts = r01ne_world_ptr(&m->cart, w, w->off_entity_insts,
                            (size_t)w->entity_inst_count * R01NE_INSTANCE_SIZE);
    if (!insts) {
        return 0;
    }
    pe = (w->player_entity == R01NE_CART_PLAYER_ENTITY_NONE) ? -1 : (int)w->player_entity;
    /* Prefer marked player; else first instance (early Nano carts often omit the flag). */
    for (i = 0; i < w->entity_inst_count; i++) {
        const uint8_t *rec = insts + (size_t)i * R01NE_INSTANCE_SIZE;
        int type_id = rec[0];
        if (pe >= 0 && type_id != pe) {
            continue;
        }
        if (out_type) {
            *out_type = type_id;
        }
        if (out_fg) {
            *out_fg = rec[1] & 7;
        }
        if (out_x) {
            *out_x = read_u16(rec + 4);
        }
        if (out_y) {
            *out_y = read_u16(rec + 6);
        }
        return 1;
    }
    return 0;
}

/* Destination tile must sit on a present screen and not be MAP SOLID. */
static int tile_enter_ok(R01neMachine *m, int tx, int ty) {
    int wx;
    int wy;
    int col;
    int row;
    if (tx < 0 || ty < 0) {
        return 0;
    }
    wx = tx * 8 + 4;
    wy = ty * 8 + 4;
    col = wx / R01NE_SCREEN_PX_W;
    row = wy / R01NE_SCREEN_PX_H;
    if (!r01ne_cart_has_screen(&m->cart, m->world_idx, col, row)) {
        return 0;
    }
    if (r01ne_cart_solid_at(&m->cart, m->world_idx, wx, wy)) {
        return 0;
    }
    return 1;
}

/*
 * Integrate one axis in pixels. Visual tile updates only when the pixel path
 * crosses a tile boundary — and only if that tile is enterable.
 */
static void try_move_axis(R01neMachine *m, int dpx, int dpy) {
    R01nePlay *pl;
    int nx, ny, ntx, nty;
    if (!m || (dpx == 0 && dpy == 0)) {
        return;
    }
    pl = &m->play;
    nx = pl->player_px + dpx;
    ny = pl->player_py + dpy;
    if (nx < 0 || ny < 0) {
        return;
    }
    ntx = nx / 8;
    nty = ny / 8;
    if (ntx != pl->player_tx || nty != pl->player_ty) {
        if (!tile_enter_ok(m, ntx, nty)) {
            /* Stay in current tile; park on the edge facing the block. */
            if (dpx > 0) {
                pl->player_px = pl->player_tx * 8 + 7;
            } else if (dpx < 0) {
                pl->player_px = pl->player_tx * 8;
            }
            if (dpy > 0) {
                pl->player_py = pl->player_ty * 8 + 7;
            } else if (dpy < 0) {
                pl->player_py = pl->player_ty * 8;
            }
            return;
        }
    }
    pl->player_px = nx;
    pl->player_py = ny;
    pl->player_tx = ntx;
    pl->player_ty = nty;
}

int r01ne_play_start(R01neMachine *m) {
    int sx, sy, type, fg;
    if (!m || !m->booted) {
        return 0;
    }
    r01ne_play_reset(&m->play);
    /* Defaults match exported custom_logic: Idle=0, Walk=1. */
    r01_play_anim_set_idle_state(&m->play.anim, 0);
    r01_play_anim_set_walk_all(&m->play.anim, 1);
    if (player_instance_spawn(m, &type, &fg, &sx, &sy)) {
        m->play.player_type = type;
        m->play.player_fg = fg;
        /* Authoring places on tile origins; keep pixels tile-aligned at spawn. */
        m->play.player_px = (sx / 8) * 8;
        m->play.player_py = (sy / 8) * 8;
    } else {
        m->play.player_type = -1;
        m->play.player_px = m->world.spawn_col * R01NE_SCREEN_PX_W + (R01NE_SCREEN_PX_W - R01NE_PLAY_PLAYER_W) / 2;
        m->play.player_py = m->world.spawn_row * R01NE_SCREEN_PX_H + (R01NE_SCREEN_PX_H - R01NE_PLAY_PLAYER_H) / 2;
        m->play.player_px = (m->play.player_px / 8) * 8;
        m->play.player_py = (m->play.player_py / 8) * 8;
    }
    sync_tiles_from_pixels(&m->play);
    m->play.enabled = 1;
    r01ne_play_sync_screen(m);
    return 1;
}

void r01ne_play_set_pad(R01neMachine *m, uint8_t pad0) {
    if (!m) {
        return;
    }
    m->play.pad0 = pad0;
}

void r01ne_play_sync_screen(R01neMachine *m) {
    int col, row;
    int cx, cy;
    if (!m || !m->play.enabled) {
        return;
    }
    cx = m->play.player_tx * 8 + 4;
    cy = m->play.player_ty * 8 + 4;
    col = cx / R01NE_SCREEN_PX_W;
    row = cy / R01NE_SCREEN_PX_H;
    if (col == m->video.screen_col && row == m->video.screen_row && m->video.map_loaded) {
        return;
    }
    if (r01ne_video_load_screen(m, &m->world, col, row) != 0) {
        /* Keep previous MAP if the destination cell is missing. */
        return;
    }
}

void r01ne_play_tick(R01neMachine *m) {
    R01nePlay *pl;
    uint8_t pad;
    int dx = 0;
    int dy = 0;
    if (!m || !m->play.enabled) {
        return;
    }
    pl = &m->play;
    pad = pl->pad0;
    pl->pad_prev = pad;

    if (pad & R01NE_PAD_LEFT) {
        dx = -1;
    } else if (pad & R01NE_PAD_RIGHT) {
        dx = 1;
    }
    if (pad & R01NE_PAD_UP) {
        dy = -1;
    } else if (pad & R01NE_PAD_DOWN) {
        dy = 1;
    }

    r01_play_anim_update(&pl->anim, dx, dy);

    /* Axis-separated pixel integration (speed); tile/visual updates on boundaries. */
    if (dx != 0) {
        try_move_axis(m, dx, 0);
    }
    if (dy != 0) {
        try_move_axis(m, 0, dy);
    }

    r01ne_play_sync_screen(m);
}
