# Cart cache architecture (ATmega1284P)

**Status: design.** MCU stays **ATmega1284P** (16 KB SRAM). Cart flash is the source of truth. Firmware **caches aggressively** into SRAM for the live world/screen, and **leaves explicit headroom** so the kernel never sits at 100% RAM.

Related: [`memory_and_software.md`](memory_and_software.md), [`graphics.md`](graphics.md), [`video.md`](video.md).

## Goals

1. **Never SPI-fetch in the active scanline kernel.** RGBS timing plays only from SRAM.
2. **Cache everything that makes 60 Hz easy:** current MAP screen, all **4** CHR banks for the active world, world directory, entity table, small audio scratch.
3. **Do not saturate** the 16 KB. Keep a reserved free band for stack growth and future firmware.
4. Cart remains authoritative. Caches are disposable views refilled on world/screen changes.

## Planning assumptions (freeze later with cart format)

| Item | Planning value | Notes |
|------|----------------|-------|
| Tiles per BG bank | **256** | Index is one byte |
| Bytes per 8x8 1 bpp tile | **8** | |
| CHR bank size | **2 KB** | 256 x 8 |
| Four banks / world | **8 KB** | Fits in 1284P with room to spare |
| Screen nametable | **384 B** | 16x12 x (tile + attr) |
| Max entities | **64** | Logic table. Display sprites: **24** / **8** per line |

If a future cart format uses fewer tiles per bank, the CHR cache shrinks and headroom grows.

## SRAM map (16 KB)

Target fill: about **70-80%** used when a world is hot. About **20-30%** kept free.

```text
+---------------------------+  0
| Soft stack + ISR slack    |  ~0.75 KB  (grows downward in practice)
+---------------------------+
| Firmware BSS / globals    |  ~0.5 KB
| Pads, PWM, frame counters |
+---------------------------+
| RESERVED HEADROOM         |  ~2.5 KB   never assigned to caches
+---------------------------+
| Audio scratch / song ptrs |  ~0.25 KB
+---------------------------+
| Entity table (64)         |  ~1.0 KB   16 B/record planning
+---------------------------+
| Screen MAP cache          |  384 B     current nametable
| Screen MAP prefetch (opt) |  384 B     next screen (optional)
+---------------------------+
| World directory cache     |  ~0.25 KB  present-screen list / offsets
+---------------------------+
| CHR cache bank 0..3       |  8 KB      full world CHR
+---------------------------+
| Compose / stamp scratch   |  ~0.25 KB  optional dirty helpers
+---------------------------+
| Double line buffers       |  32 B      scanout
+---------------------------+  16 KB
```

### Budget table

| Region | Size | Lifetime |
|--------|------|----------|
| CHR banks 0-3 (active world) | **8192 B** | Until world change |
| Current screen nametable | **384 B** | Until screen switch |
| Prefetch nametable (optional) | **384 B** | Until consumed / discarded |
| World directory | **~256 B** | Until world change |
| Entity table | **~1024 B** | Session (cleared on screen/world as game rules say) |
| Line buffers | **32 B** | Permanent |
| Audio scratch | **~256 B** | Permanent / track change |
| Firmware BSS + pads | **~512 B** | Permanent |
| **Reserved headroom** | **~2560 B** | Untouched by cache allocator |
| Stack peak allowance | **~768 B** | Inside / beside headroom |

Exact linker layout can shuffle order. The numbers are the contract: **CHR all four banks resident**, **MAP current resident**, **~2.5 KB reserved empty**.

## Cache tiers

```text
COLD   Cart SPI only (25LC1024)
         |
         |  world enter (multi-VBlank OK)
         v
WARM   World directory + CHR banks 0..3 in SRAM
         |
         |  screen enter (one VBlank)
         v
HOT    Current nametable (+ optional prefetch) + entities + linebufs
         |
         |  active display
         v
KERNEL Reads HOT/WARM SRAM only. No SPI.
```

### World enter (WARM fill)

When `world` changes:

1. SPI-read world directory into RAM.
2. SPI-read **all four** CHR banks into the 8 KB CHR cache.
3. Clear or rebuild entity table as the game requests.
4. May span **several VBlanks** (8 KB SPI is fine across a few frames at 60 Hz). Show a black frame or previous screen until WARM is valid.

### Screen enter (HOT fill)

When the screen switches (only motion model):

1. SPI-read **384 B** nametable for the new screen into the current MAP slot (one VBlank, easy).
2. If prefetch holds that screen already, **swap pointers** instead of reading.
3. Optionally kick a prefetch read for a predicted neighbor (door / edge) into the second 384 B slot during later spare VBlank time.
4. Entity stamps apply in RAM after MAP is valid (see below).

### Active frame (no SPI)

Each frame in HOT:

1. Start from cached nametable.
2. Stamp up to **64** entities (opaque over MAP).
3. Scanline kernel walks tiles, indexes **CHR cache[bank][tile]**, applies flip + FG color, backdrop black.
4. Game logic may move entities using pixel/tile coords. It must not touch SPI in the timed region.

## What is not cached (stays on cart)

- Other worlds CHR/MAP
- Cold music banks beyond the small scratch / current pattern
- Save payloads (go through 24C64 when needed)
- Large script blobs until the SDK defines streaming windows

This keeps the 16 KB from turning into a second cart image.

## Prefetch policy (optional but recommended)

Because Nano has **no scrolling**, the expensive case is only screen switches.

| Policy | Behavior |
|--------|----------|
| None | Every switch SPI-reads 384 B in VBlank (still OK) |
| Single prefetch | After a screen loads, read one predicted neighbor MAP into the second slot |
| Pointer swap | On walk into the prefetched screen, swap current/prefetch and refill the empty slot |

CHR does **not** need prefetch if all four banks for the world are already WARM.

## Entity record (planning)

16 bytes per entity keeps alignment simple (64 x 16 = 1024 B):

| Offset | Field | Notes |
|--------|-------|-------|
| 0-1 | pixel_x | Sub-tile |
| 2-3 | pixel_y | Sub-tile |
| 4 | tile_x | |
| 5 | tile_y | |
| 6 | tile | Pattern index (bank policy TBD: fixed bank or field later) |
| 7 | color | FG in low bits, upper reserved |
| 8-15 | reserved / flags | Speed, anim, owner, etc. later |

Entity patterns come from the **same CHR cache** (or a reserved tile range). No per-entity SPI.

## Failure / overflow rules

- If a cart world declares more CHR than 8 KB, firmware **rejects** the world or loads a reduced bank set (format must declare sizes).
- Cache allocator must **never** eat the reserved headroom.
- If SPI underruns a WARM fill, stay on black / last good frame. Do not enter the active kernel with incomplete CHR pointers.

## Why 1284P (not 644) for this plan

| | ATmega644 (4 KB) | ATmega1284P (16 KB) |
|--|------------------|---------------------|
| All 4 CHR banks resident | No (would dominate RAM) | **Yes (~8 KB)** |
| MAP + entities + headroom | Tight / windowed CHR only | **Comfortable** |
| SPI only on world/screen edges | Required + CHR streaming | **World/screen edges only** |

Nano keeps **1284P** so "cache all we can" means **full world CHR + live MAP**, not a thin tile window.

## Summary

- Cart = source of truth  
- SRAM = full **world CHR** + **current screen MAP** + **entities** + linebufs  
- SPI = world enter + screen enter (+ optional MAP prefetch)  
- Active video = SRAM only  
- **~2.5 KB** deliberately left free  

That is the intended use of the 1284P memory advantage.
