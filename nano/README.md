# Retr01 Nano

**Status: design only.** No firmware or PCB in this tree yet.

Retr01 Nano is a **spiritual child** of [Retr01](../README.md): same 128-wide arcade language, far less hardware. One **ATmega1284P** runs video, game logic, audio, and input. There is **no 6502**, **no cartridge**, and **no external video ASIC**. The game lives in on-chip Flash (EEPROM for saves).

| | Retr01 (full) | Retr01 Nano |
|--|---------------|-------------|
| CPU / host | 6502 + MCUs | Single ATmega1284P @ 20 MHz (preferred) |
| Storage | Cart flash | Internal Flash + EEPROM |
| Video | Multi-layer tile / sprite pipeline | 1 bpp tilemap, no sprites |
| Resolution (target) | 128x120 logical playfield | **128x96** @ stable **60 Hz** RGBS |
| Players | 2P arcade GPIO | **2P** same pad-byte spirit |
| Audio | Multi-channel APU path | 1 PWM channel |

Primary goal: a **minimal, stable 60 Hz** single-chip board that still feels like Retr01 (clean playfield, arcade controls, simple elegant design).

## Docs

| Doc | Topic |
|-----|--------|
| [`docs/overview.md`](docs/overview.md) | Goals, feature cuts, roadmap |
| [`docs/video.md`](docs/video.md) | RGBS timing, line buffers, tiles, scroll |
| [`docs/memory_and_software.md`](docs/memory_and_software.md) | Flash / SRAM / EEPROM budget, C SDK direction |
| [`docs/hardware.md`](docs/hardware.md) | Small PCB layout vs Uzebox-class boards, BOM sketch |

Source concept notes also live in the repo temp folder (`temp/Retr01_Nano_Spec.md`). Where this tree disagrees (for example **2 players**), **these docs win**.

## Non-goals (v1)

- Cart format or Studio cart export compatibility
- Composite / NTSC encoder chips
- Sprites, BG0, fine pixel scroll, multi-color PROMs
- Host speaker tooling or full Retr01 Sim netlist reuse

Authoring tools and exact host toolchain are **TBD**. Direction: a small **C SDK** plus a tiny assembly video kernel.

## License / ownership

Same project umbrella as Retr01. See the parent repo for licensing when published.
