#include "retr01_nano_sim/board.h"

#include "retr01_sim/bus.h"
#include "retr01_sim/health.h"

#include <stdio.h>
#include <string.h>

/* Crystal half-period ns @ 20 MHz (nano/docs: 20 MHz preferred). OSC entity still toggles. */
#define R01NS_XTAL_HALF_NS 25ull

static R01nsBoard *board_from_group(R01sIslandGroup *group) {
    return group ? (R01nsBoard *)group->impl : NULL;
}

R01nsBoard *r01ns_board_from_group(R01sIslandGroup *group) {
    return board_from_group(group);
}

R01sIslandGroup *r01ns_board_group(R01nsBoard *board) {
    return board ? r01s_island_builder_group(&board->builder) : NULL;
}

static void wire_power(R01nsBoard *b) {
    R01sLevel vdd;
    r01s_entity_drive(&b->pwr.base, "VIN", R01S_LVL_H);
    r01s_entity_drive(&b->pwr.base, "EN", R01S_LVL_H);
    r01s_entity_eval(&b->pwr.base);
    vdd = r01s_entity_sense(&b->pwr.base, "VDD");

    r01s_entity_drive(&b->osc.base, "VDD", vdd);
    r01s_entity_drive(&b->osc.base, "OE#", R01S_LVL_Z);

    r01s_entity_drive(&b->mcu.base, "VCC", vdd);
    r01s_entity_drive(&b->mcu.base, "AVCC", vdd);
    r01s_entity_drive(&b->mcu.base, "RESET#", r01s_level_is_high(vdd) ? R01S_LVL_H : R01S_LVL_L);
    r01s_entity_drive(&b->flash.base, "VCC", vdd);
    r01s_entity_drive(&b->flash.base, "WP#", R01S_LVL_H);
    r01s_entity_drive(&b->flash.base, "HOLD#", R01S_LVL_H);
    r01s_entity_drive(&b->eeprom.base, "VCC", vdd);
    r01s_entity_drive(&b->eeprom.base, "WP#", R01S_LVL_H);
    r01s_entity_drive(&b->pwm.base, "VCC", vdd);
    r01s_entity_drive(&b->sink.base, "VCC", vdd);
}

static void wire_clock(R01nsBoard *b) {
    /* Board crystal stand-in → XTAL1 (pinmap). OSC PHI2 pin is the toggling rail. */
    R01sLevel xtal = r01s_entity_sense(&b->osc.base, "PHI2");
    r01s_entity_drive(&b->mcu.base, "XTAL1", xtal);
}

static void wire_spi(R01nsBoard *b) {
    /* PB4/5/6/7 ↔ 25LC1024 CS#/SI/SO/SCK (nano/docs/pinmap.md). */
    r01s_entity_drive(&b->flash.base, "CE#", r01s_entity_sense(&b->mcu.base, "PB4"));
    r01s_entity_drive(&b->flash.base, "SCK", r01s_entity_sense(&b->mcu.base, "PB7"));
    r01s_entity_drive(&b->flash.base, "SI", r01s_entity_sense(&b->mcu.base, "PB5"));
    r01s_entity_drive(&b->mcu.base, "PB6", r01s_entity_sense(&b->flash.base, "SO"));
}

static void wire_i2c(R01nsBoard *b) {
    r01s_entity_drive(&b->eeprom.base, "SCL", r01s_entity_sense(&b->mcu.base, "PC0"));
    r01s_entity_drive(&b->eeprom.base, "SDA", r01s_entity_sense(&b->mcu.base, "PC1"));
}

static void wire_video(R01nsBoard *b) {
    r01s_entity_drive(&b->sink.base, "HSYNC", r01s_entity_sense(&b->mcu.base, "PD0"));
    r01s_entity_drive(&b->sink.base, "VSYNC", r01s_entity_sense(&b->mcu.base, "PD1"));
}

static void wire_pwm(R01nsBoard *b) {
    r01s_entity_drive(&b->pwm.base, "PWM0", r01s_entity_sense(&b->mcu.base, "PD5"));
    r01s_entity_drive(&b->pwm.base, "PWM1", r01s_entity_sense(&b->mcu.base, "PD4"));
}

static void board_wire(R01sIslandGroup *group) {
    R01nsBoard *b = board_from_group(group);
    if (!b) {
        return;
    }
    wire_power(b);
    wire_clock(b);
    wire_spi(b);
    wire_i2c(b);
    wire_video(b);
    wire_pwm(b);
    r01s_pads_refresh_preview(&b->pads);
    r01s_entity_eval(&b->mcu.base);
}

static void board_settle(R01nsBoard *b) {
    int pass;
    for (pass = 0; pass < 2; pass++) {
        board_wire(&b->builder.group);
        r01s_entity_eval(&b->pwr.base);
        r01s_entity_eval(&b->osc.base);
        r01s_entity_eval(&b->mcu.base);
        r01s_entity_eval(&b->pads.base);
        r01s_entity_eval(&b->pwm.base);
        r01s_entity_eval(&b->flash.base);
        r01s_entity_eval(&b->eeprom.base);
        r01s_entity_eval(&b->sink.base);
    }
}

static void board_step(R01sIslandGroup *group) {
    R01nsBoard *b = board_from_group(group);
    if (!b) {
        return;
    }
    /* One PHI2 half + one RGBS scanline (or VBlank line) of the video kernel. */
    b->sim_ns += R01NS_XTAL_HALF_NS;
    b->steps++;
    board_settle(b);
    r01s_entity_tick(&b->osc.base);
    board_settle(b);
    r01s_entity_tick(&b->mcu.base);
    r01ns_atmega1284p_nano_kernel_tick(&b->mcu);
    board_settle(b);
}

static void board_eval_idle(R01sIslandGroup *group) {
    R01nsBoard *b = board_from_group(group);
    if (!b) {
        return;
    }
    board_settle(b);
}

static void board_reset(R01sIslandGroup *group) {
    R01nsBoard *b = board_from_group(group);
    int i;
    if (!b) {
        return;
    }
    b->sim_ns = 0;
    b->steps = 0;
    for (i = 0; i < group->island_count; i++) {
        r01s_island_reset(group->islands[i]);
    }
    board_settle(b);
}

static void board_shutdown(R01sIslandGroup *group) {
    R01nsBoard *b = board_from_group(group);
    if (!b) {
        return;
    }
    r01ne_machine_shutdown(&b->mcu.machine);
    b->mcu.booted = 0;
}

static void board_status(R01sIslandGroup *group, char *buf, size_t buf_len) {
    R01nsBoard *b = board_from_group(group);
    if (!buf || buf_len < 1) {
        return;
    }
    if (!b) {
        snprintf(buf, buf_len, "no board");
        return;
    }
    snprintf(buf, buf_len,
             "%s  steps=%u  fields=%u  line=%d/%d%s  SPI_MAP=%uB  PHI2=%s",
             group->running ? "RUN" : "PAUSE", b->steps, b->mcu.frames, b->mcu.field_line,
             R01NS_FIELD_LINES, b->mcu.in_vblank ? " VB" : "", b->mcu.map_bytes_spi,
             r01s_level_is_high(r01s_entity_sense(&b->osc.base, "PHI2")) ? "H" : "L");
}

static void board_update_probes(R01sIslandGroup *group, int *probe_vdd, int *probe_phi2,
                                int *probe_resb_low) {
    R01nsBoard *b = board_from_group(group);
    if (!b) {
        return;
    }
    if (probe_vdd) {
        *probe_vdd = r01s_level_is_high(r01s_entity_sense(&b->pwr.base, "VDD"));
    }
    if (probe_phi2) {
        *probe_phi2 = r01s_level_is_high(r01s_entity_sense(&b->osc.base, "PHI2"));
    }
    if (probe_resb_low) {
        *probe_resb_low = r01s_level_is_low(r01s_entity_sense(&b->mcu.base, "RESET#"));
    }
}

static void board_fill_health(R01sIslandGroup *group, R01sSystemHealth *out) {
    R01nsBoard *b = board_from_group(group);
    if (!out) {
        return;
    }
    memset(out, 0, sizeof(*out));
    if (!b) {
        out->system = R01S_HEALTH_FAIL;
        snprintf(out->system_label, sizeof(out->system_label), "NO BOARD");
        return;
    }
    out->island_count = 3;
    out->islands[0].letter = 'V';
    out->islands[0].health = R01S_HEALTH_OK;
    snprintf(out->islands[0].activity, sizeof(out->islands[0].activity), "SCR %ux%u",
             (unsigned)R01NS_VIDEO_W, (unsigned)R01NS_VIDEO_H);
    out->islands[1].letter = 'M';
    out->islands[1].health = b->mcu.booted ? R01S_HEALTH_OK : R01S_HEALTH_BOOT;
    snprintf(out->islands[1].activity, sizeof(out->islands[1].activity), "frames=%u", b->mcu.frames);
    out->islands[2].letter = 'C';
    out->islands[2].health = b->flash.image_len > 0 ? R01S_HEALTH_OK : R01S_HEALTH_WARN;
    snprintf(out->islands[2].activity, sizeof(out->islands[2].activity), "SPI %uB",
             b->mcu.map_bytes_spi);
    out->system = b->mcu.booted ? R01S_HEALTH_OK : R01S_HEALTH_BOOT;
    snprintf(out->system_label, sizeof(out->system_label), "%s", b->mcu.booted ? "NANO OK" : "BOOT");
    snprintf(out->system_detail, sizeof(out->system_detail), "1284 behavioral + 25LC1024");
}

static const R01sIslandGroupVTable BOARD_GROUP_VT = {
    board_shutdown, board_reset, board_wire, board_step, board_eval_idle,
    board_status,   board_update_probes, board_fill_health,
};

static void island_noop_init(R01sIsland *island) {
    (void)island;
}
static void island_noop_shutdown(R01sIsland *island) {
    (void)island;
}
static void island_noop_reset(R01sIsland *island) {
    int i;
    for (i = 0; i < island->entity_count; i++) {
        if (island->entities[i]) {
            r01s_entity_reset(island->entities[i]);
        }
    }
}
static void island_noop_eval(R01sIsland *island) {
    int i;
    for (i = 0; i < island->entity_count; i++) {
        if (island->entities[i]) {
            r01s_entity_eval(island->entities[i]);
        }
    }
}
static void island_noop_tick(R01sIsland *island) {
    (void)island;
}

static const R01sIslandVTable ISLAND_VT = {island_noop_init, island_noop_shutdown, island_noop_reset,
                                          island_noop_eval, island_noop_tick};

int r01ns_board_build(R01nsBoard *board) {
    int iv, im, ic;
    if (!board) {
        return -1;
    }
    memset(board, 0, sizeof(*board));

    r01s_pwr5v_init(&board->pwr, "PS1");
    r01s_osc8m_init(&board->osc, "Y1");
    r01ns_atmega1284p_nano_init(&board->mcu, "U1");
    r01s_pads_init(&board->pads, "J1");
    r01ns_pwm_init(&board->pwm, "U30");
    r01ns_sst25_init(&board->flash, "U20");
    r01s_i2c_eeprom_init(&board->eeprom, "U50");
    r01ns_video_sink_init(&board->sink, "SCR1");

    r01ns_atmega1284p_nano_bind(&board->mcu, &board->flash, &board->eeprom, &board->pwm, &board->sink,
                                &board->pads);

    r01s_island_builder_init(&board->builder);
    r01s_island_builder_bind(&board->builder, &BOARD_GROUP_VT, board);

    iv = r01s_island_builder_add(&board->builder, &ISLAND_VT, "VIDEO", 0, 0, 200, 160, board);
    im = r01s_island_builder_add(&board->builder, &ISLAND_VT, "MCU", 0, 0, 280, 200, board);
    ic = r01s_island_builder_add(&board->builder, &ISLAND_VT, "CART", 0, 0, 200, 140, board);
    if (iv < 0 || im < 0 || ic < 0) {
        return -1;
    }

    r01s_island_builder_mount_rel(&board->builder, r01ns_video_sink_entity(&board->sink), iv, 8, 8);

    r01s_island_builder_mount_rel(&board->builder, r01s_pwr5v_entity(&board->pwr), im, 8, 8);
    r01s_island_builder_mount_rel(&board->builder, r01s_osc8m_entity(&board->osc), im, 48, 8);
    r01s_island_builder_mount_rel(&board->builder, r01ns_atmega1284p_nano_entity(&board->mcu), im, 8, 60);
    r01s_island_builder_mount_rel(&board->builder, r01s_pads_entity(&board->pads), im, 160, 8);
    r01s_island_builder_mount_rel(&board->builder, r01ns_pwm_entity(&board->pwm), im, 160, 80);

    r01s_island_builder_mount_rel(&board->builder, r01ns_sst25_entity(&board->flash), ic, 8, 8);
    r01s_island_builder_mount_rel(&board->builder, r01s_i2c_eeprom_entity(&board->eeprom), ic, 80, 8);

    r01s_island_builder_fit_all(&board->builder);
    r01s_island_builder_arrange_rows(&board->builder, 16, 24, 24, 24, 620);

    if (r01s_island_builder_finish(&board->builder) != 0) {
        return -1;
    }
    board->builder.group.running = 1;
    board->builder.group.powered = 1;
    return 0;
}

void r01ns_board_shutdown(R01nsBoard *board) {
    if (!board) {
        return;
    }
    r01s_island_builder_shutdown(&board->builder);
}

int r01ns_board_load_cart(R01nsBoard *board, const char *path, char *err, size_t err_cap) {
    if (!board || !path) {
        if (err && err_cap) {
            snprintf(err, err_cap, "bad args");
        }
        return -1;
    }
    if (r01ns_sst25_load_path(&board->flash, path, err, err_cap) != 0) {
        return -1;
    }
    snprintf(board->cart_path, sizeof(board->cart_path), "%s", path);
    return 0;
}

int r01ns_board_boot(R01nsBoard *board) {
    if (!board) {
        return -1;
    }
    board_settle(board);
    return r01ns_atmega1284p_nano_boot(&board->mcu);
}

void r01ns_board_set_pads(R01nsBoard *board, uint8_t p1, uint8_t p2) {
    if (!board) {
        return;
    }
    r01s_pads_set(&board->pads, 0, p1);
    r01s_pads_set(&board->pads, 1, p2);
    r01s_pads_refresh_preview(&board->pads);
}
