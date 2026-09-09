# Hardware

**Status: design -> connector freeze for Sim/SKiDL.** Goals: **smallest practical motherboard**, and the **cutest dual-sided carts** we can build.

Reference class to beat on mobo size: early Uzebox-style boards (DIP AVR, RCA pair, NES plugs, NTSC encoder). See [uzebox.org](https://uzebox.org/). Nano wins by deleting parts and using a tiny cart, not by packing a large BOM tighter.

Port / edge electrical SoT: [`pinmap.md`](pinmap.md).  
SKiDL netlists: [`nano/schematic_generator/`](../schematic_generator/) (`nano_mobo.net`, `nano_cart.net`).

## Why Nano can be smaller

| Uzebox-like board | Retr01 Nano |
|-------------------|-------------|
| DIP-40 AVR | **DIP-40** ATmega1284P (fully THT mobo) |
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
| Board size | **100 x 100 mm** (10 x 10 cm) |
| Status | **Starting target only** (not locked). Quilter / hand layout may shrink or grow |

Fits DIP-40, cart edge, barrel, headers, and passives with room to spare for first spin. Prefer keeping connectors on edges. Final outline follows placement.

## Cute cartridge (locked 8+8)

Classic **plane gold fingers**, both sides. Same finger recipe as full Retr01 cart ([`docs/cart.md`](../../docs/cart.md)), fewer positions.

| Item | Value |
|------|-------|
| Contacts | **8 per side** (16 total) |
| Pitch | **2.54 mm** (same as Retr01 36-pin) |
| Finger pad size | **8 x 1.7 mm** (same as Retr01) |
| Board thickness | **1.6 mm** |
| Finger center span | **7 x 2.54 = 17.78 mm** (A1..A8) |
| Outline | As small as flash + EEPROM + fingers allow (no minimum size goal beyond that) |

**Doable:** yes. Same pad width/pitch as the large cart. Only the mating length shrinks.

### Cart IC packages (hand-buildability)

**Edge pads != IC pins.** The cart connector is **8+8** for SPI/I2C/power only. Both cart ICs are **PDIP-8** (fully THT).

| Role | Locked MPN | Package | Notes |
|------|------------|---------|-------|
| Game image | **25LC1024-I/P** | **PDIP-8** | 1 Mbit SPI EEPROM, **2.5-5.5 V**, JEDEC SPI pinout. SKiDL default. |
| Save EEPROM | **24C64** family | **PDIP-8** | Same pinout as SOIC. |

Optional SMD twin for game chip: **25LC1024-I/SM** (SOIJ-8, ~208 mil). Legacy **SST25VF010A** (SOIC-8 150 mil, 3.3 V only) is no longer the cart default.

Flasher FW must use **25LC1024** write/erase commands (SPI EEPROM), not raw NOR flash opcodes. Reads for `.r01nano` streaming match JEDEC SPI.

DIP-16 / parallel NOR still do not fit the cute **8+8** edge model (see prior notes).


### Female connector (motherboard + flasher)

| Role | Part (target) | Notes |
|------|----------------|-------|
| Mobo / flasher cart slot | **EDAC 395-016-520-201** class (**2x8**, straight / vertical) or Sullins **EBC08DRXN** | Same family as full Retr01's 2x18 EDAC 395. KiCad stand-in until CAD lands: `PinSocket_2x8_P2.54mm_Vertical` |
| Cart PCB fingers | Custom `Cart_Edge_2x8_P2.54mm` (clone of `Cart_Edge_2x18_...` truncated) | Pads 1-8 = side A (F.Cu), 9-16 = side B (B.Cu under A) |

Right-angle 2x8 (console shell later) can swap footprint later. **Electrical pinout stays**.

### Cart edge pinout (play + program)

Signals needed for **25LC1024** + 24C64 are only **eight unique nets**. Both sides carry mirrored power/SPI/I2C for contact reliability. **B8** is detect.

| Pos | Side A | Side B |
|-----|--------|--------|
| 1 | `GND` | `GND` |
| 2 | `VCC` | `VCC` |
| 3 | `SPI_SS#` | `SPI_SS#` |
| 4 | `SPI_SCK` | `SPI_SCK` |
| 5 | `SPI_MOSI` | `SPI_MOSI` |
| 6 | `SPI_MISO` | `SPI_MISO` |
| 7 | `I2C_SDA` | `I2C_SDA` |
| 8 | `I2C_SCL` | `CART_DET#` (cart ties to `GND`, mobo pull-up) |

Flash `HOLD#` / `WP#` **tied on the cart PCB** (not on the edge). SPI programming uses the **same SPI pins** (no parallel `WE#`).

## Programming: USBasp for MCU, then MCU flashes the cart

**Yes:** one **USBasp** on **J_ISP** fully programs the **ATmega1284P** (console firmware + fuses). After that, with the ATmega **running**, that firmware flashes the **plugged-in cart** (SPI master, `PB4` = `SPI_SS#`) with the `.r01nano` game image into the **25LC1024**. Optional 24C64 work goes over I2C from the same FW.

USBasp never talks to the cart chips itself (wrong protocol: `SPI_SS#` / I2C are not on the 6-pin ISP header). The cart write is always **through the running 1284**.

```text
  USBasp --J_ISP--> ATmega1284P   (console FW, once / when updating)
                         |
                         | SPI (PB4 SS#) + I2C
                         v
                    plugged-in cart (25LC1024 + 24C64)
```

### Preferred field flow

1. **USBasp -> J_ISP:** write open console firmware (and fuses) into the 1284. While ISP holds `RESET#`, cart `SPI_SS#` stays idle (high) so the 25LC1024 stays deselected on the shared SPI lines.
2. **Unplug USBasp (or leave it idle).** Power the board, cart inserted. Console FW runs and **SPI-masters** the cart to program / verify `.r01nano`.

How the host feeds the image *into* that running FW is TBD (UART, USB CDC, Studio tool, etc.). That host link is not ISP / avrdude.

### Bench flasher (still useful)

| Port on **motherboard** | Connector | Programs |
|-------------------------|-----------|----------|
| **J_ISP** | **2x3** vertical pin header (AVR ISP) | ATmega1284P console firmware / fuses (USBasp) |
| **J_CART** | **2x8** card-edge female | Play socket. Also the face a bench tool uses for direct cart SPI/I2C |

| Port on **Nano flasher** (bench PCB) | Connector | Programs |
|--------------------------------------|-----------|----------|
| Cart slot | Same **2x8** EDAC-class | `.r01nano` into 25LC1024 + optional 24C64 (no console required) |
| Console ISP lead | **2x3** ISP cable / header | Same USBasp/Atmel-ICE pinout as J_ISP |

**One hardware flasher** = USB MCU (or host + USBasp dock) with **both** a 2x8 cart socket and a 2x3 ISP header. Use it for blank carts, recovery, or before console FW cart-write exists.

Motherboard **J_ISP** stays for field console updates without removing the DIP.

## Connectors (motherboard, locked intent)

All bring-up I/O uses **vertical 2.54 mm pin headers** unless noted.

| Ref | Connector | Pins | Nets |
|-----|-----------|------|------|
| **J_BARREL** | **CUI PJ-063AH** (2.1 mm ID), same as full Retr01 | n/a | Tip = +5 V in, sleeve = GND ([`docs/passive_rf_etc.md`](../../docs/passive_rf_etc.md)) |
| **J_P1** | **1x10** vertical pin header | 10 | `P1_D0..D7`, `GND`, key/NC (microswitches to GND) |
| **J_P2** | **1x10** vertical pin header | 10 | `P2_D0..D7`, `GND`, key/NC |
| **J_PWR** | **1x4** vertical pin header | 4 | `+5V`, `GND`, `RESET#`, `NC` (bench power/reset access) |
| **J_AV** | **1x8** vertical pin header | 8 | `R`, `G`, `B`, `CSYNC`, `AUD`, `AGND`, `VGND`, key/NC |
| **J_ISP** | **2x3** vertical pin header | 6 | Standard AVR ISP: `MOSI`, `MISO`, `SCK`, `RESET#`, `VCC`, `GND` |
| **J_CART** | EDAC-class **2x8** vertical card edge | 16 | See cart table |

**J_AV notes:** `CSYNC` is composite sync for RGBS (H+V combined in FW or a tiny glue RC). `AGND` / `VGND` are separate returns at the header. Star to plane near the connector. Mono audio = resistor mix of the two PWM channels + DC block.

## Motherboard floorplan sketch

```text
+--------------------------------------------------+
| [J_BARREL]     [J_ISP 2x3]      [J_PWR 1x4]      |
|              ATmega1284P DIP-40                   |
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
| 100 nF X7R (MCU, flash, EEPROM, local) | **8-12** | HF bypass at every VCC pin |
| 1-10 uF ceramic bulk | **2-4** | Island / post-barrel bulk |
| Bulk electrolytic / polymer at barrel | **1** | Input reservoir |
| Reverse-polarity diode or P-FET ideal diode | **1** | Barrel abuse |
| Optional PPTC on barrel | **1** | Cable fault |
| 20 MHz crystal | **1** | MCU clock |
| Crystal load caps | **2** | Per crystal datasheet |
| I2C pull-ups (SDA/SCL) | **2** | Typically 4.7 kohm to 5 V |
| RESET pull-up (+ optional RC) | **1-2** | Open RESET# |
| Cart `CART_DET#` pull-up | **1** | |
| Series R on SPI/I2C to cart edge (optional ESD) | **4-8** | Soften edges / TVS companion |
| TVS at barrel / cart / headers (bring-up: light) | **2-6** | Touchable nets |
| FG resistor network (3-bit -> RGB + sync) | **1 network or ~10 discretes** | 8 FG colors + black |
| PWM mix resistors + DC block cap | **4-6** | Music + SFX -> `AUD` |
| ISP / header pin shrouds | as needed | Polarized 2x3 preferred |

**ICs (recap):** ATmega1284P **PDIP-40**. Cart **25LC1024 PDIP-8** + **24C64 PDIP-8** (fully THT). See [cart IC packages](#cart-ic-packages-hand-buildability). No second MCU on the motherboard.

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
- USB-C power-only remains an optional later shell feature. **Barrel is the locked v1 inlet**.
- ISP is the recovery path for console firmware.

## Planned later (not in v1 netlist / PCB)

These are intentional follow-ons. They are **not** in the current SKiDL / bring-up design (vertical headers only).

| Feature | Intent | Parent Retr01 reference |
|---------|--------|-------------------------|
| **RCA-like AV** | Mono **audio** RCA + **composite video** RCA (same spirit as full Retr01 RCJ footprints) | [`docs/passive_rf_etc.md`](../../docs/passive_rf_etc.md), CUI **RCJ-01x** |
| **Composite encode** | On-board NTSC/PAL from RGBS + CSYNC (e.g. AD725 class) once RGBS bring-up is solid | Full Retr01 **AD725** path |
| **TRS gamepads** | High-quality PCB footprints for real aux pads: same **Switchcraft 35RAPC** TRS, **ATtiny85** pad boards, **3-wire** half-duplex UART protocol as Retr01-C | [`docs/controllers.md`](../../docs/controllers.md) |
| **Light gun** | Same TRS bus / protocol family as pads (CRT accessory roadmap) | [`docs/lightgun.md`](../../docs/lightgun.md) |

v1 keep **J_AV** (RGBS + mono on a pin header) and **J_P1 / J_P2** arcade headers. Pad byte contract stays the same bitfield so TRS pads can drop in later without rewriting game I/O.

## Bring-up order

1. Video kernel on a 1284 breakout (no cart yet)
2. SPI memory on a breakout as a fake cart
3. First cute cart PCB + 2x8 slot
4. Dual-port flasher (cart socket + ISP)
5. Refine motherboard outline from the **100 x 100 mm** starting target

## Non-goals for v1 PCB

- On-board NTSC / PAL encode (**planned later**, see above)
- Full Retr01 36-pin cart compatibility
- TRS aux pads / light gun footprints on this spin (**planned later**, same parent protocol)
- MIDI
- Complex PMIC / Li-ion
