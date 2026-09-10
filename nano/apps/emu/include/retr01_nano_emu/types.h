#ifndef RETR01_NANO_EMU_TYPES_H
#define RETR01_NANO_EMU_TYPES_H

#include <stddef.h>
#include <stdint.h>

/* nano/docs/cart_format.md + nano/docs/graphics.md + nano/docs/video.md */

#define R01NE_CART_MAGIC "r01nan"
#define R01NE_CART_FORMAT_VER 1
#define R01NE_CART_HDR_BYTES 16u
#define R01NE_CART_PTR_SLOTS 4u
#define R01NE_CART_PTR_TABLE_BYTES (R01NE_CART_PTR_SLOTS * 6u) /* 24 */
#define R01NE_CART_FLASH_BYTES (128u * 1024u)

#define R01NE_MAX_WORLDS 8
#define R01NE_MAX_PRESENT_SCREENS 16

#define R01NE_SCREEN_TILES_X 16
#define R01NE_SCREEN_TILES_Y 12
#define R01NE_SCREEN_PX_W 128
#define R01NE_SCREEN_PX_H 96
#define R01NE_TILES_PER_SCREEN 192
#define R01NE_ATTRS_PER_SCREEN 192
#define R01NE_SCREEN_PAYLOAD 384u

#define R01NE_BG_BANKS 4
#define R01NE_TILES_PER_BANK 256
#define R01NE_TILE_BYTES 8 /* 1 bpp, row-major */
#define R01NE_BANK_CHR_BYTES (R01NE_TILES_PER_BANK * R01NE_TILE_BYTES)

#define R01NE_WORLD_DIR_BYTES (R01NE_MAX_WORLDS * 8u)
#define R01NE_WORLD_HDR_BYTES 32u
#define R01NE_SCREEN_DIR_BYTES 8u

#define R01NE_CART_WHDR_SPAWN_CELL 0
#define R01NE_CART_WHDR_DEFAULT_BANK 2
#define R01NE_CART_WHDR_DEFAULT_FG 4
#define R01NE_CART_WHDR_SCREEN_COUNT 5
#define R01NE_CART_WHDR_FLAGS 7
#define R01NE_CART_WHDR_OFF_CHR 8
#define R01NE_CART_WHDR_OFF_SCREEN_DIR 11
/* World header bytes 17..29 reserved (legacy entity tables); loaders ignore. */

#define R01NE_ATTR_BANK_MASK 0x03u
#define R01NE_ATTR_FLIP_H 0x04u
#define R01NE_ATTR_FLIP_V 0x08u
#define R01NE_ATTR_FG_MASK 0x70u
#define R01NE_ATTR_FG_SHIFT 4
#define R01NE_ATTR_SOLID 0x80u

/* Logical compose; RGBS presentation is 2x nearest-neighbor. */
#define R01NE_VISIBLE_W (R01NE_SCREEN_PX_W * 2) /* 256 */
#define R01NE_VISIBLE_H (R01NE_SCREEN_PX_H * 2) /* 192 */

#define R01NE_CELL_PACK(col, row) ((uint8_t)(((unsigned)(col)&0x0fu) | (((unsigned)(row)&0x0fu) << 4)))
#define R01NE_CELL_COL(b) ((int)((unsigned)(b)&0x0fu))
#define R01NE_CELL_ROW(b) ((int)(((unsigned)(b) >> 4) & 0x0fu))

static inline int r01ne_attr_bank(uint8_t a) {
    return (int)(a & R01NE_ATTR_BANK_MASK);
}
static inline int r01ne_attr_fg(uint8_t a) {
    return (int)((a & R01NE_ATTR_FG_MASK) >> R01NE_ATTR_FG_SHIFT);
}
static inline int r01ne_attr_flip_h(uint8_t a) {
    return (a & R01NE_ATTR_FLIP_H) ? 1 : 0;
}
static inline int r01ne_attr_flip_v(uint8_t a) {
    return (a & R01NE_ATTR_FLIP_V) ? 1 : 0;
}

#endif
