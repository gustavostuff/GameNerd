#ifndef RETR01_NANO_SIM_ATMEGA1284P_NANO_H
#define RETR01_NANO_SIM_ATMEGA1284P_NANO_H

#include "retr01_sim/entity.h"

#include "retr01_nano_emu/machine.h"
#include "video_sink.h"

#include <stdint.h>

struct R01nsSst25;
struct R01nsPwm;
struct R01sI2cEeprom;
struct R01sPads;

#define R01NS_MAP_PAYLOAD 384u
#define R01NS_RGBS_ACTIVE_LINES R01NS_VIDEO_H
#define R01NS_VBLANK_LINES 40
#define R01NS_SPI_BYTES_PER_VB_STEP 12u
#define R01NS_FIELD_LINES (R01NS_RGBS_ACTIVE_LINES + R01NS_VBLANK_LINES)

/*
 * Nano-role ATmega1284P — sole console MCU.
 * Pin names follow nano/docs/pinmap.md (PORT.bit / datasheet signals).
 * Package on PCB is TQFP-44; sim draws PDIP-40 with the same signal names.
 */
typedef struct R01nsAtmega1284pNano {
    R01sEntity base;
    R01neMachine machine;
    struct R01nsSst25 *flash;
    struct R01sI2cEeprom *eeprom;
    struct R01nsPwm *pwm;
    struct R01nsVideoSink *sink;
    struct R01sPads *pads;

    int field_line;
    int in_vblank;
    int need_compose;

    int spi_active;
    uint32_t spi_addr;
    uint32_t spi_off;
    uint32_t spi_len;

    uint8_t last_screen;
    uint32_t frames;
    uint32_t vblank_refills;
    uint32_t map_bytes_spi;
    int booted;
    int prev_phi2_high;
    char err[256];
} R01nsAtmega1284pNano;

void r01ns_atmega1284p_nano_init(R01nsAtmega1284pNano *chip, const char *refdes);
R01sEntity *r01ns_atmega1284p_nano_entity(R01nsAtmega1284pNano *chip);

void r01ns_atmega1284p_nano_bind(R01nsAtmega1284pNano *chip,
                                 struct R01nsSst25 *flash,
                                 struct R01sI2cEeprom *eeprom,
                                 struct R01nsPwm *pwm,
                                 struct R01nsVideoSink *sink,
                                 struct R01sPads *pads);

int r01ns_atmega1284p_nano_boot(R01nsAtmega1284pNano *chip);
void r01ns_atmega1284p_nano_kernel_tick(R01nsAtmega1284pNano *chip);
const char *r01ns_atmega1284p_nano_err(const R01nsAtmega1284pNano *chip);

#endif
