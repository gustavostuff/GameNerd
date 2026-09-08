# Memory and software

**Status: design.** Budgets are planning numbers.

## Split of responsibility

| Where | Holds |
|-------|--------|
| ATmega1284P Flash (128 KB) | **Fixed open console firmware**: video kernel, cart SPI/I2C drivers, entity/compose helpers, pad/PWM services |
| ATmega1284P SRAM (16 KB) | See [`cache_architecture.md`](cache_architecture.md): full world CHR cache, current MAP, entities, reserved headroom |
| ATmega1284P EEPROM (4 KB) | Machine config only (not game saves) |
| Cart SST25VF010A (128 KB) | Game image: maps, CHR banks, music/tables, soft logic data |
| Cart 24C64 (8 KB) | Per-game saves (always fitted on the cart PCB) |

Flashing the **1284** (motherboard **J_ISP**) updates the shared console. Flashing the **cart SPI** (same **8+8** edge, or the bench flasher’s cart slot) installs or replaces a game. One bench tool can carry **both** a 2×8 cart socket and a 2×3 ISP header ([`hardware.md`](hardware.md#programming--two-ports-one-bench-tool)). Community firmware builds are intentional.

## MCU SRAM sketch

High level only. The live map is in [`cache_architecture.md`](cache_architecture.md).

| Block | Size | Notes |
|-------|------|-------|
| CHR cache (4 banks) | **8 KB** | Entire active world, planning 256 tiles x 8 B |
| Current screen nametable | **384 B** | 16x12 x (tile + attr) |
| Prefetch nametable (optional) | **384 B** | Next screen MAP |
| Entity table | **~1 KB** | Up to 64 x 16 B |
| Line buffers | **32 B** | Scanout |
| World directory + audio scratch | **~0.5 KB** | |
| Reserved headroom | **~2.5 KB** | Do not allocate caches here |
| Stack / BSS | **~1.2 KB** | Firmware + peak stack |

Rule: active scanline code never SPI-reads. Refills happen on world/screen enter (VBlank).

## Cart flash budget (128 KB)

Rough planning (see [`cart_format.md`](cart_format.md) for the exported layout):

| Region | Order-of-magnitude |
|--------|--------------------|
| Header / directory | small |
| Up to 8 worlds x 16 screens x 384 B MAP | up to ~48 KB if dense |
| CHR (4 banks/world, 1 bpp, 8 KB/world) | fits if worlds share or stay modest |
| Music / tables / extras | remainder |

## Save EEPROM

**24C64** is **always present** on the cart so the motherboard and cart copper stay simple (no populate jumpers). Games that do not save can ignore it. I2C master is the **1284**.

## Entities in RAM

Cap: **64**.

Minimum fields per entity:

- `pixel_x`, `pixel_y` (sub-tile motion)
- `tile_x`, `tile_y`
- `tile` (current pattern index, swappable)
- `color` (FG in low bits, upper bits reserved)

See [`graphics.md`](graphics.md) for draw priority (entity opaque over MAP).

## Input software contract

Match the **spirit** of full Retr01 pad ports:

| Player | Byte | Bit meaning (1 = pressed) |
|--------|------|---------------------------|
| P1 | pad0 | bit0 Right, bit1 Left, bit2 Down, bit3 Up, bit4 X, bit5 Y, bit6 Coin/Select, bit7 Start |
| P2 | pad1 | same layout |

## Audio software

- **Two** hardware **PWM** channels: music (pulse) and SFX
- Updated in **VBlank** / on SFX trigger (not in the active video kernel)
- Analog resistor mix to one jack ([`sound.md`](sound.md))

Music data can live on the cart. The console firmware owns both PWM ticks.

## Open firmware / C SDK (TBD)

Product direction:

- Console firmware stays **open** (community forks welcome)
- Thin HAL for cart read, screen load, entity table, pads, PWM
- Assembly video kernel as a fixed object inside that firmware
- Game authors ship **cart images**, not a private MCU binary (unless they also ship a custom open firmware build)

Exact host toolchain (avr-gcc Make, etc.) is still TBD. Authoring UI beyond hand tools is TBD.

## Performance outlook

- Stable 60 Hz at 128x96 should be comfortable
- VBlank nametable SPI loads are the normal screen-change path
- Entity stamp cost scales with live entity count (budget for 64)

## Validation path

1. Kernel + black backdrop + FG colors
2. RAM nametable from a test pattern
3. SPI cart screen load in VBlank
4. Entity stamp + 2P pads + PWM beep
5. Tiny sample cart
