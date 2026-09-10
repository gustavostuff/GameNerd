#ifndef retr01_STUDIO_PRG_PHASE1_H
#define retr01_STUDIO_PRG_PHASE1_H

#include "retr01_studio/types.h"

/* Nano has no 6502 PRG; keep API for callers that still pass a layout. */
typedef struct R01PrgCartLayout {
    uint32_t off_pal_bg;
    uint32_t len_pal_bg;
    uint32_t off_pal_spr;
    uint32_t len_pal_spr;
    uint32_t off_map_screen0;
    uint8_t default_pal_row;
} R01PrgCartLayout;

void r01_prg_fill_phase1(uint8_t prg[R01_PRG_BYTES], const R01World *w, const R01PrgCartLayout *layout);

#endif
