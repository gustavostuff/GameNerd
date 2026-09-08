#include "retr01_nano_emu/machine.h"

#include <stdio.h>
#include <string.h>

static int fb_nonzero(const R01neMachine *m) {
    size_t i;
    const uint8_t *fb = m->video.fb;
    size_t n = sizeof(m->video.fb);
    for (i = 0; i < n; i++) {
        if (fb[i] != 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : R01NE_DEFAULT_CART;
    R01neMachine m;
    char err[128];

    memset(&m, 0, sizeof(m));
    if (r01ne_machine_boot(&m, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "boot: %s\n", err);
        return 1;
    }
    if (!m.video.chr_loaded || !m.video.map_loaded) {
        fprintf(stderr, "assets not loaded\n");
        return 1;
    }
    /* Empty maps are valid; still require a composed FB size and reset path. */
    r01ne_machine_frame(&m);
    if (r01ne_machine_reset(&m) != 0) {
        fprintf(stderr, "reset failed\n");
        return 1;
    }
    printf("ok boot fb_any=%d visible=%dx%d\n", fb_nonzero(&m), R01NE_VISIBLE_W, R01NE_VISIBLE_H);
    r01ne_machine_shutdown(&m);
    return 0;
}
