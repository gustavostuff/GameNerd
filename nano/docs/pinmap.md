# Nano pin map (provisional freeze)

**Status:** sim + SKiDL SoT until the first schematic locks.  
Connectors / cart mechanics: [`hardware.md`](hardware.md).  
Package on motherboard: **PDIP-40** (fully THT). Wire by **net / PORT**; DIP numbers match KiCad **`ATmega1284P-P`**.

Reference pinout: [`hw/md/ATmega1284P.md`](../../hw/md/ATmega1284P.md).  
Extract: `nano/schematic_generator/nano_schem/kicad_pin_extract.json`. Generator: [`nano/schematic_generator/`](../schematic_generator/).

## Console MCU — net ↔ PORT ↔ DIP pin

| Net | PORT | DIP-40 | Notes |
|-----|------|--------|-------|
| `SPI_SS#` | **PB4** | **5** | Cart **25LC1024** CS# / edge |
| `SPI_MOSI` | **PB5** | **6** | Cart SI; shared with **J_ISP** MOSI |
| `SPI_MISO` | **PB6** | **7** | Cart SO; shared with **J_ISP** MISO |
| `SPI_SCK` | **PB7** | **8** | Cart SCK; shared with **J_ISP** SCK |
| `I2C_SCL` | **PC0** | **22** | Cart 24C64 (+ pull-up) |
| `I2C_SDA` | **PC1** | **23** | Cart 24C64 (+ pull-up) |
| `PWM_MUSIC` | **PD5** (OC1A) | **19** | → mix → `AUD` |
| `PWM_SFX` | **PD4** (OC1B) | **18** | → mix → `AUD` |
| `HSYNC` / `VSYNC` | **PD0** / **PD1** | **14** / **15** | Resistor-mix → `CSYNC` on **J_AV** (FW XOR later) |
| `P1_D0..D7` | **PA0..PA7** | **40..33** | **J_P1** |
| `P2_D0..D5` | **PC2..PC7** | **24..29** | **J_P2** |
| `P2_D6..D7` | **PD2**, **PD3** | **16**, **17** | **J_P2** |
| `FG0..FG2` | **PB0..PB2** | **1..3** | Bring-up RGB enables |
| `CART_DET#` | **PB3** | **4** | Mobo pull-up; cart B8 → GND |
| `XTAL1` / `XTAL2` | XTAL1 / XTAL2 | **13** / **12** | **20 MHz** + load caps |
| `RESET#` | RESET | **9** | **J_ISP** + **J_PWR** |
| `VCC` | VCC | **10** | + decoupling |
| `GND` | GND | **11**, **31** | |
| `AVCC` / `AREF` | AVCC / AREF | **30** / **32** | AVCC to +5V; AREF bypass to GND |

### FG color (bring-up stub)

| Net | PORT / DIP | Role |
|-----|------------|------|
| `FG0..FG2` | PB0..PB2 / 1..3 | Bring-up: FG0→R, FG1→G, FG2→B via 470R + 75R |
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
2. Symbols: `ATmega1284P-P` PDIP-40; cart **25LC1024** + `24LC64` (both DIP-8).
3. Cart footprint: `Retr01_Lib:Cart_Edge_2x8_P2.54mm` (same pad recipe as 2x18).
4. Keep PORT names stable; DIP numbers are filled in the table above.
5. Import `output/nano_mobo.net` + `output/nano_cart.net` into separate KiCad projects.
