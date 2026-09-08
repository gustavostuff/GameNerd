#include "atmega1284p_nano.h"

#include "i2c_eeprom.h"
#include "pads.h"
#include "pwm.h"
#include "sst25vf010a.h"
#include "video_sink.h"

#include "retr01_nano_emu/cart.h"
#include "retr01_nano_emu/play.h"
#include "retr01_nano_emu/types.h"
#include "retr01_sim/bus.h"

#include <stdio.h>
#include <string.h>

#define R01NS_MAP_PAYLOAD 384u

static uint32_t screen_payload_abs(const R01neMachine *m, int col, int row) {
    int di;
    const uint8_t *dir;
    uint32_t rel;
    if (!m || !m->booted) {
        return 0;
    }
    di = r01ne_world_find_screen(&m->cart, &m->world, col, row);
    if (di < 0) {
        return 0;
    }
    dir = r01ne_world_ptr(&m->cart, &m->world, m->world.off_screen_dir,
                          (size_t)m->world.screen_count * R01NE_SCREEN_DIR_BYTES);
    if (!dir) {
        return 0;
    }
    rel = (uint32_t)dir[(size_t)di * R01NE_SCREEN_DIR_BYTES + 3] |
          ((uint32_t)dir[(size_t)di * R01NE_SCREEN_DIR_BYTES + 4] << 8) |
          ((uint32_t)dir[(size_t)di * R01NE_SCREEN_DIR_BYTES + 5] << 16);
    return m->world.base + rel;
}

static void spi_drive_mcu(R01nsAtmega1284pNano *c, R01sLevel ss, R01sLevel sck, R01sLevel mosi) {
    r01s_entity_drive(&c->base, "SS#", ss);
    r01s_entity_drive(&c->base, "SCK", sck);
    r01s_entity_drive(&c->base, "MOSI", mosi);
}

static int model_map_spi_refill(R01nsAtmega1284pNano *c, int col, int row) {
    uint8_t scratch[R01NS_MAP_PAYLOAD];
    uint32_t abs;
    if (!c || !c->flash) {
        return -1;
    }
    abs = screen_payload_abs(&c->machine, col, row);
    if (abs == 0) {
        return -1;
    }
    spi_drive_mcu(c, R01S_LVL_L, R01S_LVL_L, R01S_LVL_H);
    if (r01ns_sst25_spi_read(c->flash, abs, scratch, R01NS_MAP_PAYLOAD) != 0) {
        spi_drive_mcu(c, R01S_LVL_H, R01S_LVL_L, R01S_LVL_L);
        return -1;
    }
    /* Mirror MISO onto MCU pin from flash SO. */
    r01s_entity_drive(&c->base, "MISO", r01s_entity_sense(&c->flash->base, "SO"));
    spi_drive_mcu(c, R01S_LVL_H, R01S_LVL_L, R01S_LVL_L);
    c->vblank_refills++;
    c->map_bytes_spi += R01NS_MAP_PAYLOAD;
    return 0;
}

static void present_frame(R01nsAtmega1284pNano *c) {
    if (!c || !c->sink) {
        return;
    }
    r01ns_video_sink_blit_rgb(c->sink, c->machine.video.fb, sizeof(c->machine.video.fb));
    r01ns_video_sink_on_vblank(c->sink);
}

static void mcu_reset(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    r01ne_machine_shutdown(&c->machine);
    c->booted = 0;
    c->frames = 0;
    c->vblank_refills = 0;
    c->map_bytes_spi = 0;
    c->last_screen = 0;
    c->prev_phi2_high = 0;
    c->err[0] = '\0';
    spi_drive_mcu(c, R01S_LVL_H, R01S_LVL_L, R01S_LVL_L);
    r01s_entity_drive(e, "MISO", R01S_LVL_Z);
    r01s_entity_drive(e, "SDA", R01S_LVL_Z);
    r01s_entity_drive(e, "SCL", R01S_LVL_L);
    r01s_entity_drive(e, "OC1A", R01S_LVL_L);
    r01s_entity_drive(e, "OC1B", R01S_LVL_L);
}

static void mcu_eval(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    int i;
    /* Reflect pad GPIO from PADS entity when wired. */
    if (c->pads) {
        uint8_t bits = r01s_pads_get(c->pads, 0);
        for (i = 0; i < 8; i++) {
            char name[8];
            snprintf(name, sizeof(name), "PAD%d", i);
            r01s_entity_drive(e, name, (bits & (1u << i)) ? R01S_LVL_H : R01S_LVL_L);
        }
    }
}

static void mcu_firmware_frame(R01nsAtmega1284pNano *c) {
    uint8_t before;
    uint8_t after;
    uint8_t pad;
    int i;

    if (!c->booted) {
        return;
    }
    pad = 0;
    for (i = 0; i < 8; i++) {
        char name[8];
        snprintf(name, sizeof(name), "PAD%d", i);
        if (r01s_level_is_high(r01s_entity_sense(&c->base, name))) {
            pad |= (uint8_t)(1u << i);
        }
    }
    r01ne_play_set_pad(&c->machine, pad);
    before = c->last_screen;
    r01ne_machine_frame(&c->machine);
    after = (uint8_t)((c->machine.video.screen_col & 0x0f) |
                      ((c->machine.video.screen_row & 0x0f) << 4));
    if (after != before) {
        (void)model_map_spi_refill(c, c->machine.video.screen_col, c->machine.video.screen_row);
        c->last_screen = after;
    }
    if (c->pwm) {
        r01s_entity_tick(&c->pwm->base);
        r01s_entity_drive(&c->base, "OC1A", r01s_entity_sense(&c->pwm->base, "PWM0"));
        r01s_entity_drive(&c->base, "OC1B", r01s_entity_sense(&c->pwm->base, "PWM1"));
    }
    /* Soft RGBS out each VBlank. */
    r01s_entity_drive(&c->base, "VSYNC", R01S_LVL_H);
    present_frame(c);
    c->frames++;
}

static void mcu_tick(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    /* Clock pin follows PHI2; frame work is r01ns_atmega1284p_nano_vblank (board step). */
    c->prev_phi2_high = r01s_level_is_high(r01s_entity_sense(e, "CLK"));
}

static void mcu_destroy(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    r01ne_machine_shutdown(&c->machine);
}

static const R01sEntityVTable MCU_VT = {mcu_reset, mcu_eval, mcu_tick, mcu_destroy};

void r01ns_atmega1284p_nano_init(R01nsAtmega1284pNano *chip, const char *refdes) {
    static const char *const pad_names[8] = {"PAD0", "PAD1", "PAD2", "PAD3",
                                             "PAD4", "PAD5", "PAD6", "PAD7"};
    int i;
    if (!chip) {
        return;
    }
    memset(chip, 0, sizeof(*chip));
    r01s_entity_init(&chip->base, &MCU_VT, "ATMEGA1284P", refdes ? refdes : "U1");
    chip->base.impl = chip;
    r01s_entity_add_pin(&chip->base, 1, "CLK", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 2, "RESET#", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 3, "MOSI", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 4, "MISO", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 5, "SCK", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 6, "SS#", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 7, "SDA", R01S_PIN_IO);
    r01s_entity_add_pin(&chip->base, 8, "SCL", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 9, "OC1A", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 10, "OC1B", R01S_PIN_OUT);
    for (i = 0; i < 8; i++) {
        r01s_entity_add_pin(&chip->base, 11 + i, pad_names[i], R01S_PIN_IN);
    }
    r01s_entity_add_pin(&chip->base, 19, "VSYNC", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 20, "VCC", R01S_PIN_PWR);
    r01s_entity_add_pin(&chip->base, 40, "GND", R01S_PIN_PWR);
    r01s_entity_set_dip(&chip->base, 40);
    r01s_entity_reset(&chip->base);
}

R01sEntity *r01ns_atmega1284p_nano_entity(R01nsAtmega1284pNano *chip) {
    return chip ? &chip->base : NULL;
}

void r01ns_atmega1284p_nano_bind(R01nsAtmega1284pNano *chip,
                                 R01nsSst25 *flash,
                                 R01sI2cEeprom *eeprom,
                                 R01nsPwm *pwm,
                                 R01nsVideoSink *sink,
                                 R01sPads *pads) {
    if (!chip) {
        return;
    }
    chip->flash = flash;
    chip->eeprom = eeprom;
    chip->pwm = pwm;
    chip->sink = sink;
    chip->pads = pads;
}

int r01ns_atmega1284p_nano_boot(R01nsAtmega1284pNano *chip) {
    if (!chip || !chip->flash) {
        if (chip) {
            snprintf(chip->err, sizeof(chip->err), "mcu not wired");
        }
        return -1;
    }
    if (chip->flash->image_len < 16) {
        snprintf(chip->err, sizeof(chip->err), "SST25 empty — load cart first");
        return -1;
    }
    if (r01ne_machine_boot_mem(&chip->machine, chip->flash->mem, chip->flash->image_len, chip->err,
                               sizeof(chip->err)) != 0) {
        chip->booted = 0;
        return -1;
    }
    (void)model_map_spi_refill(chip, chip->machine.video.screen_col, chip->machine.video.screen_row);
    chip->last_screen = (uint8_t)((chip->machine.video.screen_col & 0x0f) |
                                  ((chip->machine.video.screen_row & 0x0f) << 4));
    chip->booted = 1;
    chip->err[0] = '\0';
    present_frame(chip);
    return 0;
}

void r01ns_atmega1284p_nano_vblank(R01nsAtmega1284pNano *chip) {
    if (!chip) {
        return;
    }
    if (r01s_level_is_low(r01s_entity_sense(&chip->base, "RESET#"))) {
        return;
    }
    mcu_firmware_frame(chip);
}

const char *r01ns_atmega1284p_nano_err(const R01nsAtmega1284pNano *chip) {
    return chip ? chip->err : "null mcu";
}
