#ifndef RETR01_NANO_EMU_PLAY_H
#define RETR01_NANO_EMU_PLAY_H

#include "retr01_nano_emu/types.h"
#include "r01_play_anim.h"

#include <stdint.h>

struct R01neMachine;

#define R01NE_PLAY_PLAYER_W 8
#define R01NE_PLAY_PLAYER_H 8

#define R01NE_LASERS_MAX 8
#define R01NE_LASER_STATE_H 0
#define R01NE_LASER_STATE_V 1

/*
 * Movement strategies — see nano/docs/movement.md.
 * Default: tile-enter on press from rest, then 1 px/frame while held.
 */
typedef enum R01neMoveStrategy {
    R01NE_MOVE_PIXEL_CONTINUOUS = 0, /* default: 1 px/frame, no press/release tile snap */
    R01NE_MOVE_TILE_ENTER_PIXEL = 1  /* legacy: press enters next tile, release snaps to local 0 */
} R01neMoveStrategy;

typedef struct R01neLaser {
    int active;
    int tx; /* world tile */
    int ty;
    int dtx; /* -1/0/1 per frame */
    int dty;
    int state; /* laser entity state: H or V */
    int flip_h;
    int fg;
} R01neLaser;

/*
 * Nano motion: pixel_x/y integrate speed and drive sprite draw.
 * tile_x/y = pixel/8 for MAP SOLID, screen switch, and movement enter rules.
 * Player is a display sprite (transparent 0-bits over MAP). See nano/docs/graphics.md.
 */
typedef struct R01nePlay {
    int enabled;
    int player_px; /* world pixels: motion + sprite origin */
    int player_py;
    int player_tx; /* occupied tile: solid / screen */
    int player_ty;
    int player_type; /* entity type index drawn as player; -1 = none */
    int player_fg;
    int laser_type; /* entity type for lasers; -1 = none */
    R01neLaser lasers[R01NE_LASERS_MAX];
    R01neMoveStrategy move_strategy;
    uint8_t pad0;
    uint8_t pad_prev;
    R01PlayAnimCtx anim;
} R01nePlay;

void r01ne_play_reset(R01nePlay *pl);
int r01ne_play_start(struct R01neMachine *m);
void r01ne_play_set_pad(struct R01neMachine *m, uint8_t pad0);
void r01ne_play_tick(struct R01neMachine *m);

/* Keep MAP on the screen under the player's tile (instant switch). */
void r01ne_play_sync_screen(struct R01neMachine *m);

#endif
