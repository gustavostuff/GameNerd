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

/* P1 = PA0..PA7. P2 = PC2..PC7, PD2, PD3 (nano/docs/pinmap.md). */
static const char *const P1_PINS[8] = {"PA0", "PA1", "PA2", "PA3", "PA4", "PA5", "PA6", "PA7"};
static const char *const P2_PINS[8] = {"PC2", "PC3", "PC4", "PC5", "PC6", "PC7", "PD2", "PD3"};

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
    r01s_entity_drive(&c->base, "PB4", ss);  /* SS# */
    r01s_entity_drive(&c->base, "PB7", sck); /* SCK */
    r01s_entity_drive(&c->base, "PB5", mosi); /* MOSI */
}

static void spi_begin_map_refill(R01nsAtmega1284pNano *c, int col, int row) {
    uint32_t abs = screen_payload_abs(&c->machine, col, row);
    if (!c->flash || abs == 0) {
        c->spi_active = 0;
        return;
    }
    c->spi_active = 1;
    c->spi_addr = abs;
    c->spi_off = 0;
    c->spi_len = R01NS_MAP_PAYLOAD;
    spi_drive_mcu(c, R01S_LVL_L, R01S_LVL_L, R01S_LVL_H);
    c->vblank_refills++;
}

static void spi_clock_bytes(R01nsAtmega1284pNano *c, uint32_t nbytes) {
    uint32_t i;
    if (!c->spi_active || !c->flash) {
        return;
    }
    for (i = 0; i < nbytes && c->spi_off < c->spi_len; i++) {
        uint8_t b = 0;
        if (r01ns_sst25_spi_read_byte(c->flash, c->spi_addr + c->spi_off, &b) != 0) {
            break;
        }
        r01s_entity_drive(&c->base, "PB6", r01s_entity_sense(&c->flash->base, "SO")); /* MISO */
        r01s_entity_drive(&c->base, "PB7", R01S_LVL_H);
        r01s_entity_drive(&c->base, "PB7", R01S_LVL_L);
        c->spi_off++;
        c->map_bytes_spi++;
    }
    if (c->spi_off >= c->spi_len) {
        c->spi_active = 0;
        spi_drive_mcu(c, R01S_LVL_H, R01S_LVL_L, R01S_LVL_L);
        r01s_entity_drive(&c->flash->base, "CE#", R01S_LVL_H);
        r01s_entity_drive(&c->flash->base, "SO", R01S_LVL_Z);
        c->flash->spi_txns++;
    }
}

static uint8_t read_port_byte(R01nsAtmega1284pNano *c, const char *const *names) {
    uint8_t pad = 0;
    int i;
    for (i = 0; i < 8; i++) {
        if (r01s_level_is_high(r01s_entity_sense(&c->base, names[i]))) {
            pad |= (uint8_t)(1u << i);
        }
    }
    return pad;
}

static void vblank_enter_services(R01nsAtmega1284pNano *c) {
    uint8_t before;
    uint8_t after;

    /* P1 drives Host Play for now (P2 sensed on pins for netlist / future). */
    r01ne_play_set_pad(&c->machine, read_port_byte(c, P1_PINS));
    before = c->last_screen;
    r01ne_machine_frame(&c->machine);
    after = (uint8_t)((c->machine.video.screen_col & 0x0f) |
                      ((c->machine.video.screen_row & 0x0f) << 4));
    if (after != before) {
        spi_begin_map_refill(c, c->machine.video.screen_col, c->machine.video.screen_row);
        c->last_screen = after;
    }
    if (c->pwm) {
        r01s_entity_tick(&c->pwm->base);
        r01s_entity_drive(&c->base, "PD5", r01s_entity_sense(&c->pwm->base, "PWM0")); /* OC1A */
        r01s_entity_drive(&c->base, "PD4", r01s_entity_sense(&c->pwm->base, "PWM1")); /* OC1B */
    }
    r01s_entity_drive(&c->base, "PD1", R01S_LVL_H); /* VSYNC */
    if (c->sink) {
        r01ns_video_sink_on_vblank(c->sink);
    }
    c->frames++;
}

static void emit_active_line(R01nsAtmega1284pNano *c, int rgbs_y) {
    r01s_entity_drive(&c->base, "PD1", R01S_LVL_L); /* VSYNC inactive */
    r01s_entity_drive(&c->base, "PD0", R01S_LVL_L); /* HSYNC pulse edge */
    r01s_entity_drive(&c->base, "PD0", R01S_LVL_H);
    if (c->sink) {
        r01ns_video_sink_plot_line_from_fb(c->sink, rgbs_y, c->machine.video.fb);
    }
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
    c->field_line = 0;
    c->in_vblank = 0;
    c->need_compose = 1;
    c->spi_active = 0;
    c->err[0] = '\0';
    spi_drive_mcu(c, R01S_LVL_H, R01S_LVL_L, R01S_LVL_L);
    r01s_entity_drive(e, "PB6", R01S_LVL_Z);
    r01s_entity_drive(e, "PC1", R01S_LVL_Z);
    r01s_entity_drive(e, "PC0", R01S_LVL_L);
    r01s_entity_drive(e, "PD5", R01S_LVL_L);
    r01s_entity_drive(e, "PD4", R01S_LVL_L);
    r01s_entity_drive(e, "PD0", R01S_LVL_L);
    r01s_entity_drive(e, "PD1", R01S_LVL_L);
}

static void drive_pad_port(R01sEntity *e, const char *const *names, uint8_t bits) {
    int i;
    for (i = 0; i < 8; i++) {
        r01s_entity_drive(e, names[i], (bits & (1u << i)) ? R01S_LVL_H : R01S_LVL_L);
    }
}

static void mcu_eval(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    if (c->pads) {
        drive_pad_port(e, P1_PINS, r01s_pads_get(c->pads, 0));
        drive_pad_port(e, P2_PINS, r01s_pads_get(c->pads, 1));
    }
}

static void mcu_tick(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    c->prev_phi2_high = r01s_level_is_high(r01s_entity_sense(e, "XTAL1"));
}

static void mcu_destroy(R01sEntity *e) {
    R01nsAtmega1284pNano *c = (R01nsAtmega1284pNano *)e;
    r01ne_machine_shutdown(&c->machine);
}

static const R01sEntityVTable MCU_VT = {mcu_reset, mcu_eval, mcu_tick, mcu_destroy};

void r01ns_atmega1284p_nano_init(R01nsAtmega1284pNano *chip, const char *refdes) {
    /* PDIP-40 signal order from hw/md/ATmega1284P.md (sim package). */
    static const struct {
        int num;
        const char *name;
        R01sPinDir dir;
    } pins[] = {
        {1, "PB0", R01S_PIN_IO},   {2, "PB1", R01S_PIN_IO},   {3, "PB2", R01S_PIN_IO},
        {4, "PB3", R01S_PIN_IO},   {5, "PB4", R01S_PIN_OUT},  {6, "PB5", R01S_PIN_OUT},
        {7, "PB6", R01S_PIN_IN},   {8, "PB7", R01S_PIN_OUT},  {9, "RESET#", R01S_PIN_IN},
        {10, "VCC", R01S_PIN_PWR}, {11, "GND", R01S_PIN_PWR}, {12, "XTAL2", R01S_PIN_OUT},
        {13, "XTAL1", R01S_PIN_IN},{14, "PD0", R01S_PIN_OUT}, {15, "PD1", R01S_PIN_OUT},
        {16, "PD2", R01S_PIN_IN},  {17, "PD3", R01S_PIN_IN},  {18, "PD4", R01S_PIN_OUT},
        {19, "PD5", R01S_PIN_OUT}, {20, "PD6", R01S_PIN_IO},  {21, "PD7", R01S_PIN_IO},
        {22, "PC0", R01S_PIN_OUT}, {23, "PC1", R01S_PIN_IO},  {24, "PC2", R01S_PIN_IN},
        {25, "PC3", R01S_PIN_IN},  {26, "PC4", R01S_PIN_IN},  {27, "PC5", R01S_PIN_IN},
        {28, "PC6", R01S_PIN_IN},  {29, "PC7", R01S_PIN_IN},  {30, "AVCC", R01S_PIN_PWR},
        {31, "GND", R01S_PIN_PWR}, {32, "AREF", R01S_PIN_PWR},{33, "PA7", R01S_PIN_IN},
        {34, "PA6", R01S_PIN_IN},  {35, "PA5", R01S_PIN_IN},  {36, "PA4", R01S_PIN_IN},
        {37, "PA3", R01S_PIN_IN},  {38, "PA2", R01S_PIN_IN},  {39, "PA1", R01S_PIN_IN},
        {40, "PA0", R01S_PIN_IN},
    };
    size_t i;
    if (!chip) {
        return;
    }
    memset(chip, 0, sizeof(*chip));
    r01s_entity_init(&chip->base, &MCU_VT, "ATMEGA1284P", refdes ? refdes : "U1");
    chip->base.impl = chip;
    for (i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
        r01s_entity_add_pin(&chip->base, pins[i].num, pins[i].name, pins[i].dir);
    }
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
        snprintf(chip->err, sizeof(chip->err), "cart SPI empty — load cart first");
        return -1;
    }
    if (r01ne_machine_boot_mem(&chip->machine, chip->flash->mem, chip->flash->image_len, chip->err,
                               sizeof(chip->err)) != 0) {
        chip->booted = 0;
        return -1;
    }
    spi_begin_map_refill(chip, chip->machine.video.screen_col, chip->machine.video.screen_row);
    while (chip->spi_active) {
        spi_clock_bytes(chip, R01NS_SPI_BYTES_PER_VB_STEP);
    }
    chip->last_screen = (uint8_t)((chip->machine.video.screen_col & 0x0f) |
                                  ((chip->machine.video.screen_row & 0x0f) << 4));
    chip->booted = 1;
    chip->field_line = 0;
    chip->in_vblank = 0;
    chip->need_compose = 0;
    chip->err[0] = '\0';
    if (chip->sink) {
        r01ns_video_sink_blit_rgb(chip->sink, chip->machine.video.fb, sizeof(chip->machine.video.fb));
    }
    return 0;
}

void r01ns_atmega1284p_nano_kernel_tick(R01nsAtmega1284pNano *chip) {
    if (!chip || !chip->booted) {
        return;
    }
    if (r01s_level_is_low(r01s_entity_sense(&chip->base, "RESET#"))) {
        return;
    }

    if (chip->field_line < R01NS_RGBS_ACTIVE_LINES) {
        chip->in_vblank = 0;
        emit_active_line(chip, chip->field_line);
        chip->field_line++;
        return;
    }

    chip->in_vblank = 1;
    if (chip->field_line == R01NS_RGBS_ACTIVE_LINES) {
        vblank_enter_services(chip);
    } else {
        r01s_entity_drive(&chip->base, "PD1", R01S_LVL_H);
        spi_clock_bytes(chip, R01NS_SPI_BYTES_PER_VB_STEP);
    }

    chip->field_line++;
    if (chip->field_line >= R01NS_FIELD_LINES) {
        chip->field_line = 0;
        chip->in_vblank = 0;
        while (chip->spi_active) {
            spi_clock_bytes(chip, R01NS_SPI_BYTES_PER_VB_STEP);
        }
    }
}

const char *r01ns_atmega1284p_nano_err(const R01nsAtmega1284pNano *chip) {
    return chip ? chip->err : "null mcu";
}
