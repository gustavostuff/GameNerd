# Nano pin map (provisional freeze)

**Status:** sim + SKiDL SoT until the first schematic locks.
Connectors / cart mechanics: [`hardware.md`](hardware.md).
Package on motherboard: **PDIP-40** (fully THT). Wire by **net / PORT**. DIP numbers match KiCad **`ATmega1284P-P`**.

Reference pinout: [`hw/md/ATmega1284P.md`](../../hw/md/ATmega1284P.md).
Extract: `nano/schematic_generator/nano_schem/kicad_pin_extract.json`. Generator: [`nano/schematic_generator/`](../schematic_generator/).

## Console MCU (net / PORT / DIP pin)

| Net | PORT | DIP-40 | Notes |
|-----|------|--------|-------|
| `SPI_SS#` | **PB4** | **5** | Cart **25LC1024** CS# / edge |
| `SPI_MOSI` | **PB5** | **6** | Cart SI. Shared with **J_ISP** MOSI |
| `SPI_MISO` | **PB6** | **7** | Cart SO. Shared with **J_ISP** MISO |
| `SPI_SCK` | **PB7** | **8** | Cart SCK. Shared with **J_ISP** SCK |
| `I2C_SCL` | **PC0** | **22** | Cart 24C64 (+ pull-up) |
| `I2C_SDA` | **PC1** | **23** | Cart 24C64 (+ pull-up) |
| `PWM_MUSIC` | **PD5** (OC1A) | **19** | Mix to `AUD` → **J_AV** + **J8** |
| `PWM_SFX` | **PD4** (OC1B) | **18** | Mix to `AUD` → **J_AV** + **J8** |
| `HSYNC` / `VSYNC` | **PD0** / **PD1** | **14** / **15** | Resistor-mix to `CSYNC` on **J_AV** (+ AD725 HSYNC) |
| `PAD_DATA` | **PD2** (RXD1) | **16** | Open-drain UART to **J3/J4** Ring (Retr01-C protocol) |
| `P1_D0..D7` | **PA0..PA7** | **40..33** | **J_PAD** (left column) |
| `P2_D0..D5` | **PC2..PC7** | **24..29** | **J_PAD** (right column) |
| `P2_D6..D7` | **PD6**, **PD7** | **20**, **21** | **J_PAD** (right column; moved off PD2/PD3 for UART) |
| `FG0..FG2` | **PB0..PB2** | **1..3** | Bring-up RGB enables → guns → AD725 |
| `CART_DET#` | **PB3** | **4** | Mobo pull-up. Cart B8 -> GND |
| `XTAL1` / `XTAL2` | XTAL1 / XTAL2 | **13** / **12** | **20 MHz** + load caps |
| `RESET#` | RESET | **9** | **J_ISP** + **J_PWR** |
| `VCC` | VCC | **10** | + decoupling |
| `GND` | GND | **11**, **31** | |
| `AVCC` / `AREF` | AVCC / AREF | **30** / **32** | AVCC to +5V. AREF bypass to GND |

### FG color (bring-up stub)

| Net | PORT / DIP | Role |
|-----|------------|------|
| `FG0..FG2` | PB0..PB2 / 1..3 | Bring-up: FG0->R, FG1->G, FG2->B via 470R + 75R |
| (backdrop) | grounded black | |

## Cart edge (**8 + 8 locked**)

Mechanics and female part: [`hardware.md`](hardware.md#cute-cartridge-locked-8-8).

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

Double-row headers use KiCad **PinHeader_2xN** numbering (column pairs):

```text
1  2
3  4
...
```

### J_AV (2x4)

| Pin | Net | Pin | Net |
|-----|-----|-----|-----|
| 1 | `R` | 2 | `G` |
| 3 | `B` | 4 | `CSYNC` |
| 5 | `AUD` | 6 | `AGND` |
| 7 | `VGND` | 8 | key / NC |

Same `R`/`G`/`B`/`CSYNC` nets AC-couple into **U725** (AD725). `AUD` also drives **J8**.

### J8 / J9 (CUI RCJ-01x RCA)

| Jack | Tip (pad 1) | Shell (pad 2) |
|------|-------------|----------------|
| **J8** RCJ-012 (black) | `AUD` | `GND` |
| **J9** RCJ-014 (yellow) | `COMPOSITE_OUT` | `GND` |

### J3 / J4 (Switchcraft 35RAPC2BVN4)

Same pad map as full Retr01: Tip=**4**, Ring=**2**, Sleeve=**1**; pads **3** and **5** NC (mechanical).

| Jack | Tip | Ring | Sleeve |
|------|-----|------|--------|
| **J3** P1 | `PAD_VCC_P1` (via PPTC F2) | `PAD_DATA_P1` (47R → `PAD_DATA`) | `GND` |
| **J4** P2 | `PAD_VCC_P2` (via PPTC F3) | `PAD_DATA_P2` (47R → `PAD_DATA`) | `GND` |

`PAD_DATA` → **PD2** with **4.7k** pull-up. Protocol: [`docs/controllers.md`](../../docs/controllers.md).

### AD725 (U725) — assembly notes

| Item | Value |
|------|--------|
| Chip | **AD725ARZ** wide SOIC-16 |
| Board footprint | **DIP-16** + **Proto Advantage PA0006** |
| Clock | **Y3** ACH-14.31818 MHz → `4FSC` |
| RGB in | AC-coupled from `R`/`G`/`B` (100 nF) |
| Sync | `CSYNC` → HSYNC; VSYNC/STND/CE tied **+5V** (NTSC, CSYNC-only) |
| Power | APOS/DPOS on `+5V_ANALOG` (ferrite FB2 + 10 µF); local 100 nF |
| YTRAP | ~68 µH + 100 nF |
| Out | COMP → 75 Ω → **J9** tip. CRMA/LUMA unused |

### J_PAD (2x10) — arcade 2P

P1 on the odd (left) column, P2 on the even (right) column; bit *n* is paired.

| Pin | Net | Pin | Net |
|-----|-----|-----|-----|
| 1 | `P1_D0` | 2 | `P2_D0` |
| 3 | `P1_D1` | 4 | `P2_D1` |
| 5 | `P1_D2` | 6 | `P2_D2` |
| 7 | `P1_D3` | 8 | `P2_D3` |
| 9 | `P1_D4` | 10 | `P2_D4` |
| 11 | `P1_D5` | 12 | `P2_D5` |
| 13 | `P1_D6` | 14 | `P2_D6` |
| 15 | `P1_D7` | 16 | `P2_D7` |
| 17 | `GND` | 18 | `GND` |
| 19 | key / NC | 20 | NC |

### J_PWR (2x2)

| Pin | Net | Pin | Net |
|-----|-----|-----|-----|
| 1 | `+5V` | 2 | `GND` |
| 3 | `RESET#` | 4 | NC |

### J_ISP (2x3)

Standard AVR ISP: `MOSI`, `MISO`, `SCK`, `RESET#`, `VCC`, `GND` (same assignment as full Retr01 / USBasp).

## What Nano Sim models today

| Layer | Fidelity |
|-------|----------|
| Islands / wire settle / pin levels | Yes. MCU PORT nets above |
| RGBS scanline kernel | Yes. One board step / RGBS line |
| Host Play + soft compose | Behavioral (emu core) |
| SPI MAP refill in VBlank | Byte-clocked on pins |
| Cart 8+8 / headers as entities | Documented. UI still abstract pads |
| AVR ISA / pixel FG DAC | **No** |

## SKiDL / Quilter handoff

1. Generator: [`nano/schematic_generator/`](../schematic_generator/) (`python generate.py`).
2. Symbols: `ATmega1284P-P` PDIP-40. Cart **25LC1024** + `24LC64` (both DIP-8).
3. Cart footprint: `Retr01_Lib:Cart_Edge_2x8_P2.54mm` (same pad recipe as 2x18).
4. Keep PORT names stable. DIP numbers are filled in the table above.
5. Import `output/nano_mobo.net` + `output/nano_cart.net` into separate KiCad projects.
