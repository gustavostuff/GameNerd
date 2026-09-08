#ifndef RETR01_NANO_SIM_SST25_H
#define RETR01_NANO_SIM_SST25_H

#include "retr01_sim/entity.h"

#include <stddef.h>
#include <stdint.h>

#define R01NS_SST25_SIZE (128u * 1024u)

/*
 * SST25VF010A SPI flash (cart image). 8-pin package:
 *   1 CE#  2 SO  3 WP#  4 GND  5 SI  6 SCK  7 HOLD#  8 VCC
 * Behavioral: board wire + SPI READ helper; not bit-bang protocol complete.
 */
typedef struct R01nsSst25 {
    R01sEntity base;
    uint8_t mem[R01NS_SST25_SIZE];
    size_t image_len;
    uint32_t spi_bytes_read;
    uint32_t spi_txns;
} R01nsSst25;

void r01ns_sst25_init(R01nsSst25 *chip, const char *refdes);
R01sEntity *r01ns_sst25_entity(R01nsSst25 *chip);

int r01ns_sst25_load_path(R01nsSst25 *chip, const char *path, char *err, size_t err_cap);
/* Single-byte SPI READ pulse (updates CE#/SCK/SI/SO). */
int r01ns_sst25_spi_read_byte(R01nsSst25 *chip, uint32_t addr, uint8_t *out);

#endif
