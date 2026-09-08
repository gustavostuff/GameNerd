# Retr01 Nano

**Status: design only.** No firmware or PCB in this tree yet.

Retr01 Nano is a **spiritual child** of [Retr01](../README.md): same 128-wide arcade language, far less hardware. One **ATmega1284P** runs a **fixed, open console firmware**. Games live on a **tiny dual-sided cartridge** (SPI flash + save EEPROM). There is **no 6502** and **no external video ASIC**.

| | Retr01 (full) | Retr01 Nano |
|--|---------------|-------------|
| CPU / host | 6502 + MCUs | Single ATmega1284P @ 20 MHz (preferred) |
| Storage | Large cart flash + 24C64 | **Cute cart:** SST25VF010A (128 KB) + **24C64** always on cart |
| Firmware | Cart PRG + MCU assists | **Fixed open** 1284 firmware (community-flashable) |
| Video | Multi-layer tile / sprite pipeline | 1 bpp tiles, attr FG color, **no sprites** |
| Worlds | Up to 8, rich MAP | Up to **8** worlds, **16** screens/world, **16x16** screen grid |
| Resolution (target) | 128x120 logical playfield | **128x96** @ stable **60 Hz** RGBS |
| Motion | Pixel scroll + streaming | **Instant screen switch only** |
| Players | 2P arcade GPIO | **2P** same pad-byte spirit |
| Audio | Multi-channel APU path | 1 PWM channel |

Primary goal: a **minimal, stable 60 Hz** board with the smallest carts we can get away with, still feeling like Retr01.

## Docs

| Doc | Topic |
|-----|--------|
| [`docs/overview.md`](docs/overview.md) | Goals, cuts, roadmap |
| [`docs/graphics.md`](docs/graphics.md) | Worlds, screens, attr byte, entities |
| [`docs/video.md`](docs/video.md) | RGBS, line buffers, VBlank loads |
| [`docs/memory_and_software.md`](docs/memory_and_software.md) | MCU vs cart budgets, open firmware, SDK |
| [`docs/cache_architecture.md`](docs/cache_architecture.md) | How 16 KB SRAM caches cart MAP/CHR without filling to the brim |
| [`docs/hardware.md`](docs/hardware.md) | Mobo + cute cart connector, BOM sketch |

Source concept notes also live in `temp/Retr01_Nano_Spec.md`. Where this tree disagrees, **these docs win**.

## Non-goals (v1)

- Compatibility with full Retr01 `.retr01` cart images or 36-pin edge
- Composite / NTSC encoder chips
- Hardware sprites, BG0, any screen scrolling
- Full Retr01 Sim netlist reuse

Authoring tools and exact host toolchain are **TBD**. Direction: open **C** firmware/SDK on the 1284 plus a tiny assembly video kernel. Game data is authored onto the SPI cart image.

## License / ownership

Same project umbrella as Retr01. Console firmware is intended to stay **open** for the community. See the parent repo for licensing when published.
