/* Nano map / soft-entity hooks for custom_logic (Host Play + firmware). */
#ifndef R01_MAP_H
#define R01_MAP_H

#include <stdint.h>

/* World-pixel solid query (MAP attr SOLID). */
int r01_map_solid_at(int wx, int wy);

/* Replace MAP cell at world tile with bank0/tile0/attr0 (empty). */
void r01_map_set_empty(int tx, int ty);

/* 1 if tile sits on the player's current screen and that screen exists. */
int r01_map_tile_in_play(int tx, int ty);

/*
 * Soft laser draw list — Host Play stamps these with the "laser" entity type.
 * Call r01_soft_laser_clear once per tick, then r01_soft_laser_add for each live shot.
 */
void r01_soft_laser_clear(void);
void r01_soft_laser_add(int state, int tx, int ty, int flip_h, int fg);

#endif
