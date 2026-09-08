#ifndef RETR01_NANO_EMU_MACHINE_H
#define RETR01_NANO_EMU_MACHINE_H

#include "retr01_nano_emu/cart.h"
#include "retr01_nano_emu/play.h"
#include "retr01_nano_emu/video.h"

typedef struct R01neMachine {
    R01neCart cart;
    R01neVideo video;
    R01nePlay play;
    R01neWorldView world;
    int world_idx;
    int booted;
} R01neMachine;

void r01ne_machine_shutdown(R01neMachine *m);
int r01ne_machine_boot(R01neMachine *m, const char *cart_path, char *err, size_t err_cap);
int r01ne_machine_reset(R01neMachine *m);

/* Pad + play tick + compose FB. */
void r01ne_machine_frame(R01neMachine *m);

#endif
