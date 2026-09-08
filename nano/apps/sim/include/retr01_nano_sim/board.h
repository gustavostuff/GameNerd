#ifndef RETR01_NANO_SIM_BOARD_H
#define RETR01_NANO_SIM_BOARD_H

#include "atmega1284p_nano.h"
#include "i2c_eeprom.h"
#include "osc8m.h"
#include "pads.h"
#include "pwm.h"
#include "pwr5v.h"
#include "sst25vf010a.h"
#include "video_sink.h"

#include "retr01_sim/island_builder.h"
#include "retr01_sim/island_group.h"

enum {
    R01NS_ISLAND_VIDEO = 0,
    R01NS_ISLAND_MCU = 1,
    R01NS_ISLAND_CART = 2,
    R01NS_ISLAND_COUNT = 3
};

/*
 * Nano motherboard netlist: VIDEO | MCU | CART.
 * No 6502 / PLD / OAM / parallel SST39.
 */
typedef struct R01nsBoard {
    R01sIslandBuilder builder;

    R01sPwr5v pwr;
    R01sOsc8m osc;
    R01nsAtmega1284pNano mcu;
    R01sPads pads;
    R01nsPwm pwm;
    R01nsSst25 flash;
    R01sI2cEeprom eeprom;
    R01nsVideoSink sink;

    char cart_path[512];
    uint64_t sim_ns;
    uint32_t steps;
} R01nsBoard;

int r01ns_board_build(R01nsBoard *board);
void r01ns_board_shutdown(R01nsBoard *board);

R01sIslandGroup *r01ns_board_group(R01nsBoard *board);
R01nsBoard *r01ns_board_from_group(R01sIslandGroup *group);

int r01ns_board_load_cart(R01nsBoard *board, const char *path, char *err, size_t err_cap);
int r01ns_board_boot(R01nsBoard *board);

/* Host arcade pad byte into PADS entity (ARCADE mode). */
void r01ns_board_set_pad(R01nsBoard *board, uint8_t p1);

#endif
