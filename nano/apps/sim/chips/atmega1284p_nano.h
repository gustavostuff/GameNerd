#ifndef RETR01_NANO_SIM_ATMEGA1284P_NANO_H
#define RETR01_NANO_SIM_ATMEGA1284P_NANO_H

#include "retr01_sim/entity.h"

#include "retr01_nano_emu/machine.h"

#include <stdint.h>

struct R01nsSst25;
struct R01nsVideoSink;
struct R01nsPwm;
struct R01sI2cEeprom;
struct R01sPads;

/*
 * Nano-role ATmega1284P: sole console MCU (behavioral firmware services).
 * Not the full Retr01 assist 1284 ($FExx / OAM). Soft compose + Host Play via
 * retr01_nano_emu_core. Pins model SPI/I2C/PWM/pad GPIO + PHI2 clock.
 *
 * Representative DIP pins (subset of 40 for board readability):
 *   CLK, RESET#, MOSI, MISO, SCK, SS#, SDA, SCL, OC1A, OC1B, VCC, GND
 *   PAD0..PAD7 (P1 pad byte bits)
 */
typedef struct R01nsAtmega1284pNano {
    R01sEntity base;
    R01neMachine machine;
    struct R01nsSst25 *flash;
    struct R01sI2cEeprom *eeprom;
    struct R01nsPwm *pwm;
    struct R01nsVideoSink *sink;
    struct R01sPads *pads;

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

/* Boot Host Play from SST25 image (after cart load). */
int r01ns_atmega1284p_nano_boot(R01nsAtmega1284pNano *chip);

/* One behavioral VBlank: pad sense → Host Play → SPI MAP refill model → sink. */
void r01ns_atmega1284p_nano_vblank(R01nsAtmega1284pNano *chip);

const char *r01ns_atmega1284p_nano_err(const R01nsAtmega1284pNano *chip);

#endif
