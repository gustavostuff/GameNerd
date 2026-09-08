#include "retr01_nano_sim/board.h"

#include "retr01_sim/bus.h"
#include "retr01_sim/gamepad.h"

#include <stdio.h>

int main(int argc, char **argv) {
    R01nsBoard board;
    char err[256];
    const char *path = argc > 1 ? argv[1] : NULL;
    int i;
    int px0;
    int py0;

    if (!path) {
        fprintf(stderr, "usage: %s <cart.r01nano>\n", argv[0]);
        return 2;
    }
    if (r01ns_board_build(&board) != 0) {
        return 1;
    }
    if (r01ns_board_load_cart(&board, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "%s\n", err);
        return 1;
    }
    if (r01ns_board_boot(&board) != 0) {
        fprintf(stderr, "%s\n", r01ns_atmega1284p_nano_err(&board.mcu));
        return 1;
    }
    px0 = board.mcu.machine.play.player_px;
    py0 = board.mcu.machine.play.player_py;
    r01ns_board_set_pads(&board, R01S_PAD_RIGHT, 0);
    r01s_entity_eval(&board.mcu.base);
    /* Hold RIGHT across several fields (play advances on each VBlank enter). */
    for (i = 0; i < R01NS_FIELD_LINES * 3; i++) {
        r01s_island_group_step(r01ns_board_group(&board));
    }
    if (board.mcu.machine.play.player_px == px0 && board.mcu.machine.play.player_py == py0) {
        fprintf(stderr, "pad RIGHT did not move player (pad0=0x%02x fields=%u)\n",
                board.mcu.machine.play.pad0, board.mcu.frames);
        r01ns_board_shutdown(&board);
        return 1;
    }
    printf("ok: moved dx=%d fields=%u pad0=0x%02x\n", board.mcu.machine.play.player_px - px0,
           board.mcu.frames, board.mcu.machine.play.pad0);
    r01ns_board_shutdown(&board);
    return 0;
}
