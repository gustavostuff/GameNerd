#include "sst25vf010a.h"

#include "retr01_sim/bus.h"

#include <stdio.h>
#include <string.h>

static void flash_reset(R01sEntity *e) {
    R01nsSst25 *c = (R01nsSst25 *)e;
    r01s_entity_drive(e, "SO", R01S_LVL_Z);
    r01s_entity_drive(e, "CE#", R01S_LVL_H);
    (void)c;
}

static void flash_eval(R01sEntity *e) {
    (void)e;
}

static void flash_tick(R01sEntity *e) {
    (void)e;
}

static void flash_destroy(R01sEntity *e) {
    (void)e;
}

static const R01sEntityVTable FLASH_VT = {flash_reset, flash_eval, flash_tick, flash_destroy};

void r01ns_sst25_init(R01nsSst25 *chip, const char *refdes) {
    if (!chip) {
        return;
    }
    memset(chip, 0, sizeof(*chip));
    r01s_entity_init(&chip->base, &FLASH_VT, "25LC1024", refdes ? refdes : "U25");
    chip->base.impl = chip;
    r01s_entity_add_pin(&chip->base, 1, "CE#", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 2, "SO", R01S_PIN_OUT);
    r01s_entity_add_pin(&chip->base, 3, "WP#", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 4, "GND", R01S_PIN_PWR);
    r01s_entity_add_pin(&chip->base, 5, "SI", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 6, "SCK", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 7, "HOLD#", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 8, "VCC", R01S_PIN_PWR);
    r01s_entity_set_dip(&chip->base, 8);
    memset(chip->mem, 0xFF, sizeof(chip->mem));
    r01s_entity_reset(&chip->base);
}

R01sEntity *r01ns_sst25_entity(R01nsSst25 *chip) {
    return chip ? &chip->base : NULL;
}

int r01ns_sst25_load_path(R01nsSst25 *chip, const char *path, char *err, size_t err_cap) {
    FILE *fp;
    size_t n;
    if (!chip || !path) {
        if (err && err_cap) {
            snprintf(err, err_cap, "bad args");
        }
        return -1;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        if (err && err_cap) {
            snprintf(err, err_cap, "open failed: %s", path);
        }
        return -1;
    }
    memset(chip->mem, 0xFF, sizeof(chip->mem));
    n = fread(chip->mem, 1, sizeof(chip->mem), fp);
    fclose(fp);
    if (n < 16) {
        if (err && err_cap) {
            snprintf(err, err_cap, "cart too small (%zu)", n);
        }
        return -1;
    }
    chip->image_len = n;
    chip->spi_bytes_read = 0;
    chip->spi_txns = 0;
    return 0;
}

int r01ns_sst25_spi_read_byte(R01nsSst25 *chip, uint32_t addr, uint8_t *out) {
    uint8_t b;
    int bit;
    if (!chip || !out || addr >= R01NS_SST25_SIZE) {
        return -1;
    }
    b = chip->mem[addr];
    r01s_entity_drive(&chip->base, "CE#", R01S_LVL_L);
    for (bit = 7; bit >= 0; bit--) {
        r01s_entity_drive(&chip->base, "SI", R01S_LVL_L);
        r01s_entity_drive(&chip->base, "SCK", R01S_LVL_L);
        r01s_entity_drive(&chip->base, "SO", (b & (1u << bit)) ? R01S_LVL_H : R01S_LVL_L);
        r01s_entity_drive(&chip->base, "SCK", R01S_LVL_H);
    }
    r01s_entity_drive(&chip->base, "SCK", R01S_LVL_L);
    *out = b;
    chip->spi_bytes_read++;
    return 0;
}

int r01ns_sst25_spi_read(R01nsSst25 *chip, uint32_t addr, uint8_t *dst, size_t len) {
    size_t i;
    if (!chip || !dst || len < 1) {
        return -1;
    }
    if ((size_t)addr + len > R01NS_SST25_SIZE) {
        return -1;
    }
    for (i = 0; i < len; i++) {
        if (r01ns_sst25_spi_read_byte(chip, addr + (uint32_t)i, &dst[i]) != 0) {
            r01s_entity_drive(&chip->base, "CE#", R01S_LVL_H);
            r01s_entity_drive(&chip->base, "SO", R01S_LVL_Z);
            return -1;
        }
    }
    r01s_entity_drive(&chip->base, "CE#", R01S_LVL_H);
    r01s_entity_drive(&chip->base, "SO", R01S_LVL_Z);
    chip->spi_txns++;
    return 0;
}
