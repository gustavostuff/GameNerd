# Hardware

**Status: design.** No schematic freeze yet. Goals: **smallest practical motherboard**, and the **cutest dual-sided carts** we can build (max **8 pads per side**).

Reference class to beat on mobo size: early Uzebox-style boards (DIP AVR, RCA pair, NES plugs, NTSC encoder). See [uzebox.org](https://uzebox.org/). Nano wins by deleting parts and using a tiny cart, not by packing a large BOM tighter.

## Why Nano can be smaller

| Uzebox-like board | Retr01 Nano (proposed) |
|-------------------|-------------------------|
| DIP-40 AVR | **TQFP-44** ATmega1284P |
| 8-bit R-2R + NTSC encoder | **1 bpp** FG levels + black backdrop, **RGBS only** |
| Dual RCA | Compact RGBS (+ audio) header or mini plug |
| NES plugs | Arcade microswitch headers (2P) |
| No / large cart | **Tiny dual-sided cart** (SPI + I2C), <= 8 pads/side |
| MIDI | None |

## Motherboard block diagram

```text
                 +------------------+
   5V in ------->| LDO / filter     |
                 +--------+---------+
                          |
                          v
                        +-------------+
  Crystal ------------->| ATmega1284P |<-- ISP
                        |  (open FW)  |
  P1 / P2 arcade ------>|             |---- FG GPIOs + sync ---> RGBS
                        |             |---- PWM music --+
                        |             |---- PWM SFX ----+-- R mix --> audio jack
                        |             |
                        |             |---- SPI ---> cart flash CS/SCK/MOSI/MISO
                        |             |---- I2C ---> cart EEPROM SDA/SCL
                        +------+------+
                               |
                        cute cart edge
                        (dual side, <=8 pads/side)
```

## Cute cartridge

**Always populated:**

| IC | Package target | Role |
|----|----------------|------|
| **SST25VF010A** | SOIC-8 | 1 Mbit (**128 KB**) SPI flash: game MAP / CHR / data |
| **24C64** | SOIC-8 | 8 KB I2C save EEPROM (always fitted for simple copper) |

Optional-save population was rejected for v1 so carts do not need hand bridges or stuffing variants.

### Mechanical

- Tiny PCB, dual-sided gold pads
- **At most 8 pads per side** (16 contacts total budget)
- Short insert depth, small plastic shell later
- Not the full Retr01 36-pin / SST39SF040 cart

### Electrical sketch (pad budget)

Shared power and ground on both sides as needed. Remaining pads carry roughly:

| Net | Notes |
|-----|-------|
| SPI CS, SCK, MOSI, MISO | SST25VF010A |
| I2C SDA, SCL | 24C64 |
| VCC, GND | Shared |
| Optional detect / WP | If pad count allows |

Exact pin map freezes with the first cart schematic; until then use [`pinmap.md`](pinmap.md) (Sim + SKiDL SoT). HOLD/WP on the flash can be tied on the cart PCB to save edge pins.

### Flashing carts

Bench path TBD (clip, pogo fixture, or a tiny USB bridge). Motherboard ISP stays for **console firmware** only.

## Motherboard floorplan sketch

```text
+------------------------------------------+
|  [5V in]              [ISP]              |
|           ATmega1284P (TQFP)             |
|        xtal + decoupling                 |
|   FG resistor pack     PWM music/SFX mix |
|  [RGBS + AUD]                            |
|  [cute cart slot]                        |
|  J_P1 arcade              J_P2 arcade    |
+------------------------------------------+
```

### Size tactics

1. SMD MCU and passives
2. Arcade headers on the edge
3. Combined RGBS/audio connector
4. No NTSC encoder, MIDI, or second MCU
5. Cart slot sized for the tiny dual-sided card, not a full-size edge

### Rough BOM sketch (v1)

| Item | Role |
|------|------|
| ATmega1284P TQFP-44 | Open console firmware |
| 20 MHz crystal + load caps | Clock |
| 5V LDO / protection | Power |
| Resistor network | 8 FG colors + black / sync into RGBS |
| Series R pack + DC block | Mix **two PWM** channels (music + SFX) to one audio out |
| 2x arcade headers | P1 / P2 |
| Cute cart connector | <= 8 pads/side dual |
| 2x3 ISP | Firmware |
| Cart: SST25VF010A + 24C64 | Game + saves |

## Pad headers

Same bit layout spirit as full Retr01 `$FE60` / `$FE61` (bit set = pressed):

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

## RGBS connector

Order of preference for a small board:

1. 2.54 mm pin header (bring-up)
2. Mini-DIN or multi-pole jack once pinout freezes
3. Avoid dual full-size RCA unless an enclosure needs them

## Power

- 5V logic for early bring-up
- USB-C power-only is attractive for size
- ISP remains the recovery path for console firmware

## Bring-up order

1. Video kernel on a 1284 breakout (no cart yet)
2. SPI flash on a breakout as a fake cart
3. First cute cart PCB + slot
4. Shrink motherboard outline

## Non-goals for v1 PCB

- On-board NTSC / PAL encode
- Full Retr01 36-pin cart compatibility
- Dual NES sockets
- MIDI
- Complex PMIC
