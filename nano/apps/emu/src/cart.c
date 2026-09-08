#include "retr01_nano_emu/cart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_err(char *err, size_t err_cap, const char *msg) {
    if (err && err_cap > 0) {
        snprintf(err, err_cap, "%s", msg ? msg : "error");
    }
}

static uint32_t get_u24(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

void r01ne_cart_free(R01neCart *c) {
    if (!c) {
        return;
    }
    free(c->data);
    memset(c, 0, sizeof(*c));
}

const uint8_t *r01ne_cart_ptr(const R01neCart *c, uint32_t abs_off, size_t need) {
    if (!c || !c->data) {
        return NULL;
    }
    if ((size_t)abs_off + need > c->len) {
        return NULL;
    }
    return c->data + abs_off;
}

int r01ne_cart_load_mem(R01neCart *out, const uint8_t *img, size_t len, char *err, size_t err_cap) {
    const uint8_t *ptrs;
    uint8_t *copy;
    if (!out || !img) {
        set_err(err, err_cap, "bad args");
        return -1;
    }
    memset(out, 0, sizeof(*out));
    if (len < R01NE_CART_HDR_BYTES + R01NE_CART_PTR_TABLE_BYTES) {
        set_err(err, err_cap, "cart too small");
        return -1;
    }
    if (memcmp(img, R01NE_CART_MAGIC, 6) != 0) {
        set_err(err, err_cap, "bad magic (want r01nan)");
        return -1;
    }
    out->format_ver = img[6];
    if (out->format_ver != R01NE_CART_FORMAT_VER) {
        set_err(err, err_cap, "unsupported cart format_ver");
        return -1;
    }
    out->flags = img[7];
    copy = (uint8_t *)malloc(len);
    if (!copy) {
        set_err(err, err_cap, "oom");
        return -1;
    }
    memcpy(copy, img, len);
    out->data = copy;
    out->len = len;

    ptrs = img + R01NE_CART_HDR_BYTES;
    out->off_world_dir = get_u24(ptrs + 0);
    out->len_world_dir = get_u24(ptrs + 3);
    out->off_music = get_u24(ptrs + 6);
    out->len_music = get_u24(ptrs + 9);

    if (out->len_world_dir < 8 ||
        !r01ne_cart_ptr(out, out->off_world_dir, out->len_world_dir)) {
        r01ne_cart_free(out);
        set_err(err, err_cap, "bad world directory");
        return -1;
    }
    return 0;
}

int r01ne_cart_load_path(R01neCart *out, const char *path, char *err, size_t err_cap) {
    FILE *f;
    long sz;
    uint8_t *buf;
    size_t n;
    int rc;
    if (!out || !path) {
        set_err(err, err_cap, "bad args");
        return -1;
    }
    f = fopen(path, "rb");
    if (!f) {
        set_err(err, err_cap, "cannot open cart");
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (sz = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        set_err(err, err_cap, "cannot size cart");
        return -1;
    }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) {
        fclose(f);
        set_err(err, err_cap, "oom");
        return -1;
    }
    n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (n != (size_t)sz) {
        free(buf);
        set_err(err, err_cap, "short read");
        return -1;
    }
    rc = r01ne_cart_load_mem(out, buf, n, err, err_cap);
    free(buf);
    return rc;
}

int r01ne_cart_world(const R01neCart *c, int world_idx, R01neWorldView *out) {
    const uint8_t *slot;
    const uint8_t *hdr;
    uint32_t base;
    uint32_t len;
    if (!c || !out || world_idx < 0 || world_idx >= R01NE_MAX_WORLDS) {
        return -1;
    }
    memset(out, 0, sizeof(*out));
    if ((size_t)world_idx * 8u + 8u > c->len_world_dir) {
        return -1;
    }
    slot = r01ne_cart_ptr(c, c->off_world_dir + (uint32_t)world_idx * 8u, 8);
    if (!slot || !slot[0]) {
        return -1;
    }
    base = get_u24(slot + 2);
    len = get_u24(slot + 5);
    hdr = r01ne_cart_ptr(c, base, R01NE_WORLD_HDR_BYTES);
    if (!hdr || len < R01NE_WORLD_HDR_BYTES) {
        return -1;
    }
    out->present = 1;
    out->base = base;
    out->len = len;
    out->spawn_col = (uint8_t)R01NE_CELL_COL(hdr[R01NE_CART_WHDR_SPAWN_CELL]);
    out->spawn_row = (uint8_t)R01NE_CELL_ROW(hdr[R01NE_CART_WHDR_SPAWN_CELL]);
    out->default_bg_bank = hdr[R01NE_CART_WHDR_DEFAULT_BANK] & 3u;
    out->default_fg = hdr[R01NE_CART_WHDR_DEFAULT_FG] & 7u;
    out->screen_count = hdr[R01NE_CART_WHDR_SCREEN_COUNT];
    out->flags = hdr[R01NE_CART_WHDR_FLAGS];
    out->off_chr = get_u24(hdr + R01NE_CART_WHDR_OFF_CHR);
    out->off_screen_dir = get_u24(hdr + R01NE_CART_WHDR_OFF_SCREEN_DIR);
    out->entity_type_count = hdr[R01NE_CART_WHDR_TYPE_COUNT];
    out->entity_inst_count = hdr[R01NE_CART_WHDR_INST_COUNT];
    out->off_entity_types = get_u24(hdr + R01NE_CART_WHDR_OFF_TYPES);
    out->off_entity_insts = get_u24(hdr + R01NE_CART_WHDR_OFF_INSTS);
    out->player_entity = hdr[R01NE_CART_WHDR_PLAYER_ENTITY];
    if (out->screen_count > R01NE_MAX_PRESENT_SCREENS) {
        out->screen_count = R01NE_MAX_PRESENT_SCREENS;
    }
    if (out->entity_type_count > R01NE_MAX_ENTITY_TYPES) {
        out->entity_type_count = R01NE_MAX_ENTITY_TYPES;
    }
    if (out->entity_inst_count > R01NE_MAX_ENTITY_INSTANCES) {
        out->entity_inst_count = R01NE_MAX_ENTITY_INSTANCES;
    }
    return 0;
}

const uint8_t *r01ne_world_ptr(const R01neCart *c, const R01neWorldView *w, uint32_t rel_off, size_t need) {
    if (!c || !w || !w->present) {
        return NULL;
    }
    if ((size_t)rel_off + need > w->len) {
        return NULL;
    }
    return r01ne_cart_ptr(c, w->base + rel_off, need);
}

uint8_t *r01ne_world_ptr_mut(R01neCart *c, const R01neWorldView *w, uint32_t rel_off, size_t need) {
    if (!c || !c->data || !w || !w->present) {
        return NULL;
    }
    if ((size_t)rel_off + need > w->len) {
        return NULL;
    }
    if ((size_t)w->base + (size_t)rel_off + need > c->len) {
        return NULL;
    }
    return c->data + w->base + rel_off;
}

int r01ne_world_find_screen(const R01neCart *c, const R01neWorldView *w, int col, int row) {
    const uint8_t *dir;
    int i;
    uint8_t want;
    if (!c || !w || col < 0 || row < 0 || col > 15 || row > 15) {
        return -1;
    }
    dir = r01ne_world_ptr(c, w, w->off_screen_dir, (size_t)w->screen_count * R01NE_SCREEN_DIR_BYTES);
    if (!dir) {
        return -1;
    }
    want = R01NE_CELL_PACK(col, row);
    for (i = 0; i < w->screen_count; i++) {
        if (dir[(size_t)i * R01NE_SCREEN_DIR_BYTES] == want) {
            return i;
        }
    }
    return -1;
}

int r01ne_world_load_screen(const R01neCart *c, const R01neWorldView *w, int dir_idx,
                            uint8_t out[R01NE_SCREEN_PAYLOAD]) {
    const uint8_t *dir;
    const uint8_t *pay;
    uint32_t off;
    if (!c || !w || !out || dir_idx < 0 || dir_idx >= w->screen_count) {
        return -1;
    }
    dir = r01ne_world_ptr(c, w, w->off_screen_dir, (size_t)w->screen_count * R01NE_SCREEN_DIR_BYTES);
    if (!dir) {
        return -1;
    }
    off = get_u24(dir + (size_t)dir_idx * R01NE_SCREEN_DIR_BYTES + 3);
    pay = r01ne_world_ptr(c, w, off, R01NE_SCREEN_PAYLOAD);
    if (!pay) {
        return -1;
    }
    memcpy(out, pay, R01NE_SCREEN_PAYLOAD);
    return 0;
}

uint8_t *r01ne_world_screen_payload_mut(R01neCart *c, const R01neWorldView *w, int dir_idx) {
    const uint8_t *dir;
    uint32_t off;
    if (!c || !w || dir_idx < 0 || dir_idx >= w->screen_count) {
        return NULL;
    }
    dir = r01ne_world_ptr(c, w, w->off_screen_dir, (size_t)w->screen_count * R01NE_SCREEN_DIR_BYTES);
    if (!dir) {
        return NULL;
    }
    off = get_u24(dir + (size_t)dir_idx * R01NE_SCREEN_DIR_BYTES + 3);
    return r01ne_world_ptr_mut(c, w, off, R01NE_SCREEN_PAYLOAD);
}

int r01ne_cart_has_screen(const R01neCart *c, int world_idx, int col, int row) {
    R01neWorldView w;
    if (r01ne_cart_world(c, world_idx, &w) != 0) {
        return 0;
    }
    return r01ne_world_find_screen(c, &w, col, row) >= 0;
}

int r01ne_cart_attr_at(const R01neCart *c, int world_idx, int wx, int wy, uint8_t *out_attr) {
    R01neWorldView w;
    int col, row, di, lx, ly, tx, ty, cell;
    uint8_t map[R01NE_SCREEN_PAYLOAD];
    if (!c || wx < 0 || wy < 0) {
        return -1;
    }
    if (r01ne_cart_world(c, world_idx, &w) != 0) {
        return -1;
    }
    col = wx / R01NE_SCREEN_PX_W;
    row = wy / R01NE_SCREEN_PX_H;
    di = r01ne_world_find_screen(c, &w, col, row);
    if (di < 0 || r01ne_world_load_screen(c, &w, di, map) != 0) {
        return -1;
    }
    lx = wx % R01NE_SCREEN_PX_W;
    ly = wy % R01NE_SCREEN_PX_H;
    tx = lx / 8;
    ty = ly / 8;
    cell = ty * R01NE_SCREEN_TILES_X + tx;
    if (out_attr) {
        *out_attr = map[R01NE_TILES_PER_SCREEN + cell];
    }
    return 0;
}

int r01ne_cart_solid_at(const R01neCart *c, int world_idx, int wx, int wy) {
    uint8_t attr;
    if (r01ne_cart_attr_at(c, world_idx, wx, wy, &attr) != 0) {
        return 0;
    }
    return (attr & R01NE_ATTR_SOLID) != 0;
}

int r01ne_cart_aabb_ok(const R01neCart *c, int world_idx, int px, int py, int bw, int bh) {
    int x1, y1, c0, c1, r0, r1, col, row;
    if (!c || px < 0 || py < 0 || bw < 1 || bh < 1) {
        return 0;
    }
    x1 = px + bw - 1;
    y1 = py + bh - 1;
    c0 = px / R01NE_SCREEN_PX_W;
    c1 = x1 / R01NE_SCREEN_PX_W;
    r0 = py / R01NE_SCREEN_PX_H;
    r1 = y1 / R01NE_SCREEN_PX_H;
    for (col = c0; col <= c1; col++) {
        for (row = r0; row <= r1; row++) {
            if (!r01ne_cart_has_screen(c, world_idx, col, row)) {
                return 0;
            }
        }
    }
    if (r01ne_cart_solid_at(c, world_idx, px, py) || r01ne_cart_solid_at(c, world_idx, x1, py) ||
        r01ne_cart_solid_at(c, world_idx, px, y1) || r01ne_cart_solid_at(c, world_idx, x1, y1)) {
        return 0;
    }
    return 1;
}
