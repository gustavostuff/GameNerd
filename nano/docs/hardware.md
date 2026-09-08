# Hardware

**Status: design.** No schematic freeze yet. Goal: **smallest practical PCB**, clearly smaller than early Uzebox-class prototypes (large DIP AVR, dual RCA, NES plugs, NTSC encoder, MIDI).

Reference class to beat on size: hobby single-AVR consoles with bulky I/O (see [uzebox.org](https://uzebox.org/)). Nano wins by deleting parts, not by packing the same BOM tighter.

## Why Nano can be smaller

| Uzebox-like board | Retr01 Nano (proposed) |
|-------------------|-------------------------|
| DIP-40 AVR | **TQFP-44** ATmega1284P (or similar SMD) |
| 8-bit R-2R video DAC | Tiny **1 bpp** resistor network (two levels + sync) |
| AD725 / NTSC encoder | **None** (RGBS analog out only) |
| Dual RCA A/V | Compact **RGBS (+ audio)** connector or pin header |
| NES controller plugs | **Arcade microswitch headers** (2P), Retr01 spirit |
| MIDI IN | **None** |
| Cart / large edge I/O | **None** (Flash holds the game) |

## Proposed block diagram

```text
                 +------------------+
   5V in ------->| LDO / filter     |
                 +--------+---------+
                             |
                             v
                           +-------------+
   Crystal --------------->| ATmega1284P |
   ISP 6-pin ------------->|   TQFP-44   |
                           |             |
     P1 arcade GPIO ------>|             |---- PWM ---> audio out (R + AC couple)
     P2 arcade GPIO ------>|             |
                           |             |---- R/G/B level GPIOs ---> resistor net
                           |             |---- CSYNC / H+V sync ----+--> RGBS header
                           +-------------+                          |    or mini plug
                                                                    v
                                                              upscaler / CRT
```

## Mechanical / layout sketch

Target: a **single-sided or 2-layer** board that fits in a small enclosure (credit-card class is a stretch goal, pocket rectangle is enough). Keep tall connectors on one edge.

Suggested floorplan (top view):

```text
+------------------------------------------+
|  [USB-C or barrel 5V]   [ISP]            |
|                                          |
|           ATmega1284P (TQFP)             |
|        xtal + decoupling nearby          |
|                                          |
|   R-pack (RGB + sync)    PWM R           |
|                                          |
|  [RGBS + AUD connector]                  |
|                                          |
|  J_P1 1x10 arcade          J_P2 1x10     |
|  (Right..Start bit order)  (same)        |
+------------------------------------------+
```

### Size tactics

1. Prefer **SMD** MCU and passives (0805/0603).
2. Put **arcade headers** on the edge, not mid-board shrouded NES shells.
3. One **combined** video/audio connector beats two RCA + DIN.
4. Skip encoder ICs, MIDI, second MCU, cart finger.
5. Keep the analog resistor network next to the RGBS connector to shorten video runs.

### Rough BOM sketch (v1)

| Item | Role |
|------|------|
| ATmega1284P TQFP-44 | CPU + video + audio + pads |
| 20 MHz crystal + load caps | System clock |
| 5V LDO (if USB-C) or barrel + polyfuse | Power |
| Resistor network | 1 bpp RGB levels + sync into 75 ohm-ish world |
| Series R + DC block | PWM audio |
| 2x 1x10 (or 1x8) headers | P1 / P2 arcade GPIO |
| 1x RGBS+AUD connector | Design choice: pin header first, mini DIN / TRRS later |
| 2x3 ISP | Programming |
| Decoupling / bulk caps | Standard AVR practice |

Optional later: TRS jacks for console-style pads (full Retr01 dual-path idea). Not required for Nano v1 arcade headers.

## Pad headers

Same **bit layout spirit** as full Retr01 `$FE60` / `$FE61` (bit set = pressed):

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

Wire microswitches to GPIO with MCU internal pull-ups (active low in hardware, inverted in firmware to match the bit contract). Exact pin assign is schematic work.

## RGBS connector (open choice)

Order of preference for a **small** board:

1. **2.54 mm pin header** (bring-up, smallest on PCB)
2. Mini-DIN or multi-pole jack once pinout freezes
3. Avoid dual full-size RCA unless a specific enclosure needs them

Audio can share the same shell or use a separate tiny pad.

## Power

- 5V logic assumed for early bring-up
- USB-C **power only** is attractive for size (no data required)
- Keep ISP independent so bricks stay recoverable

## Bring-up order vs PCB

Firmware kernel can start on a **commercial 1284 breakout** with flying leads (RGBS breadboard DAC, pad buttons). Parallel PCB work should freeze:

1. MCU package and crystal
2. RGBS resistor values and connector
3. P1/P2 header pinout
4. Board outline smaller than the Uzebox reference class above

## Non-goals for v1 PCB

- On-board NTSC / PAL encode
- Cart edge
- Dual NES sockets
- MIDI
- Battery fuel gauge / complex PMIC
