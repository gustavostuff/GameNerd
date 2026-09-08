#ifndef RETR01_NANO_EMU_PLAY_H
#define RETR01_NANO_EMU_PLAY_H

#include "retr01_nano_emu/types.h"
#include "r01_play_anim.h"

#include <stdint.h>

struct R01neMachine;

#define R01NE_PLAY_PLAYER_W 8
#define R01NE_PLAY_PLAYER_H 8

/*
 * Movement strategies — see nano/docs/movement.md.
 * Default: tile-enter on press from rest, then 1 px/frame while held.
 */
typedef enum R01neMoveStrategy {
    R01NE_MOVE_TILE_ENTER_PIXEL = 0
    /* Future: continuous pixel, grid-locked per frame, etc. */
} R01neMoveStrategy;

/*
 * Nano motion: pixel_x/y integrate speed; tile_x/y track the occupied cell for
 * MAP/draw. The picture always stamps the entity on its tile cell — never at
 * sub-tile pixel offsets (nano/docs/graphics.md).
 */
typedef struct R01nePlay {
    int enabled;
    int player_px; /* internal integration (world pixels) */
    int player_py;
    int player_tx; /* occupied tile — draw / solid / screen */
    int player_ty;
    int player_type; /* entity type index drawn as player; -1 = none */
    int player_fg;
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
