# Video

**Status: design.** Timing numbers below are targets, not measured silicon.

## Output

- Progressive **RGBS** analog only (v1)
- No AD725-style RGB-to-NTSC chip
- Aimed at 15 kHz CRTs and upscalers that accept RGBS

Pixels are **1 bpp**. Hardware presents:

- Backdrop: always **black**
- Foreground: one of **8** fixed board colors (resistor DAC / GPIO levels), chosen per tile or entity via attributes

## Resolution and rate

| Item | Target |
|------|--------|
| Active pixels | **128** wide x **96** tall |
| Frame rate | **~60.0 to 60.1 Hz** |
| Tile size | **8x8** |
| Screen | **16x12** tiles |
| Stretch goal | 128x100 with tighter code |

Primary ship target is **128x96 @ 60 Hz**.

## Line buffer strategy

Two line buffers in internal SRAM:

- 16 bytes each (128 pixels at 1 bpp for the shift-out path), or an equivalent composed form
- While one line is output to RGBS, the next is rendered from the composed nametable
- Swap every scanline

```text
Active display:
  Timed loop outputs the current line buffer
  Remaining cycles + HBlank render the next line
  (apply bank, flip, FG color, entity-already-stamped cells)

VBlank:
  Game logic
  Instant screen load from cart SPI (nametable +/- CHR)
  Entity pixel integration
  Compose entity stamps into the RAM screen
  Audio tick
  Pad read
```

## Rendering model

See [`graphics.md`](graphics.md) for worlds, attr layout, and entities.

Hot path assumptions:

- No sprites, no priority stack beyond **entity over MAP**
- No scroll registers
- Backdrop black simplifies the DAC (FG only needs 8 levels plus grounded black)

## Screen switches (not scrolling)

There is **no** tile scroll and **no** pixel scroll.

A screen change copies a new **384-byte** nametable from cart flash into RAM during VBlank. That is the intended navigation model (doors, map edges, menus).

## Kernel notes

- Mostly **C** in the open console firmware for game services
- Small **assembly** video kernel for the timed active-line loop
- Cart SPI traffic stays in world/screen enter (VBlank). Active lines use the SRAM cache only ([`cache_architecture.md`](cache_architecture.md)).

## What full Retr01 does differently

Full Retr01 uses MCU-assisted fetch, VRAM islands, sprites, BG0, and scroll latches. Nano is a **software scanline kernel** plus an in-RAM screen buffer fed by a tiny SPI cart.
