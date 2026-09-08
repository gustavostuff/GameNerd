#include "pwm.h"

#include "retr01_sim/bus.h"

#include <string.h>

static void pwm_reset(R01sEntity *e) {
    R01nsPwm *c = (R01nsPwm *)e;
    c->duty[0] = c->duty[1] = 0;
    c->freq_hz[0] = c->freq_hz[1] = 0;
    c->ticks = 0;
    r01s_entity_drive(e, "PWM0", R01S_LVL_L);
    r01s_entity_drive(e, "PWM1", R01S_LVL_L);
}

static void pwm_eval(R01sEntity *e) {
    R01nsPwm *c = (R01nsPwm *)e;
    r01s_entity_drive(e, "PWM0", c->duty[0] > 0 ? R01S_LVL_H : R01S_LVL_L);
    r01s_entity_drive(e, "PWM1", c->duty[1] > 0 ? R01S_LVL_H : R01S_LVL_L);
}

static void pwm_tick(R01sEntity *e) {
    R01nsPwm *c = (R01nsPwm *)e;
    c->ticks++;
    pwm_eval(e);
}

static void pwm_destroy(R01sEntity *e) {
    (void)e;
}

static const R01sEntityVTable PWM_VT = {pwm_reset, pwm_eval, pwm_tick, pwm_destroy};

void r01ns_pwm_init(R01nsPwm *chip, const char *refdes) {
    if (!chip) {
        return;
    }
    memset(chip, 0, sizeof(*chip));
    r01s_entity_init(&chip->base, &PWM_VT, "PWM2CH", refdes ? refdes : "U30");
    chip->base.impl = chip;
    r01s_entity_add_pin(&chip->base, 1, "PWM0", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 2, "PWM1", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 3, "GND", R01S_PIN_PWR);
    r01s_entity_add_pin(&chip->base, 4, "VCC", R01S_PIN_PWR);
    r01s_entity_set_dip(&chip->base, 8);
    r01s_entity_reset(&chip->base);
}

R01sEntity *r01ns_pwm_entity(R01nsPwm *chip) {
    return chip ? &chip->base : NULL;
}

void r01ns_pwm_set(R01nsPwm *chip, int ch, uint8_t duty, uint16_t freq_hz) {
    if (!chip || ch < 0 || ch > 1) {
        return;
    }
    chip->duty[ch] = duty;
    chip->freq_hz[ch] = freq_hz;
}
