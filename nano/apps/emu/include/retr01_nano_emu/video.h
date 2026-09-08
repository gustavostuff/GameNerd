#ifndef RETR01_NANO_EMU_VIDEO_H
#define RETR01_NANO_EMU_VIDEO_H

#include "retr01_nano_emu/cart.h"
#include "retr01_nano_emu/types.h"

#include <stdint.h>

struct R01neMachine;

typedef struct R01neVideo {
    uint8_t chr[R01NE_BG_BANKS][R01NE_BANK_CHR_BYTES];
    int chr_loaded;

    uint8_t map[R01NE_SCREEN_PAYLOAD]; /* tiles[192] || attrs[192] */
    int screen_col;
    int screen_row;
    int map_loaded;

    /* Logical 128x96 RGB (source of truth), then 2x RGBS FB. */
    uint8_t logical[R01NE_SCREEN_PX_W * R01NE_SCREEN_PX_H * 3];
    uint8_t fb[R01NE_VISIBLE_W * R01NE_VISIBLE_H * 3];
} R01neVideo;

void r01ne_fg_rgb(int fg, uint8_t *r, uint8_t *g, uint8_t *b);

void r01ne_video_reset(R01neVideo *vid);

/* Cache all 4 CHR banks for the world. */
int r01ne_video_load_chr(struct R01neMachine *m, const R01neWorldView *w);

/* Load MAP for grid cell (col,row). Stamps entities for that screen into a compose buffer at render. */
int r01ne_video_load_screen(struct R01neMachine *m, const R01neWorldView *w, int col, int row);

/* Compose logical picture + 2x FB from MAP + entity stamps. */
void r01ne_video_render_frame(struct R01neMachine *m);

#endif
