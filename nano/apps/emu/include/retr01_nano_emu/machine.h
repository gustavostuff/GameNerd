#ifndef RETR01_NANO_EMU_MACHINE_H
#define RETR01_NANO_EMU_MACHINE_H

#include "retr01_nano_emu/cart.h"
#include "retr01_nano_emu/video.h"

typedef struct R01neMachine {
    R01neCart cart;
    R01neVideo video;
    R01neWorldView world;
    int world_idx;
    int booted;
} R01neMachine;

void r01ne_machine_shutdown(R01neMachine *m);
int r01ne_machine_boot(R01neMachine *m, const char *cart_path, char *err, size_t err_cap);
/* Boot from an in-memory cart image (e.g. SPI flash contents). Copies img. */
int r01ne_machine_boot_mem(R01neMachine *m, const uint8_t *img, size_t len, char *err, size_t err_cap);
int r01ne_machine_reset(R01neMachine *m);

/* Recompose spawn-screen MAP into the FB. */
void r01ne_machine_frame(R01neMachine *m);

#endif
