#ifndef retr01_STUDIO_CART_H
#define retr01_STUDIO_CART_H

#include "retr01_studio/types.h"

/* World header byte offsets (relative to world blob base). */
#define R01_CART_WHDR_SPAWN_CELL 0
#define R01_CART_WHDR_DEFAULT_BANK 2
#define R01_CART_WHDR_DEFAULT_FG 4
#define R01_CART_WHDR_SCREEN_COUNT 5
#define R01_CART_WHDR_FLAGS 7
#define R01_CART_WHDR_OFF_CHR 8
#define R01_CART_WHDR_OFF_SCREEN_DIR 11
#define R01_CART_WHDR_TYPE_COUNT 17
#define R01_CART_WHDR_INST_COUNT 18
#define R01_CART_WHDR_OFF_TYPES 19
#define R01_CART_WHDR_OFF_INSTS 22
#define R01_CART_WHDR_PLAYER_ENTITY 25
#define R01_CART_WHDR_PLAYER_HIT_X 26
#define R01_CART_WHDR_PLAYER_HIT_Y 27
#define R01_CART_WHDR_PLAYER_HIT_W 28
#define R01_CART_WHDR_PLAYER_HIT_H 29
#define R01_CART_PLAYER_ENTITY_NONE 0xFFu

void r01_prom_fill(uint8_t out64[R01_MASTER_COLORS]);
int r01_prom_write(const char *path, char *err_buf, size_t err_cap);
int r01_prg_write_asm(const R01Project *p, const char *path, char *err_buf, size_t err_cap);
int r01_cart_write(const R01Project *p, const char *path, char *err_buf, size_t err_cap);
int r01_cart_write_flash(const R01Project *p, const char *path, char *err_buf, size_t err_cap);
int r01_export_bundle(const R01Project *p, const char *path_stem, char *err_buf, size_t err_cap);

#endif
