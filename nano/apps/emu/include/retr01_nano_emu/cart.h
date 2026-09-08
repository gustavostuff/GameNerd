#ifndef RETR01_NANO_EMU_CART_H
#define RETR01_NANO_EMU_CART_H

#include "retr01_nano_emu/types.h"

#include <stddef.h>
#include <stdint.h>

typedef struct R01neCart {
    uint8_t *data;
    size_t len;
    uint8_t format_ver;
    uint8_t flags;
    uint32_t off_world_dir;
    uint32_t len_world_dir;
    uint32_t off_music;
    uint32_t len_music;
} R01neCart;

typedef struct R01neWorldView {
    int present;
    uint32_t base; /* absolute offset of world blob */
    uint32_t len;
    uint8_t spawn_col;
    uint8_t spawn_row;
    uint8_t default_bg_bank;
    uint8_t default_fg;
    uint8_t screen_count;
    uint8_t flags;
    uint32_t off_chr; /* relative to world base */
    uint32_t off_screen_dir;
    uint8_t entity_type_count;
    uint8_t entity_inst_count;
    uint32_t off_entity_types;
    uint32_t off_entity_insts;
    uint8_t player_entity;
} R01neWorldView;

void r01ne_cart_free(R01neCart *c);
int r01ne_cart_load_path(R01neCart *out, const char *path, char *err, size_t err_cap);
int r01ne_cart_load_mem(R01neCart *out, const uint8_t *img, size_t len, char *err, size_t err_cap);

const uint8_t *r01ne_cart_ptr(const R01neCart *c, uint32_t abs_off, size_t need);

/* World slot 0..7 from the world directory. */
int r01ne_cart_world(const R01neCart *c, int world_idx, R01neWorldView *out);

/* Absolute pointer into cart for a world-relative offset. */
const uint8_t *r01ne_world_ptr(const R01neCart *c, const R01neWorldView *w, uint32_t rel_off, size_t need);

/* Find present screen dir index for grid cell; -1 if absent. */
int r01ne_world_find_screen(const R01neCart *c, const R01neWorldView *w, int col, int row);

/* Copy 384-byte screen payload (tiles||attrs). Returns 0 on success. */
int r01ne_world_load_screen(const R01neCart *c, const R01neWorldView *w, int dir_idx, uint8_t out[R01NE_SCREEN_PAYLOAD]);

/* MAP attr / solid queries in world pixels (non-entity tiles only). */
int r01ne_cart_has_screen(const R01neCart *c, int world_idx, int col, int row);
int r01ne_cart_attr_at(const R01neCart *c, int world_idx, int wx, int wy, uint8_t *out_attr);
int r01ne_cart_solid_at(const R01neCart *c, int world_idx, int wx, int wy);
int r01ne_cart_aabb_ok(const R01neCart *c, int world_idx, int px, int py, int bw, int bh);

#endif
