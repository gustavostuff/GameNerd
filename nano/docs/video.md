# Video

**Status: design.** Timing numbers below are targets, not measured silicon.

## Output

- Progressive **RGBS** analog only (v1)
- No AD725-style RGB-to-NTSC chip
- No composite RCA path required on the board
- Aimed at 15 kHz CRTs and upscalers that accept RGBS

Color is **1 bpp**: two software-chosen levels. Examples: white on black, amber on black, green on black, inverted black on white. Levels come from a small resistor network driven by GPIO (not an 8-bit R-2R DAC).

## Resolution and rate

| Item | Target |
|------|--------|
| Active pixels | **128** wide x **96** tall |
| Frame rate | **~60.0 to 60.1 Hz** |
| Tile size | **8x8** |
| Stretch goal | 128x100 with tighter code |
| Hard stretch | 128x120 only with heavy assembly optimization |

Primary ship target is **128x96 @ 60 Hz**. Do not chase 128x120 until the kernel is boringly stable.

## Line buffer strategy

Two line buffers in internal SRAM:

- 16 bytes each (128 pixels at 1 bpp)
- **32 bytes** total
- While one line is shifted out to RGBS, the other is rendered
- Swap every scanline

```text
Active display:
  Timed loop outputs the current line buffer
  Remaining cycles + HBlank render the next line

VBlank:
  Game logic
  Scroll / screen pointer updates
  Audio tick
  Pad read
```

This is predictable: no sprites, no priority, no transparency tests in the hot path.

## Rendering model

- Pure **tilemap** (nametable of 8x8 tile indices)
- Example map size: 16x12 tiles for a 128x96 field (~192 bytes)
- CHR (tile bitmaps) live in Flash
- CPU copies or streams decoded pixels into the next line buffer

No OAM. No sprite evaluation. Player / enemy graphics are baked into the tilemap (or redrawn by game code into tiles) if needed.

## Scrolling and screens

Two cheap modes:

1. **Tile-level scroll**  
   Change the nametable start column / row. Coarse motion only.

2. **Instant screen switch**  
   Swap the base pointer (or copy a new screen). Classic handheld / early arcade feel.

Fine pixel scrolling is **out of scope**.

## Kernel notes (implementation direction)

- Mostly **C** for game and setup
- Small **assembly** video kernel for the timed active-line loop
- Colors / sync polarity programmable where the resistor DAC allows
- Runtime color swaps are allowed (flashes, modes) as long as the line loop stays deterministic

## What full Retr01 does differently

Full Retr01 uses MCU-assisted tile fetch, VRAM islands, sprites, and a richer playfield. Nano replaces that pipeline with a **software scanline kernel** on one chip. Same 128-wide *language*, different machine.
