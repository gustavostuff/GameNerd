# Retr01 Nano

**The smallest, cutest, simplest cartridge-based dual gaming system (arcade + console).**

**Status: Phase 1 bring-up next.** Host Studio / Emu / Sim already run a MAP-only picture path. Console silicon starts with a bare RGBS lab setup (below). Later phases grow toward the full Nano form already sketched in [`docs/`](docs/).

Retr01 Nano is a **spiritual child** of [Retr01](../README.md): same 128-wide arcade language, far less hardware. One **ATmega1284P** runs a **fixed, open console firmware**. Games live on a **tiny dual-sided cartridge** (SPI EEPROM + save EEPROM). There is **no 6502** and **no external video ASIC**.

## Roadmap (phases)

We are starting at **Phase 1** (not phase 0). Later phases add the rest of the product. The tables and docs below describe that **final form**. Phase 1 is only the smallest path that proves RGBS on a real CRT.

### How we bring up hardware

We grow the console **one phase at a time** instead of assembling the full board and then sorting out every fault at once. Each phase documents, models, builds, and tests a bounded slice of hardware. If something fails, the cause is almost certainly in what that phase added.

Within each phase:

1. **Document** the hardware and how it should behave (pin map, nets, timing, expected picture / signals). Much of the final BOM is already planned in [`docs/`](docs/), so this step often refines known parts rather than inventing them.
2. **Model** that slice in SKiDL directly, or first in **Nano Sim** and then in SKiDL (see [`schematic_generator/`](schematic_generator/), including `generate_phase1.py`).
3. **Import** the updated / expanded netlist into KiCad. Generate or refresh a realistic BOM there (cross-check against the docs from step 1).
4. **Buy** only the new parts for that phase, wire them onto the current protoboard, and **test** on the bench (flash, probe, CRT).

That loop keeps documentation, design, implementation, and validation aligned. Full-board bring-up is deferred until the slices already work.

### Phase 1 (now): RGBS video lab

Goal: prove the video pipeline on a real CRT. Firmware holds a small pattern in ATmega RAM (black backdrop plus a few randomly placed colored squares from the **8** board FG colors), composes **128x96 @ ~60 Hz**, and drives **RGBS** at **256x192** via **2x**. No cart load yet.

| Need | Role |
|------|------|
| ATmega1284P **PDIP-40** | MCU. Flash open firmware. Hold the RAM pattern |
| External **20 MHz** crystal (+ load caps) | Solid clock from Phase 1 (same preferred rate as the final board) |
| 1 or 2 protoboards | Wire MCU + crystal + resistor DAC + power only |
| USBasp | Flash the ATmega (ISP / `J_ISP` pinout) |
| DC barrel jack to screw-terminal adapter | Plug Mean Well **+5V** into the female jack. Screw jumpers on **+** / **-** into the proto rails |
| Resistors | Build analog **R / G / B / CSYNC** (RGBS) per [`docs/pinmap.md`](docs/pinmap.md) |

**Out of scope for Phase 1:** player input, audio, cart flash, save EEPROM, AD725 / composite, sprites, Studio cart load on the MCU.

**Success:** flash via USBasp, run a minimal scanline kernel, see a stable **128x96** (shown **2x**) picture of random colored squares on black over RGBS on the CRT.

### Later phases (final form)

Build toward the full Nano already planned in the docs:

| Area | Final form (docs) |
|------|-------------------|
| Worlds / screens | Up to **8** worlds, **16** present screens/world, **16x16** virtual grid |
| Picture | **1 bpp** tiles, **8** board FG colors, black backdrop, **128x96** logical, **256x192** RGBS via **2x** |
| Motion | Instant screen switch only (no scrolling) |
| Sprites | Soft display sprites (rebuilt after MAP base). Caps TBD in [`docs/graphics.md`](docs/graphics.md) |
| Players | **2P** pad bytes (Retr01 spirit) |
| Audio | **2 PWM** channels (music pulse + SFX), resistor-mixed |
| Cart | **25LC1024** (128 KB game) + **24C64** save, cute **8+8** edge |
| Firmware | Fixed open image on the 1284. Games are cart data |

| | Retr01 (full) | Retr01 Nano (final form) |
|--|---------------|--------------------------|
| CPU / host | 6502 + MCUs | Single ATmega1284P @ 20 MHz (preferred) |
| Storage | Large cart flash + 24C64 | **Cute cart:** **25LC1024** (128 KB, DIP-8) + **24C64** (DIP-8) |
| Firmware | Cart PRG + MCU assists | **Fixed open** 1284 firmware (community-flashable) |
| Video | Multi-layer tile / sprite pipeline | 1 bpp tiles, attr FG color, soft sprites (later) |
| Worlds | Up to 8, rich MAP | Up to **8** worlds, **16** screens/world, **16x16** screen grid |
| Resolution (target) | 128x120 logical playfield | **128x96** logical, **256x192** RGBS via **2x** |
| Motion | Pixel scroll + streaming | **Instant screen switch only** |
| Players | 2P arcade GPIO | **2P** same pad-byte spirit |
| Audio | Multi-channel APU path | **2 PWM channels** (music pulse + SFX), resistor-mixed |

Primary product goal: a **minimal, stable 60 Hz** board with the smallest carts we can get away with, still feeling like Retr01. See [`docs/video.md`](docs/video.md) and [`docs/overview.md`](docs/overview.md).

## Host apps (Studio / Emu / Sim)

For now, **Nano Studio**, **Nano Emu**, and **Nano Sim** are **independent** trees under [`apps/`](apps/) (and root launchers). They may start as copies of the full Retr01 tools and diverge freely.

Later they are intended to **share common code** with the full Studio / Emu / Sim apps (shared libraries or extracted modules) instead of remaining permanent forks. Until that merge, prefer fixing Nano behavior in the Nano tree and keep full apps unchanged unless a change is deliberately shared.

**Apps:** [`apps/studio/`](apps/studio/) is Nano Studio. [`apps/emu/`](apps/emu/) is Nano Emu (MAP preview). [`apps/sim/`](apps/sim/) is the IC/netlist board. See [`apps/README.md`](apps/README.md).

| Runner | Picture | Host audio overlay |
|--------|---------|--------------------|
| Nano Studio Play | MAP preview (export + emu) | Yes |
| `./nano_emu` | MAP preview | No |
| `./nano_sim` | Soft compose MAP | No |

Host runners are **ahead of Phase 1 silicon**. They already compose MAP tiles from a cart image. Phase 1 MCU firmware does not load carts yet. It only proves RGBS from patterns in ATmega RAM.

## Docs

| Doc | Topic |
|-----|--------|
| [`docs/overview.md`](docs/overview.md) | Goals, cuts, roadmap |
| [`docs/graphics.md`](docs/graphics.md) | Worlds, screens, attr byte, MAP compose |
| [`docs/video.md`](docs/video.md) | RGBS, 2x to 256x192, line buffers, VBlank loads |
| [`docs/memory_and_software.md`](docs/memory_and_software.md) | MCU vs cart budgets, open firmware, SDK |
| [`docs/cache_architecture.md`](docs/cache_architecture.md) | How 16 KB SRAM caches cart MAP/CHR without filling to the brim |
| [`docs/sound.md`](docs/sound.md) | 2-channel music + SFX PWM |
| [`docs/cart_format.md`](docs/cart_format.md) | `.r01proj` + `.r01nano` / 128 KB flash layout |
| [`docs/hardware.md`](docs/hardware.md) | Mobo + cute cart connector, BOM sketch |
| [`docs/pinmap.md`](docs/pinmap.md) | MCU PORT / PDIP-40 map, headers, cart edge |
| [`schematic_generator/`](schematic_generator/) | SKiDL. Phase 1: `generate_phase1.py` -> `nano_phase1.net`. Full: `nano_mobo.net` + `nano_cart.net` |

From the repo root:

| Command | Role |
|---------|------|
| `./nano_studio [project.r01proj]` | Nano Studio (`bin/nano_studio` from `./build-all`) |
| `./nano_emu [cart.r01nano]` | Nano Emu MAP preview (`bin/nano_emu`) |
| `./nano_sim [cart.r01nano]` | Nano Sim: IC/netlist board UI (islands + DIPs + SCR), not a fullscreen emu |

Source concept notes also live in `temp/Retr01_Nano_Spec.md`. Where this tree disagrees, **these docs win**.

## Non-goals (final product v1)

- Compatibility with full Retr01 `.retr01` cart images or 36-pin edge
- Composite / NTSC encoder chips on the Phase 1 lab (AD725 remains a later board option)
- Hardware sprites, BG0, any screen scrolling
- Full Retr01 Sim netlist reuse

**Authoring:** Nano Studio exports `.r01nano` today (MAP + CHR). Console firmware / C SDK on the 1284 start with Phase 1 RGBS bring-up. Direction: open **C** firmware/SDK plus a tiny assembly video kernel. Game data lands on the SPI cart in later phases.

## Screenshots

<img src="../app/assets/png/cart_nano.png" alt="Nano cart" />

<img src="../app/assets/png/main_pcb_nano.png" alt="Nano Main PCB" />

<img src="../app/assets/png/sim_nano.png" alt="Nano Sim" />

<img src="../app/assets/png/studio_nano.png" alt="Nano Studio" />

## License / ownership

Same project umbrella as Retr01. Console firmware is intended to stay **open** for the community. See the parent repo for licensing when published.
