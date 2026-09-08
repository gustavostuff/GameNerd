#include "retr01_nano_emu/machine.h"
#include "r01_play_anim.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : R01NE_DEFAULT_CART;
    R01neMachine m;
    char err[128];
    int tx0, ty0, px0;
    int tile_moved = 0;
    int pixel_moved = 0;
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

    /* Hold right: pixels integrate every frame; tile advances only every 8 px. */
    r01ne_play_set_pad(&m, R01NE_PAD_RIGHT);
    for (i = 0; i < 8; i++) {
        int px = m.play.player_px;
        int tx = m.play.player_tx;
        r01ne_machine_frame(&m);
        if (m.play.player_px != px) {
            pixel_moved = 1;
        }
        if (i < 7 && m.play.player_tx != tx) {
            fprintf(stderr, "tile advanced early at step %d (px=%d tx=%d)\n", i, m.play.player_px,
                    m.play.player_tx);
            return 1;
        }
    }
    if (m.play.player_tx != tx0 + 1 || m.play.player_ty != ty0) {
        fprintf(stderr, "expected tile +1 after 8 px, got tx=%d (from %d) px=%d\n", m.play.player_tx, tx0,
                m.play.player_px);
        return 1;
    }
    tile_moved = 1;
    if (!pixel_moved) {
        fprintf(stderr, "expected pixel integration\n");
        return 1;
    }
    if (!r01_play_anim_moving(&m.play.anim) || r01_play_anim_entity_state(&m.play.anim) != 1) {
        fprintf(stderr, "expected walk state while moving\n");
        return 1;
    }

    r01ne_play_set_pad(&m, 0);
    r01ne_machine_frame(&m);
    if (r01_play_anim_moving(&m.play.anim) || r01_play_anim_entity_state(&m.play.anim) != 0) {
        fprintf(stderr, "expected idle after release\n");
        return 1;
    }

    /* Solid: walk into a solid tile to the right if one exists. */
    {
        int found_solid = 0;
        int sx;
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

    printf("ok play tile=(%d,%d)->(%d,%d) px=%d->%d moved_tile=%d blocked=%d\n", tx0, ty0, m.play.player_tx,
           m.play.player_ty, px0, m.play.player_px, tile_moved, blocked);
    r01ne_machine_shutdown(&m);
    return 0;
}
