#include "retr01_studio/export_codegen.h"

#include <stdio.h>

/* MAP/CHR-only Studio: no entity/player/OAM host codegen. */
int r01_export_codegen(const R01Project *p, const char *path_stem, char *err_buf, size_t err_cap) {
    (void)p;
    (void)path_stem;
    if (err_buf && err_cap > 0) {
        err_buf[0] = '\0';
    }
    return 0;
}
