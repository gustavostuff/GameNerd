#include "retr01_studio/prg_phase1.h"

#include <string.h>

void r01_prg_fill_phase1(uint8_t prg[R01_PRG_BYTES], const R01World *w, const R01PrgCartLayout *layout) {
    (void)w;
    (void)layout;
    if (prg) {
        memset(prg, 0, R01_PRG_BYTES);
    }
}
