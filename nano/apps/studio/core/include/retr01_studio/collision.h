#ifndef retr01_STUDIO_COLLISION_H
#define retr01_STUDIO_COLLISION_H

#include "retr01_studio/types.h"

/* Attr without SOLID (bit7). Used by tile-edit “apply all” matching. */
#define R01_ATTR_HW_MASK 0x7Fu

static inline uint8_t r01_attr_hw(uint8_t a) {
    return (uint8_t)(a & R01_ATTR_HW_MASK);
}

static inline int r01_attr_hw_match(uint8_t a, uint8_t b) {
    return r01_attr_hw(a) == r01_attr_hw(b);
}

/*
 * Set or clear R01_ATTR_SOLID on every MAP cell in w that uses CHR bank+tile_id.
 * Returns number of cells touched.
 */
int r01_world_apply_solid_tile(R01World *w, int bank, uint8_t tile_id, int set_solid);

#endif
