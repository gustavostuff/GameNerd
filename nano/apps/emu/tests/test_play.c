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

    /* Press right from rest: immediate enter of next tile at local (0,0). */
    r01ne_play_set_pad(&m, R01NE_PAD_RIGHT);
    r01ne_machine_frame(&m);
    if (m.play.player_tx != tx0 + 1 || m.play.player_ty != ty0) {
        fprintf(stderr, "expected immediate tile +1 on press, got tx=%d (from %d) px=%d\n", m.play.player_tx,
                tx0, m.play.player_px);
        return 1;
    }
    if (m.play.player_px != m.play.player_tx * 8 || m.play.player_py != m.play.player_ty * 8) {
        fprintf(stderr, "expected entry at local (0,0), got px=%d py=%d\n", m.play.player_px, m.play.player_py);
        return 1;
    }
    if (!r01_play_anim_moving(&m.play.anim) || r01_play_anim_entity_state(&m.play.anim) != 1) {
        fprintf(stderr, "expected slide_x state while moving right\n");
        return 1;
    }

    /* Hold: 1 px/frame a few steps, then release → snap X to local 0 (same tile). */
    for (i = 0; i < 3; i++) {
        int px = m.play.player_px;
        int tx = m.play.player_tx;
        r01ne_machine_frame(&m);
        if (m.play.player_tx != tx) {
            fprintf(stderr, "tile changed early during pixel walk at step %d\n", i);
            return 1;
        }
        if (m.play.player_px != px + 1) {
            fprintf(stderr, "expected +1 px at step %d (px %d -> %d)\n", i, px, m.play.player_px);
            return 1;
        }
    }
    if (m.play.player_px != m.play.player_tx * 8 + 3) {
        fprintf(stderr, "expected local x=3 before release, px=%d\n", m.play.player_px);
        return 1;
    }
    {
        int tx = m.play.player_tx;
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (m.play.player_tx != tx || m.play.player_px != tx * 8) {
            fprintf(stderr, "expected release snap to local x=0, tx=%d px=%d\n", m.play.player_tx,
                    m.play.player_px);
            return 1;
        }
    }
    if (r01_play_anim_moving(&m.play.anim)) {
        fprintf(stderr, "expected not-moving after release\n");
        return 1;
    }
    if (r01_play_anim_entity_state(&m.play.anim) != 1) {
        fprintf(stderr, "expected to keep slide_x after release, got %d\n",
                r01_play_anim_entity_state(&m.play.anim));
        return 1;
    }

    /* Left from rest: enter left tile at local (7,0); release snaps X to 0. */
    {
        int tx = m.play.player_tx;
        int ty = m.play.player_ty;
        /* Snap to rest in current tile first. */
        m.play.player_px = tx * 8;
        m.play.player_py = ty * 8;
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        r01ne_play_set_pad(&m, R01NE_PAD_LEFT);
        r01ne_machine_frame(&m);
        if (m.play.player_tx != tx - 1 || m.play.player_px != m.play.player_tx * 8 + 7 ||
            m.play.player_py != ty * 8) {
            fprintf(stderr, "expected left enter at local (7,0), tx=%d px=%d py=%d\n", m.play.player_tx,
                    m.play.player_px, m.play.player_py);
            return 1;
        }
        r01ne_play_set_pad(&m, 0);
        r01ne_machine_frame(&m);
        if (m.play.player_px != m.play.player_tx * 8) {
            fprintf(stderr, "expected left release snap to local x=0, px=%d\n", m.play.player_px);
            return 1;
        }
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

    printf("ok play tile=(%d,%d)->(%d,%d) px=%d->%d blocked=%d\n", tx0, ty0, m.play.player_tx, m.play.player_ty,
           px0, m.play.player_px, blocked);
    r01ne_machine_shutdown(&m);
    return 0;
}
