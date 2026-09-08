# Nano pin map (provisional freeze)

**Status:** sim + SKiDL SoT until the first schematic locks.  
Connectors / cart mechanics: [`hardware.md`](hardware.md).  
Package on PCB: **TQFP-44**. Sim DIP-40 uses the same **PORT** names — wire by **net / PORT**, not DIP number.

Reference pinout: [`hw/md/ATmega1284P.md`](../../hw/md/ATmega1284P.md). TQFP-44 mapping follows the Microchip datasheet when the KiCad symbol is chosen.

## Console MCU — net ↔ PORT

| Net | ATmega1284P | Notes |
|-----|-------------|-------|
| `SPI_SS#` | **PB4** (SS) | Cart flash CE# / edge |
| `SPI_MOSI` | **PB5** | Cart SI; shared with **J_ISP** MOSI |
| `SPI_MISO` | **PB6** | Cart SO; shared with **J_ISP** MISO |
| `SPI_SCK` | **PB7** | Cart SCK; shared with **J_ISP** SCK |
| `I2C_SCL` | **PC0** | Cart 24C64 (+ pull-up) |
| `I2C_SDA` | **PC1** | Cart 24C64 (+ pull-up) |
| `PWM_MUSIC` | **PD5** (OC1A) | → mix → `AUD` |
| `PWM_SFX` | **PD4** (OC1B) | → mix → `AUD` |
| `HSYNC` / `VSYNC` | **PD0** / **PD1** | Combined to `CSYNC` for **J_AV** (FW or tiny glue) |
| `P1_D0..D7` | **PA0..PA7** | **J_P1** |
| `P2_D0..D7` | **PC2..PC7**, **PD2**, **PD3** | **J_P2** |
| `XTAL1` / `XTAL2` | XTAL1 / XTAL2 | **20 MHz** + load caps |
| `RESET#` | RESET | **J_ISP** + **J_PWR** |
| `VCC` / `GND` / `AVCC` / `AREF` | power | Decoupling per datasheet |
| `CART_DET#` | (GPIO TBD, e.g. **PB3**) | Mobo pull-up; cart B8 → GND |

### FG color (bring-up stub)

| Net | Candidate | Role |
|-----|-----------|------|
| `FG0..FG2` | **PB0..PB2** | 3-bit FG index during active pixels |
| (backdrop) | grounded black | |

## Cart edge — **8 + 8 locked**

Mechanics and female part: [`hardware.md`](hardware.md#cute-cartridge--locked-8--8).

| Pos | Side A | Side B |
|-----|--------|--------|
| 1 | `GND` | `GND` |
| 2 | `VCC` | `VCC` |
| 3 | `SPI_SS#` | `SPI_SS#` |
| 4 | `SPI_SCK` | `SPI_SCK` |
| 5 | `SPI_MOSI` | `SPI_MOSI` |
| 6 | `SPI_MISO` | `SPI_MISO` |
| 7 | `I2C_SDA` | `I2C_SDA` |
| 8 | `I2C_SCL` | `CART_DET#` |

## Header pinouts (vertical)

### J_AV (1×8)

| Pin | Net |
|-----|-----|
| 1 | `R` |
| 2 | `G` |
| 3 | `B` |
| 4 | `CSYNC` |
| 5 | `AUD` |
| 6 | `AGND` |
| 7 | `VGND` |
| 8 | key / NC |

### J_P1 / J_P2 (1×10 each)

| Pin | Net |
|-----|-----|
| 1–8 | `Px_D0` … `Px_D7` |
| 9 | `GND` |
| 10 | key / NC |

### J_PWR (1×4)

| Pin | Net |
|-----|-----|
| 1 | `+5V` |
| 2 | `GND` |
| 3 | `RESET#` |
| 4 | NC |

### J_ISP (2×3)

Standard AVR ISP: `MOSI`, `MISO`, `SCK`, `RESET#`, `VCC`, `GND` (same assignment as full Retr01 / USBasp).

## What Nano Sim models today

| Layer | Fidelity |
|-------|----------|
| Islands / wire settle / pin levels | Yes — MCU PORT nets above |
| RGBS scanline kernel | Yes — one board step / RGBS line |
| Host Play + soft compose | Behavioral (emu core) |
| SPI MAP refill in VBlank | Byte-clocked on pins |
| Cart 8+8 / headers as entities | Documented; UI still abstract pads |
| AVR ISA / pixel FG DAC | **No** |

## SKiDL / Quilter handoff

1. Import this pinmap + [`hardware.md`](hardware.md) connector table.
2. Symbols: ATmega1284P **TQFP-44**, SST25VF010A, 24C64, PJ-063AH, PinSocket 2×8 / headers.
3. Cart footprint: truncate Retr01 gold-finger recipe to **2×8**.
4. Keep PORT names stable when TQFP pin numbers are filled in.
