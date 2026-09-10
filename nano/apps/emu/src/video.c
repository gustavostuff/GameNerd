#include "retr01_nano_emu/video.h"
#include "retr01_nano_emu/machine.h"

#include <string.h>

/* Match Studio r01_nano_fg_rgb (board resistor DAC preview). */
static const uint8_t NANO_FG_RGB[8][3] = {
    {0xFF, 0xFF, 0xFF}, /* 0 white */
    {0xFF, 0x20, 0x20}, /* 1 red */
    {0xFF, 0xA0, 0x00}, /* 2 orange */
    {0xFF, 0xF0, 0x00}, /* 3 yellow */
    {0x20, 0xE0, 0x40}, /* 4 green */
    {0x00, 0xE0, 0xE8}, /* 5 cyan */
    {0x30, 0x60, 0xFF}, /* 6 blue */
    {0xF0, 0x20, 0xD0}, /* 7 magenta */
};

void r01ne_fg_rgb(int fg, uint8_t *r, uint8_t *g, uint8_t *b) {
    int i = fg & 7;
    if (r) {
        *r = NANO_FG_RGB[i][0];
    }
    if (g) {
        *g = NANO_FG_RGB[i][1];
    }
    if (b) {
        *b = NANO_FG_RGB[i][2];
    }
}

void r01ne_video_reset(R01neVideo *vid) {
    if (!vid) {
        return;
    }
    memset(vid, 0, sizeof(*vid));
    vid->screen_col = -1;
    vid->screen_row = -1;
}

int r01ne_video_load_chr(struct R01neMachine *m, const R01neWorldView *w) {
    const uint8_t *src;
    int bi;
    if (!m || !w) {
        return -1;
    }
    src = r01ne_world_ptr(&m->cart, w, w->off_chr, (size_t)R01NE_BG_BANKS * R01NE_BANK_CHR_BYTES);
    if (!src) {
        return -1;
    }
    for (bi = 0; bi < R01NE_BG_BANKS; bi++) {
        memcpy(m->video.chr[bi], src + (size_t)bi * R01NE_BANK_CHR_BYTES, R01NE_BANK_CHR_BYTES);
    }
    m->video.chr_loaded = 1;
    return 0;
}

int r01ne_video_load_screen(struct R01neMachine *m, const R01neWorldView *w, int col, int row) {
    int di;
    if (!m || !w) {
        return -1;
    }
    di = r01ne_world_find_screen(&m->cart, w, col, row);
    if (di < 0) {
        return -1;
    }
    if (r01ne_world_load_screen(&m->cart, w, di, m->video.map) != 0) {
        return -1;
    }
    m->video.screen_col = col;
    m->video.screen_row = row;
    m->video.map_loaded = 1;
    return 0;
}

static int tile_bit(const uint8_t *tile, int sx, int sy, int flip_h, int flip_v) {
    int x = flip_h ? (7 - sx) : sx;
    int y = flip_v ? (7 - sy) : sy;
    return (tile[y] >> (7 - x)) & 1;
}

static void put_logical(R01neVideo *vid, int px, int py, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t *p;
    if (px < 0 || py < 0 || px >= R01NE_SCREEN_PX_W || py >= R01NE_SCREEN_PX_H) {
        return;
    }
    p = vid->logical + ((size_t)py * R01NE_SCREEN_PX_W + (size_t)px) * 3u;
    p[0] = r;
    p[1] = g;
    p[2] = b;
}

/* MAP cells: bit1 = FG, bit0 = black backdrop (opaque cell). */
static void draw_map_tile_px(R01neVideo *vid, int log_x, int log_y, int bank, int tile_id, int fg, int flip_h,
                             int flip_v) {
    const uint8_t *raw;
    uint8_t fr, fg_c, fb;
    int sy, sx;
    if (bank < 0 || bank >= R01NE_BG_BANKS || tile_id < 0 || tile_id >= R01NE_TILES_PER_BANK) {
        return;
    }
    raw = vid->chr[bank] + (size_t)tile_id * R01NE_TILE_BYTES;
    r01ne_fg_rgb(fg, &fr, &fg_c, &fb);
    for (sy = 0; sy < 8; sy++) {
        for (sx = 0; sx < 8; sx++) {
            int on = tile_bit(raw, sx, sy, flip_h, flip_v);
            int px = log_x + sx;
            int py = log_y + sy;
            if (on) {
                put_logical(vid, px, py, fr, fg_c, fb);
            } else {
                put_logical(vid, px, py, 0, 0, 0);
            }
        }
    }
}

static void draw_map_cell(R01neVideo *vid, int tile_x, int tile_y, int bank, int tile_id, int fg, int flip_h,
                          int flip_v) {
    if (tile_x < 0 || tile_y < 0 || tile_x >= R01NE_SCREEN_TILES_X || tile_y >= R01NE_SCREEN_TILES_Y) {
        return;
    }
    draw_map_tile_px(vid, tile_x * 8, tile_y * 8, bank, tile_id, fg, flip_h, flip_v);
}

static void scale_2x(R01neVideo *vid) {
    int y, x;
    for (y = 0; y < R01NE_SCREEN_PX_H; y++) {
        for (x = 0; x < R01NE_SCREEN_PX_W; x++) {
            const uint8_t *src = vid->logical + ((size_t)y * R01NE_SCREEN_PX_W + (size_t)x) * 3u;
            int dy, dx;
            for (dy = 0; dy < 2; dy++) {
                for (dx = 0; dx < 2; dx++) {
                    uint8_t *dst =
                        vid->fb + ((size_t)(y * 2 + dy) * R01NE_VISIBLE_W + (size_t)(x * 2 + dx)) * 3u;
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                }
            }
        }
    }
}

void r01ne_video_render_frame(struct R01neMachine *m) {
    int ty, tx;
    R01neVideo *vid;
    const uint8_t *tiles;
    const uint8_t *attrs;
    if (!m) {
        return;
    }
    vid = &m->video;
    memset(vid->logical, 0, sizeof(vid->logical));
    memset(vid->fb, 0, sizeof(vid->fb));
    if (!vid->map_loaded || !vid->chr_loaded) {
        return;
    }
    tiles = vid->map;
    attrs = vid->map + R01NE_TILES_PER_SCREEN;
    for (ty = 0; ty < R01NE_SCREEN_TILES_Y; ty++) {
        for (tx = 0; tx < R01NE_SCREEN_TILES_X; tx++) {
            int cell = ty * R01NE_SCREEN_TILES_X + tx;
            uint8_t tile_id = tiles[cell];
            uint8_t attr = attrs[cell];
            draw_map_cell(vid, tx, ty, r01ne_attr_bank(attr), tile_id, r01ne_attr_fg(attr),
                          r01ne_attr_flip_h(attr), r01ne_attr_flip_v(attr));
        }
    }
    scale_2x(vid);
}
