#include "retr01_nano_sim/board.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    R01nsBoard board;
    char err[256];
    const char *path = argc > 1 ? argv[1] : NULL;
    R01sIslandGroup *group;
    int i;

    if (!path) {
        fprintf(stderr, "usage: %s <cart.r01nano>\n", argv[0]);
        return 2;
    }
    if (r01ns_board_build(&board) != 0) {
        fprintf(stderr, "board_build failed\n");
        return 1;
    }
    group = r01ns_board_group(&board);
    if (!group || group->island_count != 3) {
        fprintf(stderr, "expected 3 islands\n");
        return 1;
    }
    for (i = 0; i < board.builder.mount_count; i++) {
        const R01sEntity *e = board.builder.mounts[i].entity;
        if (e && e->part && strstr(e->part, "6502")) {
            fprintf(stderr, "unexpected 6502 part\n");
            return 1;
        }
    }
    if (r01ns_board_load_cart(&board, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "load: %s\n", err);
        return 1;
    }
    if (r01ns_board_boot(&board) != 0) {
        fprintf(stderr, "boot: %s\n", r01ns_atmega1284p_nano_err(&board.mcu));
        return 1;
    }
    r01ns_board_set_pads(&board, 0, 0);
    /* One full field (active + VBlank) so kernel runs Host Play once. */
    for (i = 0; i < R01NS_FIELD_LINES + 8; i++) {
        r01s_island_group_step(group);
    }
    if (board.mcu.frames < 1) {
        fprintf(stderr, "no completed fields after %d steps\n", R01NS_FIELD_LINES + 8);
        return 1;
    }
    if (board.mcu.vblank_refills < 1) {
        fprintf(stderr, "expected SPI MAP refill\n");
        return 1;
    }
    printf("ok: islands=%d fields=%u steps=%u spi=%u line=%d\n", group->island_count, board.mcu.frames,
           board.steps, board.mcu.map_bytes_spi, board.mcu.field_line);
    r01ns_board_shutdown(&board);
    return 0;
}
