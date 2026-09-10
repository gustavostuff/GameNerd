#include "retr01_studio/cart.h"
#include "retr01_studio/chr_pack.h"
#include "retr01_studio/export_codegen.h"
#include "retr01_studio/project.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Buf {
    uint8_t *data;
    size_t len;
    size_t cap;
} Buf;

static void set_err(char *err_buf, size_t err_cap, const char *msg) {
    if (err_buf && err_cap > 0) {
        snprintf(err_buf, err_cap, "%s", msg ? msg : "error");
    }
}

static int buf_reserve(Buf *b, size_t need) {
    uint8_t *n;
    size_t cap = b->cap ? b->cap : 4096;
    if (need <= b->cap) {
        return 0;
    }
    while (cap < need) {
        cap *= 2;
    }
    n = (uint8_t *)realloc(b->data, cap);
    if (!n) {
        return -1;
    }
    b->data = n;
    b->cap = cap;
    return 0;
}

static int buf_append(Buf *b, const void *src, size_t n) {
    if (buf_reserve(b, b->len + n) != 0) {
        return -1;
    }
    memcpy(b->data + b->len, src, n);
    b->len += n;
    return 0;
}

static int buf_pad(Buf *b, size_t to_len, uint8_t fill) {
    if (to_len < b->len || buf_reserve(b, to_len) != 0) {
        return -1;
    }
    memset(b->data + b->len, fill, to_len - b->len);
    b->len = to_len;
    return 0;
}

static void wr_u8(uint8_t *p, uint8_t v) {
    p[0] = v;
}

static void wr_u24(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
}

static void tile_2bpp_to_1bpp(const uint8_t src[R01_TILE_BYTES], uint8_t dst[R01_NANO_TILE_BYTES]) {
    int row;
    for (row = 0; row < 8; row++) {
        uint8_t bits = 0;
        int sx;
        for (sx = 0; sx < 8; sx++) {
            if (r01_tile_pixel_color(src, sx, row) != 0) {
                bits |= (uint8_t)(1u << (7 - sx));
            }
        }
        dst[row] = bits;
    }
}

static int build_world_blob(Buf *blob, const R01World *w) {
    uint8_t hdr[R01_CART_WORLD_HDR_BYTES];
    uint8_t dir[R01_MAX_PRESENT_SCREENS * R01_CART_SCREEN_DIR_BYTES];
    size_t off_chr;
    size_t off_sdir;
    size_t off_spay;
    int si;
    int bi;
    int present_n = 0;

    if (!blob || !w) {
        return -1;
    }
    for (si = 0; si < w->screen_count; si++) {
        if (w->screens[si].present) {
            present_n++;
        }
    }
    if (present_n > R01_MAX_PRESENT_SCREENS) {
        return -1;
    }

    off_chr = R01_CART_WORLD_HDR_BYTES;
    off_sdir = off_chr + (size_t)R01_BG_BANKS * R01_NANO_BANK_CHR_BYTES;
    off_spay = off_sdir + (size_t)present_n * R01_CART_SCREEN_DIR_BYTES;

    memset(hdr, 0, sizeof(hdr));
    {
        int ds = r01_world_default_screen(w);
        const R01Screen *spawn = &w->screens[ds];
        wr_u8(hdr + R01_CART_WHDR_SPAWN_CELL, R01_CELL_PACK(spawn->col, spawn->row));
    }
    wr_u8(hdr + R01_CART_WHDR_DEFAULT_BANK, (uint8_t)(w->default_bg_bank & 3));
    wr_u8(hdr + R01_CART_WHDR_DEFAULT_FG, (uint8_t)(w->default_pal_row & 7));
    wr_u8(hdr + R01_CART_WHDR_SCREEN_COUNT, (uint8_t)present_n);
    wr_u8(hdr + R01_CART_WHDR_FLAGS, 0);
    wr_u24(hdr + R01_CART_WHDR_OFF_CHR, (uint32_t)off_chr);
    wr_u24(hdr + R01_CART_WHDR_OFF_SCREEN_DIR, (uint32_t)off_sdir);
    /* Tile-only cart: no entity type/instance tables. */
    wr_u8(hdr + R01_CART_WHDR_TYPE_COUNT, 0);
    wr_u8(hdr + R01_CART_WHDR_INST_COUNT, 0);
    wr_u24(hdr + R01_CART_WHDR_OFF_TYPES, 0);
    wr_u24(hdr + R01_CART_WHDR_OFF_INSTS, 0);
    wr_u8(hdr + R01_CART_WHDR_PLAYER_ENTITY, R01_CART_PLAYER_ENTITY_NONE);
    wr_u8(hdr + R01_CART_WHDR_PLAYER_HIT_X, 0);
    wr_u8(hdr + R01_CART_WHDR_PLAYER_HIT_Y, 0);
    wr_u8(hdr + R01_CART_WHDR_PLAYER_HIT_W, 0);
    wr_u8(hdr + R01_CART_WHDR_PLAYER_HIT_H, 0);

    if (buf_append(blob, hdr, sizeof(hdr)) != 0) {
        return -1;
    }

    for (bi = 0; bi < R01_BG_BANKS; bi++) {
        uint8_t bank[R01_NANO_BANK_CHR_BYTES];
        int ti;
        memset(bank, 0, sizeof(bank));
        for (ti = 0; ti < w->bg_banks[bi].tile_count && ti < R01_TILES_PER_BANK; ti++) {
            const uint8_t *src = w->bg_banks[bi].chr + (size_t)ti * R01_TILE_BYTES;
            uint8_t *dst = bank + (size_t)ti * R01_NANO_TILE_BYTES;
            tile_2bpp_to_1bpp(src, dst);
        }
        if (buf_append(blob, bank, sizeof(bank)) != 0) {
            return -1;
        }
    }

    memset(dir, 0, sizeof(dir));
    {
        int di = 0;
        for (si = 0; si < w->screen_count; si++) {
            const R01Screen *s = &w->screens[si];
            uint8_t *e;
            if (!s->present) {
                continue;
            }
            e = dir + (size_t)di * R01_CART_SCREEN_DIR_BYTES;
            wr_u8(e + 0, R01_CELL_PACK(s->col, s->row));
            wr_u24(e + 3, (uint32_t)(off_spay + (size_t)di * R01_CART_SCREEN_PAYLOAD));
            di++;
        }
        if (buf_append(blob, dir, (size_t)present_n * R01_CART_SCREEN_DIR_BYTES) != 0) {
            return -1;
        }
    }

    for (si = 0; si < w->screen_count; si++) {
        const R01Screen *s = &w->screens[si];
        if (!s->present) {
            continue;
        }
        if (buf_append(blob, s->tiles, R01_TILES_PER_SCREEN) != 0 ||
            buf_append(blob, s->attrs, R01_ATTRS_PER_SCREEN) != 0) {
            return -1;
        }
    }
    return 0;
}

static int r01_cart_build(const R01Project *p, uint8_t **out, size_t *out_len, char *err_buf, size_t err_cap) {
    Buf cart = {0};
    Buf world_blob = {0};
    uint8_t hdr[R01_CART_HDR_BYTES];
    uint8_t ptrs[R01_CART_PTR_TABLE_BYTES];
    uint8_t wtable[R01_CART_WORLD_DIR_BYTES];
    uint32_t off_wdir;
    uint32_t off_world0;
    const R01World *w;

    if (!p || !out || !out_len) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    *out = NULL;
    *out_len = 0;
    w = &p->worlds[0];
    if (build_world_blob(&world_blob, w) != 0) {
        free(world_blob.data);
        set_err(err_buf, err_cap, "world blob failed");
        return -1;
    }

    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr, R01_CART_MAGIC, 6);
    hdr[6] = R01_CART_FORMAT_VER;
    hdr[7] = 0;

    off_wdir = R01_CART_HDR_BYTES + R01_CART_PTR_TABLE_BYTES;
    off_world0 = off_wdir + R01_CART_WORLD_DIR_BYTES;

    memset(ptrs, 0, sizeof(ptrs));
    wr_u24(ptrs + 0, off_wdir);
    wr_u24(ptrs + 3, R01_CART_WORLD_DIR_BYTES);
    /* slot1 music: len 0 */
    /* slot2, slot3 reserved 0 */

    memset(wtable, 0, sizeof(wtable));
    wr_u8(wtable + 0, 1);
    wr_u24(wtable + 2, off_world0);
    wr_u24(wtable + 5, (uint32_t)world_blob.len);

    if (buf_append(&cart, hdr, sizeof(hdr)) != 0 || buf_append(&cart, ptrs, sizeof(ptrs)) != 0 ||
        buf_append(&cart, wtable, sizeof(wtable)) != 0 || buf_append(&cart, world_blob.data, world_blob.len) != 0) {
        free(world_blob.data);
        free(cart.data);
        set_err(err_buf, err_cap, "oom");
        return -1;
    }
    free(world_blob.data);
    *out = cart.data;
    *out_len = cart.len;
    return 0;
}

void r01_prom_fill(uint8_t out64[R01_MASTER_COLORS]) {
    if (out64) {
        memset(out64, 0, R01_MASTER_COLORS);
    }
}

int r01_prom_write(const char *path, char *err_buf, size_t err_cap) {
    uint8_t prom[R01_MASTER_COLORS];
    FILE *f;
    (void)path;
    r01_prom_fill(prom);
    if (!path) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) {
        set_err(err_buf, err_cap, "cannot write prom");
        return -1;
    }
    if (fwrite(prom, 1, sizeof(prom), f) != sizeof(prom)) {
        fclose(f);
        set_err(err_buf, err_cap, "prom write failed");
        return -1;
    }
    fclose(f);
    return 0;
}

int r01_prg_write_asm(const R01Project *p, const char *path, char *err_buf, size_t err_cap) {
    (void)p;
    (void)path;
    (void)err_buf;
    (void)err_cap;
    return 0;
}

int r01_cart_write(const R01Project *p, const char *path, char *err_buf, size_t err_cap) {
    uint8_t *img = NULL;
    size_t len = 0;
    FILE *f;
    if (!path) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    if (r01_path_ensure_parent(path, err_buf, err_cap) != 0) {
        return -1;
    }
    if (r01_cart_build(p, &img, &len, err_buf, err_cap) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) {
        free(img);
        set_err(err_buf, err_cap, "cannot write cart");
        return -1;
    }
    if (fwrite(img, 1, len, f) != len) {
        fclose(f);
        free(img);
        set_err(err_buf, err_cap, "cart write failed");
        return -1;
    }
    fclose(f);
    free(img);
    return 0;
}

int r01_cart_write_flash(const R01Project *p, const char *path, char *err_buf, size_t err_cap) {
    uint8_t *img = NULL;
    size_t len = 0;
    Buf flash = {0};
    FILE *f;
    if (!path) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    if (r01_path_ensure_parent(path, err_buf, err_cap) != 0) {
        return -1;
    }
    if (r01_cart_build(p, &img, &len, err_buf, err_cap) != 0) {
        return -1;
    }
    if (len > R01_CART_FLASH_BYTES) {
        free(img);
        set_err(err_buf, err_cap, "cart too large");
        return -1;
    }
    if (buf_append(&flash, img, len) != 0 || buf_pad(&flash, R01_CART_FLASH_BYTES, 0xFF) != 0) {
        free(img);
        free(flash.data);
        set_err(err_buf, err_cap, "oom");
        return -1;
    }
    free(img);
    f = fopen(path, "wb");
    if (!f) {
        free(flash.data);
        set_err(err_buf, err_cap, "cannot write flash");
        return -1;
    }
    if (fwrite(flash.data, 1, flash.len, f) != flash.len) {
        fclose(f);
        free(flash.data);
        set_err(err_buf, err_cap, "flash write failed");
        return -1;
    }
    fclose(f);
    free(flash.data);
    return 0;
}

int r01_export_bundle(const R01Project *p, const char *path_stem, char *err_buf, size_t err_cap) {
    char path[R01_PATH_MAX];
    if (!p || !path_stem) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    snprintf(path, sizeof(path), "%s%s", path_stem, R01_CART_EXT);
    if (r01_path_ensure_parent(path, err_buf, err_cap) != 0) {
        return -1;
    }
    if (r01_cart_write(p, path, err_buf, err_cap) != 0) {
        return -1;
    }
    snprintf(path, sizeof(path), "%s_flash.bin", path_stem);
    if (r01_path_ensure_parent(path, err_buf, err_cap) != 0) {
        return -1;
    }
    if (r01_cart_write_flash(p, path, err_buf, err_cap) != 0) {
        return -1;
    }
    if (r01_export_codegen(p, path_stem, err_buf, err_cap) != 0) {
        return -1;
    }
    return 0;
}
