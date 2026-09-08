#include "retr01_nano_emu/machine.h"

#include <stdio.h>
#include <string.h>

void r01ne_machine_shutdown(R01neMachine *m) {
    if (!m) {
        return;
    }
    r01ne_cart_free(&m->cart);
    r01ne_video_reset(&m->video);
    r01ne_play_reset(&m->play);
    memset(&m->world, 0, sizeof(m->world));
    m->world_idx = 0;
    m->booted = 0;
}

int r01ne_machine_boot(R01neMachine *m, const char *cart_path, char *err, size_t err_cap) {
    if (!m || !cart_path) {
        if (err && err_cap) {
            snprintf(err, err_cap, "bad args");
        }
        return -1;
    }
    r01ne_machine_shutdown(m);
    if (r01ne_cart_load_path(&m->cart, cart_path, err, err_cap) != 0) {
        return -1;
    }
    m->world_idx = 0;
    if (r01ne_cart_world(&m->cart, 0, &m->world) != 0) {
        r01ne_machine_shutdown(m);
        if (err && err_cap) {
            snprintf(err, err_cap, "world 0 missing");
        }
        return -1;
    }
    if (r01ne_video_load_chr(m, &m->world) != 0) {
        r01ne_machine_shutdown(m);
        if (err && err_cap) {
            snprintf(err, err_cap, "CHR load failed");
        }
        return -1;
    }
    if (r01ne_video_load_screen(m, &m->world, m->world.spawn_col, m->world.spawn_row) != 0) {
        r01ne_machine_shutdown(m);
        if (err && err_cap) {
            snprintf(err, err_cap, "spawn screen load failed");
        }
        return -1;
    }
    m->booted = 1;
    (void)r01ne_play_start(m);
    r01ne_video_render_frame(m);
    return 0;
}

int r01ne_machine_reset(R01neMachine *m) {
    if (!m || !m->booted) {
        return -1;
    }
    if (r01ne_video_load_screen(m, &m->world, m->world.spawn_col, m->world.spawn_row) != 0) {
        return -1;
    }
    (void)r01ne_play_start(m);
    r01ne_video_render_frame(m);
    return 0;
}

void r01ne_machine_frame(R01neMachine *m) {
    if (!m || !m->booted) {
        return;
    }
    r01ne_play_tick(m);
    r01ne_video_render_frame(m);
}
