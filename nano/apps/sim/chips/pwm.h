#ifndef RETR01_NANO_SIM_PWM_H
#define RETR01_NANO_SIM_PWM_H

#include "retr01_sim/entity.h"

#include <stdint.h>

/*
 * Two PWM outputs (music + SFX), resistor-mixed on hardware.
 * Stub: duty/freq state + pin levels. No softsynth in v1.
 */
typedef struct R01nsPwm {
    R01sEntity base;
    uint8_t duty[2];
    uint16_t freq_hz[2];
    uint32_t ticks;
} R01nsPwm;

void r01ns_pwm_init(R01nsPwm *chip, const char *refdes);
R01sEntity *r01ns_pwm_entity(R01nsPwm *chip);
void r01ns_pwm_set(R01nsPwm *chip, int ch, uint8_t duty, uint16_t freq_hz);

#endif
