#include "retr01_nano_emu/machine.h"
#include "r01_play_anim.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : R01NE_DEFAULT_CART;
    R01neMachine m;
    char err[128];
    int tx0, ty0, px0;
    int blocked = 0;
    int i;

    memset(&m, 0, sizeof(m));
    if (r01ne_machine_boot(&m, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "boot: %s\n", err);
        return 1;
    }
    if (!m.play.enabled) {
        fprintf(stderr, "play not enabled\n");
        return 1;
    }
    tx0 = m.play.player_tx;
    ty0 = m.play.player_ty;
    px0 = m.play.player_px;
    if (px0 != tx0 * 8 || m.play.player_py != ty0 * 8) {
        fprintf(stderr, "expected rest at local (0,0), got px=%d py=%d tx=%d ty=%d\n", m.play.player_px,
                m.play.player_py, tx0, ty0);
        return 1;
    }

    /* Press/hold right: 1 px/frame (no tile jump). */
    r01ne_play_set_pad(&m, R01NE_PAD_RIGHT);
    r01ne_machine_frame(&m);
    if (m.play.player_px != px0 + 1 || m.play.player_py != m.play.player_ty * 8) {
        fprintf(stderr, "expected +1 px on first frame, got px=%d (from %d)\n", m.play.player_px, px0);
        return 1;
    }
    if (m.play.player_tx != tx0) {
        fprintf(stderr, "tile should not change after 1 px, tx=%d\n", m.play.player_tx);
        return 1;
    }
    if (!r01_play_anim_moving(&m.play.anim) || r01_play_anim_entity_state(&m.play.anim) != 1) {
        fprintf(stderr, "expected slide_x state while moving right\n");
        return 1;
    }

    for (i = 0; i < 3; i++) {
        int px = m.play.player_px;
        r01ne_machine_frame(&m);
        if (m.play.player_px != px + 1) {
            fprintf(stderr, "expected +1 px at step %d (px %d -> %d)\n", i, px, m.play.player_px);
            return 1;
        }
    }
    if (m.play.player_px != px0 + 4) {
        fprintf(stderr, "expected px=%d after 4 frames, got %d\n", px0 + 4, m.play.player_px);
        return 1;
    }
    /* Sprite draws at pixel origin (not tile*8). */
    {
        int ox = m.video.screen_col * R01NE_SCREEN_PX_W;
        int oy = m.video.screen_row * R01NE_SCREEN_PX_H;
        int lx = m.play.player_px - ox;
        int ly = m.play.player_py - oy;
        int tile_lx = m.play.player_tx * 8 - ox;
        int sy, sx, hit = 0;
        r01ne_video_render_frame(&m);
        for (sy = 0; sy < 8 && !hit; sy++) {
            for (sx = 0; sx < 8; sx++) {
                const uint8_t *p =
                    m.video.logical + ((size_t)(ly + sy) * R01NE_SCREEN_PX_W + (size_t)(lx + sx)) * 3u;
                if (p[0] | p[1] | p[2]) {
                    hit = 1;
                    break;
                }
            }
        }
        if (!hit) {
            fprintf(stderr, "expected player sprite pixels at px origin (%d,%d)\n", lx, ly);
            return 1;
        }
        if (lx == tile_lx) {
            fprintf(stderr, "expected mid-tile sprite offset, lx=%d tile_lx=%d\n", lx, tile_lx);
            return 1;
        }
    }
    {
        int px = m.play.player_px;
        int tx = m.play.player_tx;
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (m.play.player_tx != tx || m.play.player_px != px) {
            fprintf(stderr, "expected no release snap, tx=%d px=%d (was %d)\n", m.play.player_tx,
                    m.play.player_px, px);
            return 1;
        }
    }
    if (r01_play_anim_moving(&m.play.anim)) {
        fprintf(stderr, "expected not-moving after release\n");
        return 1;
    }
    if (r01_play_anim_entity_state(&m.play.anim) != 0) {
        fprintf(stderr, "expected slide_x release to return to idle, got %d\n",
                r01_play_anim_entity_state(&m.play.anim));
        return 1;
    }

    /* Left: 1 px, stay mid-tile on release. */
    {
        int tx = m.play.player_tx;
        int ty = m.play.player_ty;
        int px;
        m.play.player_px = tx * 8 + 4;
        m.play.player_py = ty * 8;
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        r01ne_play_set_pad(&m, R01NE_PAD_LEFT);
        r01ne_machine_frame(&m);
        if (m.play.player_px != tx * 8 + 3 || m.play.player_tx != tx) {
            fprintf(stderr, "expected left -1 px mid-tile, tx=%d px=%d\n", m.play.player_tx, m.play.player_px);
            return 1;
        }
        px = m.play.player_px;
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (m.play.player_px != px) {
            fprintf(stderr, "expected left release keep px=%d, got %d\n", px, m.play.player_px);
            return 1;
        }
    }

    /* Cross into next tile after 8 px of continuous walk. */
    {
        int start_tx = m.play.player_tx;
        m.play.player_px = start_tx * 8;
        m.play.player_py = m.play.player_ty * 8;
        r01ne_play_set_pad(&m, R01NE_PAD_RIGHT);
        for (i = 0; i < 8; i++) {
            r01ne_machine_frame(&m);
        }
        if (m.play.player_px != start_tx * 8 + 8 || m.play.player_tx != start_tx + 1) {
            fprintf(stderr, "expected cross into next tile after 8 px, tx=%d px=%d\n", m.play.player_tx,
                    m.play.player_px);
            return 1;
        }
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
    }

    /* Up / down use slide_up (2) and slide_down (3). */
    {
        int ty = m.play.player_ty;
        m.play.player_px = m.play.player_tx * 8;
        m.play.player_py = ty * 8;
        r01ne_play_set_pad(&m, R01NE_PAD_UP);
        r01ne_machine_frame(&m);
        if (r01_play_anim_entity_state(&m.play.anim) != 2) {
            fprintf(stderr, "expected slide_up state, got %d\n", r01_play_anim_entity_state(&m.play.anim));
            return 1;
        }
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (r01_play_anim_entity_state(&m.play.anim) != 2) {
            fprintf(stderr, "expected slide_up kept after release, got %d\n",
                    r01_play_anim_entity_state(&m.play.anim));
            return 1;
        }
        m.play.player_px = m.play.player_tx * 8;
        m.play.player_py = m.play.player_ty * 8;
        r01ne_play_set_pad(&m, R01NE_PAD_DOWN);
        r01ne_machine_frame(&m);
        if (r01_play_anim_entity_state(&m.play.anim) != 3) {
            fprintf(stderr, "expected slide_down state, got %d\n", r01_play_anim_entity_state(&m.play.anim));
            return 1;
        }
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (r01_play_anim_entity_state(&m.play.anim) != 3) {
            fprintf(stderr, "expected slide_down kept after release, got %d\n",
                    r01_play_anim_entity_state(&m.play.anim));
            return 1;
        }
    }

    /* Solid: walk into a solid tile to the right if one exists. */
    {
        int found_solid = 0;
        int sx;
        m.play.player_px = m.play.player_tx * 8;
        m.play.player_py = m.play.player_ty * 8;
        for (sx = (m.play.player_tx + 1) * 8 + 4; sx < m.play.player_tx * 8 + 4 + 64; sx += 8) {
            if (r01ne_cart_solid_at(&m.cart, 0, sx, m.play.player_ty * 8 + 4)) {
                found_solid = 1;
                break;
            }
        }
        if (found_solid) {
            int tx;
            r01ne_play_set_pad(&m, R01NE_PAD_RIGHT);
            for (i = 0; i < 128; i++) {
                tx = m.play.player_tx;
                r01ne_machine_frame(&m);
                if (m.play.player_tx == tx && i > 8) {
                    blocked = 1;
                    break;
                }
            }
            if (!blocked) {
                fprintf(stderr, "expected solid tile block\n");
                return 1;
            }
        }
    }

    /* Lasers: spawn ahead of facing; destroy SOLID → bank0/tile0/attr0; despawn. */
    {
        int di, lx, ly, cell, wx, wy;
        uint8_t *pay;
        int ntx, nty;
        if (m.play.laser_type < 0) {
            fprintf(stderr, "expected laser entity type\n");
            return 1;
        }
        /* Face right, plant a solid one tile ahead, fire. */
        m.play.player_px = m.play.player_tx * 8;
        m.play.player_py = m.play.player_ty * 8;
        m.play.anim.player_anim_dir = R01_PLAYER_DIR_RIGHT;
        m.play.anim.player_anim_flip_h = 0;
        ntx = m.play.player_tx + 1;
        nty = m.play.player_ty;
        wx = ntx * 8 + 4;
        wy = nty * 8 + 4;
        di = r01ne_world_find_screen(&m.cart, &m.world, wx / R01NE_SCREEN_PX_W, wy / R01NE_SCREEN_PX_H);
        if (di < 0) {
            fprintf(stderr, "no screen for laser target\n");
            return 1;
        }
        pay = r01ne_world_screen_payload_mut(&m.cart, &m.world, di);
        if (!pay) {
            fprintf(stderr, "payload mut failed\n");
            return 1;
        }
        lx = (wx % R01NE_SCREEN_PX_W) / 8;
        ly = (wy % R01NE_SCREEN_PX_H) / 8;
        cell = ly * R01NE_SCREEN_TILES_X + lx;
        pay[cell] = 1;
        pay[R01NE_TILES_PER_SCREEN + cell] = (uint8_t)(R01NE_ATTR_SOLID | (2u << R01NE_ATTR_FG_SHIFT));
        if (m.video.map_loaded && m.video.screen_col == wx / R01NE_SCREEN_PX_W &&
            m.video.screen_row == wy / R01NE_SCREEN_PX_H) {
            m.video.map[cell] = pay[cell];
            m.video.map[R01NE_TILES_PER_SCREEN + cell] = pay[R01NE_TILES_PER_SCREEN + cell];
        }
        if (!r01ne_cart_solid_at(&m.cart, 0, wx, wy)) {
            fprintf(stderr, "planted solid not detected\n");
            return 1;
        }
        r01ne_play_set_pad(&m, R01NE_PAD_X);
        r01ne_machine_frame(&m);
        if (m.play.lasers[0].active) {
            fprintf(stderr, "laser should despawn after destroying solid on spawn cell\n");
            return 1;
        }
        if (pay[cell] != 0 || pay[R01NE_TILES_PER_SCREEN + cell] != 0) {
            fprintf(stderr, "expected empty tile after laser hit, tile=%u attr=%u\n", pay[cell],
                    pay[R01NE_TILES_PER_SCREEN + cell]);
            return 1;
        }
        if (r01ne_cart_solid_at(&m.cart, 0, wx, wy)) {
            fprintf(stderr, "solid should clear after laser destroy\n");
            return 1;
        }
        /* Fire into empty space: laser lives one frame then advances / stays until edge. */
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        r01ne_play_set_pad(&m, R01NE_PAD_X);
        r01ne_machine_frame(&m);
        if (!m.play.lasers[0].active || m.play.lasers[0].tx != ntx || m.play.lasers[0].ty != nty ||
            m.play.lasers[0].state != R01NE_LASER_STATE_H) {
            fprintf(stderr, "expected active horizontal laser at (%d,%d) state=%d active=%d\n", ntx, nty,
                    m.play.lasers[0].state, m.play.lasers[0].active);
            return 1;
        }
        /* Vertical facing — reset clears custom_logic laser pool. */
        if (r01ne_machine_reset(&m) != 0) {
            fprintf(stderr, "reset failed\n");
            return 1;
        }
        m.play.anim.player_anim_dir = R01_PLAYER_DIR_UP;
        m.play.anim.player_anim_flip_h = 0;
        r01ne_play_set_pad(&m, R01NE_PAD_X);
        r01ne_machine_frame(&m);
        if (!m.play.lasers[0].active || m.play.lasers[0].state != R01NE_LASER_STATE_V ||
            m.play.lasers[0].ty != m.play.player_ty - 1) {
            fprintf(stderr, "expected vertical laser up, active=%d state=%d ty=%d (player_ty=%d)\n",
                    m.play.lasers[0].active, m.play.lasers[0].state, m.play.lasers[0].ty,
                    m.play.player_ty);
            return 1;
        }
    }

    printf("ok play tile=(%d,%d)->(%d,%d) px=%d->%d blocked=%d laser_type=%d\n", tx0, ty0, m.play.player_tx,
           m.play.player_ty, px0, m.play.player_px, blocked, m.play.laser_type);
    r01ne_machine_shutdown(&m);
    return 0;
}
