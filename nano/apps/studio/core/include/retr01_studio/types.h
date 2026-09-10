#ifndef retr01_STUDIO_TYPES_H
#define retr01_STUDIO_TYPES_H

#include <stddef.h>
#include <stdint.h>

#define R01_SCREEN_TILES_X 16
#define R01_SCREEN_TILES_Y 12 /* Nano logical playfield (128x96) */
#define R01_SCREEN_PX_W 128
#define R01_SCREEN_PX_H 96
#define R01_TILES_PER_SCREEN (R01_SCREEN_TILES_X * R01_SCREEN_TILES_Y) /* 192 */
#define R01_ATTRS_PER_SCREEN R01_TILES_PER_SCREEN

#define R01_GRID_MAX 16
#define R01_DEFAULT_GRID 3
#define R01_MAX_SCREENS (R01_GRID_MAX * R01_GRID_MAX)
#define R01_MAX_PRESENT_SCREENS 16 /* Nano: 16 present screens/world */
/* Virtual grid cell: col/row 0-15 packed as nibbles (cart dir + world spawn). */
#define R01_CELL_PACK(col, row) ((uint8_t)(((unsigned)(col)&0x0fu) | (((unsigned)(row)&0x0fu) << 4)))
#define R01_CELL_COL(b) ((int)((unsigned)(b)&0x0fu))
#define R01_CELL_ROW(b) ((int)(((unsigned)(b) >> 4) & 0x0fu))
#define R01_PARALLAX_MIN 0
#define R01_PARALLAX_MAX 8 /* unused on Nano; kept for struct compatibility */
#define R01_PARALLAX_SLICE_MAX 120
#define R01_START_COL 2
#define R01_START_ROW 0

#define R01_MAX_WORLDS 8
#define R01_BG_BANKS 4
#define R01_TILES_PER_BANK 256
/* Studio still authors 2bpp 16-byte tiles; ROM export packs 1bpp (8 bytes). */
#define R01_TILE_BYTES 16
#define R01_NANO_TILE_BYTES 8
#define R01_BANK_CHR_BYTES (R01_TILES_PER_BANK * R01_TILE_BYTES)
#define R01_NANO_BANK_CHR_BYTES (R01_TILES_PER_BANK * R01_NANO_TILE_BYTES)

#define R01_BG0_SCREENS_MAX 8

#define R01_MASTER_COLORS 64
#define R01_PAL_COLORS 4
#define R01_PALS_PER_ROW 4
#define R01_PAL_ROWS 8
#define R01_PAL_COUNT (R01_PAL_ROWS * R01_PALS_PER_ROW)
#define R01_PAL_PLANE_BYTES (R01_PAL_COUNT * R01_PAL_COLORS)

#define R01_CART_FLASH_BYTES (128u * 1024u) /* 25LC1024 */
#define R01_PRG_BYTES 32768u               /* unused on Nano (no 6502) */
#define R01_CHR_BANK_BYTES R01_NANO_BANK_CHR_BYTES
#define R01_CART_FORMAT_VER 1
#define R01_CART_MAGIC "r01nan"
#define R01_CART_EXT ".r01nano"
#define R01_CART_HDR_BYTES 16u
#define R01_CART_PTR_SLOTS 4u
#define R01_CART_PTR_TABLE_BYTES (R01_CART_PTR_SLOTS * 6u) /* 24 */
#define R01_CART_SCREEN_PAYLOAD 384u /* 192 tile + 192 attr */
#define R01_CART_WORLD_DIR_BYTES (R01_MAX_WORLDS * 8u)
#define R01_CART_WORLD_HDR_BYTES 32u
#define R01_CART_SCREEN_DIR_BYTES 8u
/* Cart header still reserves entity slots (always zero / unused). */
#define R01_CART_ENTITY_TYPE_SIZE 10
#define R01_CART_INSTANCE_SIZE 8
#define R01_CART_OTHER_MAX 48
#define R01_CART_OTHER_TITLE 0
#define R01_CART_OTHER_INTER 1
#define R01_CART_OTHER_CREDITS_FIRST 2
#define R01_CART_CREDITS_MIN 0
#define R01_CART_CREDITS_MAX (R01_CART_OTHER_MAX - R01_CART_OTHER_CREDITS_FIRST)
#define R01_CART_OTHER_HDR_BYTES 4u
#define R01_CART_OTHER_DIR_BYTES 8u
#define R01_CART_OTHER_FLAG_RLE 0x01u
#define R01_CART_OTHER_BYTES_MAX (16u * 1024u)

#define R01_NAME_MAX 64
#define R01_PATH_MAX 512
#define R01_JSON_VER 1 /* Nano project schema (independent of full Studio v11) */
#define R01_JSON_PLATFORM "nano"

#define R01_OUTPUT_DIR "output/nano"
#define R01_DEFAULT_PROJECT R01_OUTPUT_DIR "/test.r01proj"
#define R01_DEFAULT_CART_STEM R01_OUTPUT_DIR "/test"

#define R01_MAX_METATILES 64
#define R01_LABEL_MAX 32

/* Studio BGM editor (Audio tab). Host flatten uses R01_BGM_* from r01_bgm_host.h. */
#define R01_BGM_TRACKS_MAX 8
#define R01_BGM_REGIONS_MAX 128
#define R01_BGM_CH_COUNT 1 /* Nano: Music pulse only (host softsynth still has 5 slots) */
#define R01_BGM_TOK_MAX 5
#define R01_BGM_NAME_MAX 24

typedef struct R01BgmRegion {
    int start; /* ticks */
    int len;   /* ticks, >= 1 */
    int midi;
    char tok[R01_BGM_TOK_MAX];
} R01BgmRegion;

typedef struct R01BgmData {
    int present; /* 1 = authoring data saved/loaded (else UI may seed demos) */
    int track_count;
    char track_name[R01_BGM_TRACKS_MAX][R01_BGM_NAME_MAX];
    int region_count[R01_BGM_TRACKS_MAX][R01_BGM_CH_COUNT];
    R01BgmRegion region[R01_BGM_TRACKS_MAX][R01_BGM_CH_COUNT][R01_BGM_REGIONS_MAX];
} R01BgmData;

/* BG attr (nano/docs/graphics.md) */
#define R01_ATTR_BANK_MASK 0x03u
#define R01_ATTR_FLIP_H 0x04u
#define R01_ATTR_FLIP_V 0x08u
#define R01_ATTR_FG_MASK 0x70u
#define R01_ATTR_FG_SHIFT 4
#define R01_ATTR_SOLID 0x80u
/* Legacy aliases used by transitional UI (FG occupies former pal bits). */
#define R01_ATTR_PAL_MASK R01_ATTR_FG_MASK
#define R01_ATTR_PAL_SHIFT R01_ATTR_FG_SHIFT
#define R01_ATTR_ANIM 0x00u /* unused on Nano */

/* One 4-color palette (Studio preview only; board FG is 8 fixed levels). */
typedef struct R01PalRow {
    uint8_t idx[R01_PAL_COLORS];
} R01PalRow;

typedef struct R01Screen {
    int col;
    int row;
    int present;
    uint8_t pixels[R01_SCREEN_PX_W * R01_SCREEN_PX_H];
    uint8_t tiles[R01_TILES_PER_SCREEN];
    uint8_t attrs[R01_ATTRS_PER_SCREEN];
} R01Screen;

typedef struct R01ChrBank {
    int tile_count;
    uint8_t chr[R01_BANK_CHR_BYTES];
} R01ChrBank;

typedef R01ChrBank R01BgBank;

/* 2x2 BG tile group (TL, TR, BL, BR). */
typedef struct R01MetatileDef {
    char name[R01_LABEL_MAX];
    uint8_t tile[4];
    uint8_t attr[4];
} R01MetatileDef;

/* Global off-grid MAP payloads (title, interstitial, credits pages). See docs/graphics. */
typedef struct R01OtherScreen {
    int present; /* 0 = omit from cart; title/inter always present after init */
    uint8_t tiles[R01_TILES_PER_SCREEN];
    uint8_t attrs[R01_ATTRS_PER_SCREEN];
} R01OtherScreen;

typedef struct R01World {
    int present;
    int grid_cols;
    int grid_rows;
    int default_bg_bank;
    int default_pal_row;
    int default_screen; /* index into screens[]; spawn / play start */
    R01Screen screens[R01_MAX_SCREENS];
    int screen_count;
    /* Structured BG0 plane: up to 8 present screens anywhere on the 16x16 map. */
    int bg0_cols; /* present enclosing extent W (derived) */
    int bg0_rows; /* present enclosing extent H (derived) */
    R01Screen bg0_screens[R01_BG0_SCREENS_MAX];
    int bg0_screen_count;  /* slots used in bg0_screens[] (0..8, may include holes) */
    int bg0_active_screen; /* index into bg0_screens[]; -1 none */
    R01BgBank bg_banks[R01_BG_BANKS];
    R01MetatileDef metatiles[R01_MAX_METATILES];
    int metatile_count;
} R01World;

typedef struct R01Project {
    char name[R01_NAME_MAX];
    int default_world; /* Play entry world (begin_play); cart export always uses worlds[0] */
    int active_world;  /* 0..R01_MAX_WORLDS-1 */
    int active_screen; /* index into worlds[active_world].screens */
    /* 8 rows x 4 pals each (docs/graphics). Index [row][pal]. */
    R01PalRow global_pal_bg[R01_PAL_ROWS][R01_PALS_PER_ROW];
    R01PalRow global_pal_spr[R01_PAL_ROWS][R01_PALS_PER_ROW];
    R01OtherScreen other_screens[R01_CART_OTHER_MAX]; /* [0]=title [1]=inter [2+]=credits */
    R01World worlds[R01_MAX_WORLDS];
    R01BgmData bgm;
} R01Project;

static inline int r01_attr_bank(uint8_t a) {
    return (int)(a & R01_ATTR_BANK_MASK);
}
static inline int r01_attr_fg(uint8_t a) {
    return (int)((a & R01_ATTR_FG_MASK) >> R01_ATTR_FG_SHIFT);
}
static inline int r01_attr_pal(uint8_t a) {
    return r01_attr_fg(a);
}
static inline int r01_attr_flip_h(uint8_t a) {
    return (a & R01_ATTR_FLIP_H) != 0;
}
static inline int r01_attr_flip_v(uint8_t a) {
    return (a & R01_ATTR_FLIP_V) != 0;
}

static inline int r01_attr_anim(uint8_t a) {
    (void)a;
    return 0;
}
static inline int r01_attr_solid(uint8_t a) {
    return (a & R01_ATTR_SOLID) != 0;
}

static inline uint8_t r01_attr_pack(int bank, int fg, int flip_h, int flip_v) {
    uint8_t a = (uint8_t)((bank & 3) | ((fg & 7) << R01_ATTR_FG_SHIFT));
    if (flip_h) {
        a |= R01_ATTR_FLIP_H;
    }
    if (flip_v) {
        a |= R01_ATTR_FLIP_V;
    }
    return a;
}

static inline uint8_t r01_attr_merge(uint8_t old, int bank, int fg, int flip_h, int flip_v) {
    return (uint8_t)((old & R01_ATTR_SOLID) | r01_attr_pack(bank, fg, flip_h, flip_v));
}

#endif
