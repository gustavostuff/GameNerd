# Overview

**Status: design.** Ahead of any runner or silicon in `nano/`.

## Intent

Retr01 Nano keeps the *feel* of Retr01:

- 128-wide playfield
- Arcade cabinet language (digital pads, simple HUD-friendly graphics)
- One clear machine you can reason about end to end

It drops almost everything that makes the full motherboard large: cart slot, 6502, PLD tile engine, OAM, multi-layer BG, rich APU, dual MCU domains.

The trade is intentional. Nano optimizes for **stable 60 Hz**, **low part count**, and a **PCB smaller than typical single-chip hobby consoles** such as early Uzebox prototypes (DIP MCU, RCA pair, NES plugs, NTSC encoder).

## Design goals

1. Run entirely on **one ATmega1284P** (20 MHz preferred).
2. Deliver **stable progressive RGBS** at **60 Hz**.
3. Keep 128-wide tiles and an arcade control map familiar to Retr01.
4. Accept hard feature cuts for reliability and size.

## Locked feature sketch

| Feature | Value | Notes |
|---------|-------|-------|
| Resolution | **128x96** primary | 128x100 possible later with tighter code |
| Frame rate | **60 Hz** stable | Progressive RGBS only (v1) |
| Color | **1 bpp** (2 colors) | Levels chosen in software via resistor DAC |
| Tiles | **8x8** only | |
| Sprites | **None** | |
| Background | Single nametable | |
| Scroll | Tile-level **or** instant screen swap | No fine pixel scroll |
| Audio | 1 PWM channel | Square / simple tones in VBlank |
| Input | **2 players** | One byte each, Retr01 `$FE60` / `$FE61` bit spirit |
| Game storage | On-chip Flash | EEPROM for saves / config |
| Line buffer | Double buffer in SRAM | 16 + 16 bytes |

## Explicitly dropped (vs full Retr01)

- 6502 CPU and PRG cart protocol
- External cartridge connector
- Hardware tile engine / OAM / sprites
- Multi-color palettes and Color PROM
- Second background (BG0)
- Fine pixel scrolling
- Complex multi-channel APU
- Composite / NTSC encode path (RGBS analog out only for now)

## Relationship to full Retr01

**Spiritual child**, not a compatible subset.

- Same design language and pad-byte spirit
- Not the same cart format
- Not the same `$FExx` silicon map as a product requirement
- Studio / Emu / Sim for full Retr01 stay separate. Nano may grow its own C SDK later (TBD)

## Build roadmap (firmware first)

1. Rock-solid **128x96** 1 bpp RGBS kernel with double line buffer.
2. Nametable renderer (8x8 tiles).
3. Tile-level scroll and instant screen switch.
4. One-channel PWM audio in VBlank.
5. Two-player GPIO read into pad bytes.
6. Tiny sample game on the loop.
7. Only then push vertical resolution (128x100, maybe 128x120 with heavy asm).

PCB layout can proceed in parallel as a **size-first** sketch (see [`hardware.md`](hardware.md)), but the video kernel is the risk item.

## Open items

- Exact C SDK / host OS tooling (avr-gcc Make, etc.)
- Game authoring path (hand C vs a future Studio Nano profile)
- Final connector choice for RGBS + audio (headers vs mini jack)
