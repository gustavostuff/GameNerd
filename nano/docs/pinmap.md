# Nano pin map (provisional freeze)

**Status:** sim + SKiDL SoT until the first schematic locks.  
Connectors / cart mechanics: [`hardware.md`](hardware.md).  
Package on PCB: **TQFP-44**. Sim DIP-40 uses the same **PORT** names — wire by **net / PORT**, not DIP number.

Reference pinout: [`hw/md/ATmega1284P.md`](../../hw/md/ATmega1284P.md).  
**TQFP-44 pad numbers:** KiCad **`ATmega1284P-A`** (extends `ATmega164A-A`). Extract: `nano/schematic_generator/nano_schem/kicad_pin_extract.json`. Generator: [`nano/schematic_generator/`](../schematic_generator/).

## Console MCU — net ↔ PORT ↔ TQFP pin

| Net | PORT | TQFP-44 | Notes |
|-----|------|---------|-------|
| `SPI_SS#` | **PB4** | **44** | Cart flash CE# / edge |
| `SPI_MOSI` | **PB5** | **1** | Cart SI; shared with **J_ISP** MOSI |
| `SPI_MISO` | **PB6** | **2** | Cart SO; shared with **J_ISP** MISO |
| `SPI_SCK` | **PB7** | **3** | Cart SCK; shared with **J_ISP** SCK |
| `I2C_SCL` | **PC0** | **19** | Cart 24C64 (+ pull-up) |
| `I2C_SDA` | **PC1** | **20** | Cart 24C64 (+ pull-up) |
| `PWM_MUSIC` | **PD5** (OC1A) | **14** | → mix → `AUD` |
| `PWM_SFX` | **PD4** (OC1B) | **13** | → mix → `AUD` |
| `HSYNC` / `VSYNC` | **PD0** / **PD1** | **9** / **10** | Resistor-mix → `CSYNC` on **J_AV** (FW XOR later) |
| `P1_D0..D7` | **PA0..PA7** | **37..30** | **J_P1** |
| `P2_D0..D5` | **PC2..PC7** | **21..26** | **J_P2** |
| `P2_D6..D7` | **PD2**, **PD3** | **11**, **12** | **J_P2** |
| `FG0..FG2` | **PB0..PB2** | **40..42** | Bring-up RGB enables |
| `CART_DET#` | **PB3** | **43** | Mobo pull-up; cart B8 → GND |
| `XTAL1` / `XTAL2` | XTAL1 / XTAL2 | **8** / **7** | **20 MHz** + load caps |
| `RESET#` | RESET | **4** | **J_ISP** + **J_PWR** |
| `VCC` | VCC | **5**, **17**, **38** | + decoupling |
| `GND` | GND | **6**, **18**, **28**, **39** | |
| `AVCC` / `AREF` | AVCC / AREF | **27** / **29** | AVCC to +5V; AREF bypass to GND |

### FG color (bring-up stub)

| Net | PORT / TQFP | Role |
|-----|-------------|------|
| `FG0..FG2` | PB0..PB2 / 40..42 | Bring-up: FG0→R, FG1→G, FG2→B via 470R + 75R |
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

1. Generator: [`nano/schematic_generator/`](../schematic_generator/) (`python generate.py`).
2. Symbols locked via KiCad extract (`ATmega1284P-A`, SST25VF080B twin, `24LC64`).
3. Cart footprint: `Retr01_Lib:Cart_Edge_2x8_P2.54mm` (same pad recipe as 2x18).
4. Keep PORT names stable; TQFP numbers are filled in the table above.
5. Import `output/nano_mobo.net` + `output/nano_cart.net` into separate KiCad projects.
