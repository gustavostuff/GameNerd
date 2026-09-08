# Nano pin map (provisional freeze)

**Status:** sim + SKiDL SoT until the first schematic locks.  
Package on PCB: **TQFP-44** ([`hardware.md`](hardware.md)). Sim DIP-40 uses the same **port.bit** names; package pin numbers differ — always wire by **net / PORT**, not by DIP number alone.

Reference pinout: [`hw/md/ATmega1284P.md`](../../hw/md/ATmega1284P.md) (PDIP-40 table). TQFP-44 mapping must match the Microchip datasheet when Quilter/SKiDL symbols are chosen.

## Console MCU — net ↔ PORT

| Net | ATmega1284P | Notes |
|-----|-------------|-------|
| `SPI_SS#` | **PB4** (SS) | Cart flash CE# |
| `SPI_MOSI` | **PB5** | Cart SI |
| `SPI_MISO` | **PB6** | Cart SO |
| `SPI_SCK` | **PB7** | Cart SCK |
| `I2C_SCL` | **PC0** | Cart 24C64 |
| `I2C_SDA` | **PC1** | Cart 24C64 (open-drain + pull-ups on mobo) |
| `PWM_MUSIC` | **PD5** (OC1A) | → series R mix |
| `PWM_SFX` | **PD4** (OC1B) | → series R mix |
| `HSYNC` | **PD0** | RGBS |
| `VSYNC` | **PD1** | RGBS |
| `P1_D0..D7` | **PA0..PA7** | Arcade P1 bitfield (same spirit as `$FE60`) |
| `P2_D0..D7` | **PC2..PC7**, **PD2**, **PD3** | Arcade P2 (`$FE61` spirit) |
| `XTAL1` / `XTAL2` | XTAL1 / XTAL2 | **20 MHz** crystal + load caps |
| `RESET#` | RESET | ISP + reset |
| `VCC` / `GND` / `AVCC` / `AREF` | power | Decoupling per datasheet |
| `ISP_MOSI/MISO/SCK/RESET` | shared SPI + RESET | 2×3 ISP; cart SS# idle high while ISP |

### FG color (bring-up stub)

Exact resistor-DAC packing is **not frozen**. Sim currently presents composed RGB into `SCREEN_SINK` in software (firmware would drive FG GPIOs during the shift loop). Candidate for schematic v1:

| Net | Candidate | Role |
|-----|-----------|------|
| `FG0..FG2` | **PB0..PB2** | 3-bit FG index (8 colors) during active pixels |
| (backdrop) | grounded black | No DAC code for backdrop |

Revisit when the video kernel shift-out is implemented in open FW.

## Cart edge (≤8 pads/side budget)

| Net | Cart IC pin | Notes |
|-----|-------------|-------|
| `SPI_SS#` | SST25 CE# | |
| `SPI_SCK` | SCK | |
| `SPI_MOSI` | SI | |
| `SPI_MISO` | SO | |
| `I2C_SDA` | 24C64 SDA | |
| `I2C_SCL` | 24C64 SCL | |
| `VCC` | both | |
| `GND` | both | |
| Flash `HOLD#` / `WP#` | tied on cart | Saves edge pads |

Detect / extra WP on the edge: only if pad count allows after first cart outline.

## What Nano Sim models today

| Layer | Fidelity |
|-------|----------|
| Islands / wire settle / pin levels | Yes — nets above |
| RGBS scanline kernel (line time) | Yes — one board step / RGBS line |
| Host Play + soft compose | Behavioral (emu core = FW SRAM compose) |
| SPI MAP refill in VBlank | Byte-clocked on pins during VBlank |
| AVR instruction ISA | **No** |
| Pixel-timed FG GPIO shift | **No** (sink gets composed RGB) |
| I2C bit-bang protocol | Stub pins only |
| PWM softsynth | Duty stubs on OC1A/OC1B |

## SKiDL / Quilter handoff

1. Freeze this table in the first `nano` SKiDL module (mobo + cart).
2. Symbols: ATmega1284P **TQFP-44**, SST25VF010A SOIC-8, 24C64 SOIC-8.
3. Export netlists → Quilter; keep arcade headers / RGBS / ISP as board outline constraints.
4. When package pin numbers are locked, add a TQFP column here and keep PORT names stable.
