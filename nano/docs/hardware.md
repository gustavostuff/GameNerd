# Hardware

**Status: design → connector freeze for Sim/SKiDL.** Goals: **smallest practical motherboard**, and the **cutest dual-sided carts** we can build.

Reference class to beat on mobo size: early Uzebox-style boards (DIP AVR, RCA pair, NES plugs, NTSC encoder). See [uzebox.org](https://uzebox.org/). Nano wins by deleting parts and using a tiny cart, not by packing a large BOM tighter.

Port / edge electrical SoT: [`pinmap.md`](pinmap.md).  
SKiDL netlists: [`nano/schematic_generator/`](../schematic_generator/) (`nano_mobo.net`, `nano_cart.net`).

## Why Nano can be smaller

| Uzebox-like board | Retr01 Nano |
|-------------------|-------------|
| DIP-40 AVR | **TQFP-44** ATmega1284P |
| 8-bit R-2R + NTSC encoder | **1 bpp** FG levels + black backdrop, **RGBS only** |
| Dual RCA | Vertical **pin header** for RGBS + mono audio (bring-up) |
| NES plugs | Vertical arcade pin headers (2P) |
| No / large cart | **Tiny dual-sided gold-finger cart**, **8 pads/side** |
| MIDI | None |

## Motherboard block diagram

```text
  Barrel 5V (PJ-063AH) ---> protection / bulk ---> 5V plane
                                    |
                         +----------+-------------+
   Crystal 20 MHz ------>|      ATmega1284P       |<-- J_ISP (2x3 AVR)
                         |      (open FW)         |
   J_P1 / J_P2 arcade -->|                        |---- FG + sync ---> J_AV (RGBS+AUD)
                         |                        |---- PWM mix ------/
                         |                        |
                         |                        |---- SPI/I2C ------> J_CART (2x8 edge)
                         +------------------------+
```

## Motherboard outline (provisional)

| Item | Value |
|------|-------|
| Board size | **100 × 100 mm** (10 × 10 cm) |
| Status | **Starting target only** — not locked; Quilter / hand layout may shrink or grow |

Fits TQFP-44, cart edge, barrel, headers, and passives with room to spare for first spin. Prefer keeping connectors on edges; final outline follows placement.

## Cute cartridge — **locked: 8 + 8**

Classic **plane gold fingers**, both sides. Same finger recipe as full Retr01 cart ([`docs/cart.md`](../../docs/cart.md)), fewer positions.

| Item | Value |
|------|-------|
| Contacts | **8 per side** (16 total) |
| Pitch | **2.54 mm** (same as Retr01 36-pin) |
| Finger pad size | **8 × 1.7 mm** (same as Retr01) |
| Board thickness | **1.6 mm** |
| Finger center span | **7 × 2.54 = 17.78 mm** (A1..A8) |
| Outline | As small as the two SOICs + fingers allow (no minimum size goal beyond that) |

**Doable:** yes — same pad width/pitch as the large cart; only the mating length shrinks.

### Female connector (motherboard + flasher)

| Role | Part (target) | Notes |
|------|----------------|-------|
| Mobo / flasher cart slot | **EDAC 395-016-520-201** class (**2×8**, straight / vertical) or Sullins **EBC08DRXN** | Same family as full Retr01’s 2×18 EDAC 395. KiCad stand-in until CAD lands: `PinSocket_2x8_P2.54mm_Vertical` |
| Cart PCB fingers | Custom `Cart_Edge_2x8_P2.54mm` (clone of `Cart_Edge_2x18_…` truncated) | Pads 1–8 = side A (F.Cu), 9–16 = side B (B.Cu under A) |

Right-angle 2×8 (console shell later) can swap footprint later; **electrical pinout stays**.

### Cart edge pinout (play + program)

Signals needed for SST25 + 24C64 are only **eight unique nets**. Both sides carry mirrored power/SPI/I2C for contact reliability; **B8** is detect.

| Pos | Side A | Side B |
|-----|--------|--------|
| 1 | `GND` | `GND` |
| 2 | `VCC` | `VCC` |
| 3 | `SPI_SS#` | `SPI_SS#` |
| 4 | `SPI_SCK` | `SPI_SCK` |
| 5 | `SPI_MOSI` | `SPI_MOSI` |
| 6 | `SPI_MISO` | `SPI_MISO` |
| 7 | `I2C_SDA` | `I2C_SDA` |
| 8 | `I2C_SCL` | `CART_DET#` (cart ties to `GND`; mobo pull-up) |

Flash `HOLD#` / `WP#` **tied on the cart PCB** (not on the edge). SPI flash programming uses the **same SPI pins** (no parallel `WE#`).

## Programming — two ports, one bench tool

| Port on **motherboard** | Connector | Programs |
|-------------------------|-----------|----------|
| **J_ISP** | **2×3** vertical pin header (AVR ISP) | ATmega1284P console firmware / fuses |
| **J_CART** | **2×8** card-edge female (above) | Play socket; also the electrical face for in-socket cart flash if the MCU is held in reset / SPI released |

| Port on **Nano flasher** (bench PCB) | Connector | Programs |
|--------------------------------------|-----------|----------|
| Cart slot | Same **2×8** EDAC-class | `.r01nano` into SST25 + optional 24C64 blank/verify |
| Console ISP lead | **2×3** ISP cable / header | Same USBasp/Atmel-ICE pinout as J_ISP |

**One hardware flasher** = USB MCU (or host + USBasp dock) with **both** a 2×8 cart socket and a 2×3 ISP header. Cart path talks SPI+I2C; ISP path talks AVR. While ISP runs on a live motherboard, keep cart `SPI_SS#` idle (flash deselected) so the shared SPI pins do not fight.

Motherboard **J_ISP** stays for field console updates without removing the TQFP.

## Connectors (motherboard) — locked intent

All bring-up I/O uses **vertical 2.54 mm pin headers** unless noted.

| Ref | Connector | Pins | Nets |
|-----|-----------|------|------|
| **J_BARREL** | **CUI PJ-063AH** (2.1 mm ID), same as full Retr01 | — | Tip = +5 V in, sleeve = GND ([`docs/passive_rf_etc.md`](../../docs/passive_rf_etc.md)) |
| **J_P1** | **1×10** vertical pin header | 10 | `P1_D0..D7`, `GND`, key/NC — microswitches to GND |
| **J_P2** | **1×10** vertical pin header | 10 | `P2_D0..D7`, `GND`, key/NC |
| **J_PWR** | **1×4** vertical pin header | 4 | `+5V`, `GND`, `RESET#`, `NC` (bench power/reset access) |
| **J_AV** | **1×8** vertical pin header | 8 | `R`, `G`, `B`, `CSYNC`, `AUD`, `AGND`, `VGND`, key/NC |
| **J_ISP** | **2×3** vertical pin header | 6 | Standard AVR ISP: `MOSI`, `MISO`, `SCK`, `RESET#`, `VCC`, `GND` |
| **J_CART** | EDAC-class **2×8** vertical card edge | 16 | See cart table |

**J_AV notes:** `CSYNC` is composite sync for RGBS (H+V combined in FW or a tiny glue RC). `AGND` / `VGND` are separate returns at the header; star to plane near the connector. Mono audio = resistor mix of the two PWM channels + DC block.

## Motherboard floorplan sketch

```text
+--------------------------------------------------+
| [J_BARREL]     [J_ISP 2x3]      [J_PWR 1x4]      |
|              ATmega1284P TQFP-44                  |
|           xtal + decoupling                      |
|   FG R-pack     PWM R-mix + DC block             |
| [J_AV 1x8]              [J_CART 2x8 edge]        |
| [J_P1 1x10]                      [J_P2 1x10]     |
+--------------------------------------------------+
```

## Passiveives / other parts (reasonable v1 estimate)

Outside the ICs. Counts are order-of-magnitude for SKiDL BOM planning.

| Class | Est. qty | Role |
|-------|----------|------|
| 100 nF X7R (MCU, flash, EEPROM, local) | **8–12** | HF bypass at every VCC pin |
| 1–10 µF ceramic bulk | **2–4** | Island / post-barrel bulk |
| Bulk electrolytic / polymer at barrel | **1** | Input reservoir |
| Reverse-polarity diode or P-FET ideal diode | **1** | Barrel abuse |
| Optional PPTC on barrel | **1** | Cable fault |
| 20 MHz crystal | **1** | MCU clock |
| Crystal load caps | **2** | Per crystal datasheet |
| I2C pull-ups (SDA/SCL) | **2** | Typically 4.7 kΩ to 5 V |
| RESET pull-up (+ optional RC) | **1–2** | Open RESET# |
| Cart `CART_DET#` pull-up | **1** | |
| Series R on SPI/I2C to cart edge (optional ESD) | **4–8** | Soften edges / TVS companion |
| TVS at barrel / cart / headers (bring-up: light) | **2–6** | Touchable nets |
| FG resistor network (3-bit → RGB + sync) | **1 network or ~10 discretes** | 8 FG colors + black |
| PWM mix resistors + DC block cap | **4–6** | Music + SFX → `AUD` |
| ISP / header pin shrouds | as needed | Polarized 2×3 preferred |

**ICs (recap):** ATmega1284P TQFP-44; cart SST25VF010A SOIC-8; cart 24C64 SOIC-8. No second MCU on the motherboard.

## Pad bit layout

Same spirit as full Retr01 `$FE60` / `$FE61` (bit set = pressed):

| Bit | Button |
|-----|--------|
| 0 | Right |
| 1 | Left |
| 2 | Down |
| 3 | Up |
| 4 | X |
| 5 | Y |
| 6 | Coin / Select |
| 7 | Start |

## Power

- **5 V** from barrel for bring-up (logic level of the 1284 @ 20 MHz).
- USB-C power-only remains an optional later shell feature; **barrel is the locked v1 inlet**.
- ISP is the recovery path for console firmware.

## Bring-up order

1. Video kernel on a 1284 breakout (no cart yet)
2. SPI flash on a breakout as a fake cart
3. First cute cart PCB + 2×8 slot
4. Dual-port flasher (cart socket + ISP)
5. Refine motherboard outline from the **100 × 100 mm** starting target

## Non-goals for v1 PCB

- On-board NTSC / PAL encode
- Full Retr01 36-pin cart compatibility
- Dual NES sockets / TRS aux pads (arcade headers only)
- MIDI
- Complex PMIC / Li-ion
