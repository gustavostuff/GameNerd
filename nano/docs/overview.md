# Overview

**Status: design.** Ahead of any runner or silicon in `nano/`.

## Intent

Retr01 Nano keeps the *feel* of Retr01:

- 128-wide playfield
- Arcade cabinet language
- Worlds and screens on a small cart
- One clear machine you can reason about end to end

It drops the heavy motherboard: no 6502, no PLD tile engine, no OAM, no BG0, no rich APU. The console MCU runs **fixed open firmware**. Each game is a **tiny cartridge**.

## Design goals

1. One **ATmega1284P** (20 MHz preferred) as the whole console.
2. Stable progressive **RGBS** at **60 Hz**.
3. **Cute dual-sided carts** (**8+8** gold fingers, same pad pitch/width as full Retr01; see [`hardware.md`](hardware.md)).
4. Instant screen switches only (no scrolling).
5. Soft **entities** in RAM (not hardware sprites).

## Locked feature sketch

| Feature | Value | Notes |
|---------|-------|-------|
| Resolution | **128x96** logical, **256x192** RGBS (2x) | Letterboxing / empty bands OK in the logical grid |
| Frame rate | **60 Hz** stable | Progressive RGBS only (v1) |
| Pixels | **1 bpp** | FG color from per-tile attr (8 colors). Backdrop always **black** |
| Tiles | **8x8** | |
| Screen | **16x12** tiles | Matches 128x96 |
| World grid | **16x16** screens | Max **8** worlds, **16** present screens/world |
| BG banks | **4** per world | Selected by attr bank bits |
| Sprites | **None** | Up to **64** RAM entities instead |
| Scroll | **None** | Instant screen switch in VBlank |
| Audio | **2 PWM channels** | Music pulse + SFX. Resistor mix to one jack ([`sound.md`](sound.md)) |
| Input | **2 players** | Retr01 `$FE60` / `$FE61` bit spirit |
| Console Flash | 128 KB on 1284 | Open firmware + kernel only |
| Cart Flash | SST25VF010A **128 KB** | PRG data, MAP, CHR, music — **8+8** edge ([`hardware.md`](hardware.md)) |
| Cart save | **24C64** always | Simpler cart routing than optional populate |

## Explicitly dropped (vs full Retr01)

- 6502 CPU and `$FExx` bus as a product requirement
- Large 36-pin cart edge / SST39SF040 class flash
- Hardware tile engine / OAM / sprites
- Color PROM and multi-layer BG0
- Fine pixel or tile scrolling
- Complex multi-channel APU
- Composite / NTSC encode path

## Relationship to full Retr01

**Spiritual child**, not a binary-compatible subset.

- Same pad-byte spirit and 128-wide arcade language
- Different cart electricals and image format
- Studio / Emu / Sim for full Retr01 stay separate

## Build roadmap (firmware first)

1. Rock-solid **128x96** compose + **256x192** (2x) RGBS kernel with double line buffer.
2. In-RAM nametable renderer (tile + attr, black backdrop).
3. SPI cart read + **instant screen load** in VBlank.
4. Entity stamp pass (priority over MAP).
5. PWM audio + 2P GPIO.
6. Tiny sample cart game.
7. Only then push vertical resolution.

PCB and cute-cart connector can proceed in parallel (see [`hardware.md`](hardware.md); motherboard outline starts at **100 × 100 mm**, provisional). SKiDL: [`../schematic_generator/`](../schematic_generator/). Provisional port pin map: [`pinmap.md`](pinmap.md). SRAM cache policy: [`cache_architecture.md`](cache_architecture.md).

## Open items

- Exact C SDK / host OS tooling
- Cart image file format and community flash tools
- Game authoring path (hand tools vs a future Studio Nano profile)
- Final RGBS connector shell
