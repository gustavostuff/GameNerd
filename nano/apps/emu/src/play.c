#include "retr01_nano_emu/play.h"
#include "retr01_nano_emu/machine.h"
#include "retr01_nano_emu/custom_host.h"

#include <string.h>

void r01ne_play_reset(R01nePlay *pl) {
    if (!pl) {
        return;
    }
    memset(pl, 0, sizeof(*pl));
    pl->player_type = -1;
    pl->player_fg = 0;
    pl->laser_type = -1;
    pl->move_strategy = R01NE_MOVE_PIXEL_CONTINUOUS;
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

static int local_ox(const R01nePlay *pl) {
    return pl->player_px - pl->player_tx * 8;
}

static int local_oy(const R01nePlay *pl) {
    return pl->player_py - pl->player_ty * 8;
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

static int find_laser_type(const R01neMachine *m) {
    int i;
    if (!m) {
        return -1;
    }
    if (m->play.player_type < 0) {
        return (m->world.entity_type_count > 1) ? 1 : -1;
    }
    for (i = 0; i < m->world.entity_type_count; i++) {
        if (i != m->play.player_type) {
            return i;
        }
    }
    return -1;
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

static int try_enter_tile(R01neMachine *m, int ntx, int nty, int entry_ox, int entry_oy) {
    R01nePlay *pl;
    if (!m) {
        return 0;
    }
    if (!tile_enter_ok(m, ntx, nty)) {
        return 0;
    }
    pl = &m->play;
    pl->player_tx = ntx;
    pl->player_ty = nty;
    pl->player_px = ntx * 8 + entry_ox;
    pl->player_py = nty * 8 + entry_oy;
    return 1;
}

/*
 * PIXEL_CONTINUOUS (default for sprite entities):
 * Each held frame moves +/-1 px. Crossing a tile boundary requires the
 * destination MAP cell to be enterable (present + not SOLID). No press jump
 * and no release snap back to the tile origin.
 */
static void try_move_axis_pixel_continuous(R01neMachine *m, int dpx, int dpy) {
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
            return;
        }
    }
    pl->player_px = nx;
    pl->player_py = ny;
    sync_tiles_from_pixels(pl);
}

/*
 * Legacy TILE_ENTER_PIXEL:
 * From tile rest (local 0,0) on a fresh direction press: immediately enter the
 * adjacent tile at the direction entry pixel (R:0,0 L:7,0 U:0,7 D:0,0).
 * While held afterward: 1 px/frame. Release snaps that axis to local 0.
 */
static void try_move_axis_tile_enter_pixel(R01neMachine *m, int dpx, int dpy, int pressed) {
    R01nePlay *pl;
    int nx, ny, ntx, nty;
    int entry_ox, entry_oy;
    if (!m || (dpx == 0 && dpy == 0)) {
        return;
    }
    pl = &m->play;

    if (pressed && local_ox(pl) == 0 && local_oy(pl) == 0) {
        if (dpx > 0) {
            try_enter_tile(m, pl->player_tx + 1, pl->player_ty, 0, 0);
        } else if (dpx < 0) {
            try_enter_tile(m, pl->player_tx - 1, pl->player_ty, 7, 0);
        } else if (dpy > 0) {
            try_enter_tile(m, pl->player_tx, pl->player_ty + 1, 0, 0);
        } else if (dpy < 0) {
            try_enter_tile(m, pl->player_tx, pl->player_ty - 1, 0, 7);
        }
        return;
    }

    nx = pl->player_px + dpx;
    ny = pl->player_py + dpy;
    if (nx < 0 || ny < 0) {
        return;
    }
    ntx = nx / 8;
    nty = ny / 8;
    if (ntx == pl->player_tx && nty == pl->player_ty) {
        pl->player_px = nx;
        pl->player_py = ny;
        return;
    }

    entry_ox = 0;
    entry_oy = 0;
    if (dpx > 0) {
        entry_ox = 0;
        entry_oy = local_oy(pl);
        ntx = pl->player_tx + 1;
        nty = pl->player_ty;
    } else if (dpx < 0) {
        entry_ox = 7;
        entry_oy = local_oy(pl);
        ntx = pl->player_tx - 1;
        nty = pl->player_ty;
    } else if (dpy > 0) {
        entry_ox = local_ox(pl);
        entry_oy = 0;
        ntx = pl->player_tx;
        nty = pl->player_ty + 1;
    } else if (dpy < 0) {
        entry_ox = local_ox(pl);
        entry_oy = 7;
        ntx = pl->player_tx;
        nty = pl->player_ty - 1;
    }
    try_enter_tile(m, ntx, nty, entry_ox, entry_oy);
}

static void try_move_axis(R01neMachine *m, int dpx, int dpy, int pressed) {
    if (!m) {
        return;
    }
    switch (m->play.move_strategy) {
    case R01NE_MOVE_TILE_ENTER_PIXEL:
        try_move_axis_tile_enter_pixel(m, dpx, dpy, pressed);
        break;
    case R01NE_MOVE_PIXEL_CONTINUOUS:
    default:
        (void)pressed;
        try_move_axis_pixel_continuous(m, dpx, dpy);
        break;
    }
}

int r01ne_play_start(R01neMachine *m) {
    int sx, sy, type, fg;
    if (!m || !m->booted) {
        return 0;
    }
    r01ne_play_reset(&m->play);
    /* Pose policy + lasers: output/nano/C/custom_logic.c via Host bridge. */
    if (player_instance_spawn(m, &type, &fg, &sx, &sy)) {
        m->play.player_type = type;
        m->play.player_fg = fg;
        /* Studio instances are tile-placed. Snap spawn to tile origin; walk uses pixels. */
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
    m->play.laser_type = find_laser_type(m);
    m->play.enabled = 1;
    r01ne_custom_start(m);
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
    cx = m->play.player_px + 4;
    cy = m->play.player_py + 4;
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
    uint8_t pressed;
    uint8_t released;
    int dx = 0;
    int dy = 0;
    int had_h;
    int had_v;
    int has_h;
    int has_v;
    if (!m || !m->play.enabled) {
        return;
    }
    pl = &m->play;
    pad = pl->pad0;
    pressed = (uint8_t)(pad & (uint8_t)~pl->pad_prev);
    released = (uint8_t)(pl->pad_prev & (uint8_t)~pad);

    had_h = (pl->pad_prev & (R01NE_PAD_LEFT | R01NE_PAD_RIGHT)) != 0;
    had_v = (pl->pad_prev & (R01NE_PAD_UP | R01NE_PAD_DOWN)) != 0;
    has_h = (pad & (R01NE_PAD_LEFT | R01NE_PAD_RIGHT)) != 0;
    has_v = (pad & (R01NE_PAD_UP | R01NE_PAD_DOWN)) != 0;

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

    if (dx != 0) {
        int edge = (dx < 0) ? (pressed & R01NE_PAD_LEFT) : (pressed & R01NE_PAD_RIGHT);
        try_move_axis(m, dx, 0, edge != 0);
    }
    if (dy != 0) {
        int edge = (dy < 0) ? (pressed & R01NE_PAD_UP) : (pressed & R01NE_PAD_DOWN);
        try_move_axis(m, 0, dy, edge != 0);
    }

    /* Release an axis → snap that axis to local 0 in the current tile. */
    if (pl->move_strategy == R01NE_MOVE_TILE_ENTER_PIXEL) {
        if (had_h && !has_h && (released & (R01NE_PAD_LEFT | R01NE_PAD_RIGHT))) {
            pl->player_px = pl->player_tx * 8;
        }
        if (had_v && !has_v && (released & (R01NE_PAD_UP | R01NE_PAD_DOWN))) {
            pl->player_py = pl->player_ty * 8;
        }
    }

    /* Game logic (lasers, etc.) — output/nano/C/custom_logic.c */
    r01ne_custom_frame(m);

    pl->pad_prev = pad;
    r01ne_play_sync_screen(m);
}
