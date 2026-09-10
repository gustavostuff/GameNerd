# Video

**Status: design.** Timing numbers below are targets, not measured silicon.

## Output

- Progressive **RGBS** on **J_AV** (bring-up / RGB monitors / upscalers)
- **NTSC composite** on **J9** via **AD725** (same path as full Retr01)
- Mono PWM audio on **J_AV** and **J8** RCA

Pixels are **1 bpp**. Hardware presents:

- Backdrop: always **black**
- Foreground: one of **8** fixed board colors (resistor DAC / GPIO levels), chosen per MAP tile via attributes

RGB guns + CSYNC feed both the header and the AD725 AC-coupled inputs.

## Resolution and rate

| Item | Target |
|------|--------|
| Logical playfield | **128** wide x **96** tall (authoring / kernel compose) |
| RGBS active output | **256** wide x **192** tall (**2x** nearest-neighbor) |
| Frame rate | **~60.0 to 60.1 Hz** |
| Tile size | **8x8** (logical) |
| Screen | **16x12** tiles |
| Stretch goal | logical 128x100 with tighter code (still 2x on the wire) |

Primary ship target: compose **128x96 @ 60 Hz**, then shift each logical pixel out **twice** horizontally and emit each scanline **twice** vertically so the RGBS picture is **256x192**.

That keeps MAP/CHR math on a small 128x96 grid while the analog picture sits closer to classic 256x192 class displays and is easier to see on CRTs and capture boxes.

```text
Logical (SRAM compose)     RGBS out (timed kernel)
  128 x 96                   256 x 192
  one cell = 1 bit           each cell -> 2x2 identical samples
```

How 2x is done (planning):

- **H:** pixel clock / shift loop outputs each 1 bpp sample for two pixel periods
- **V:** after composing a logical line, output that line buffer twice (or duplicate in the active-line path)

Studio, Emu, and Sim should treat **128x96** as the source of truth and apply the same 2x presentation when showing the playfield.

## Line buffer strategy

Two line buffers in internal SRAM:

- 16 bytes each (128 logical pixels at 1 bpp for the shift-out path), or an equivalent composed form
- While one line is output to RGBS (including its vertical duplicate), the next logical line is rendered from the composed nametable
- Swap every **logical** scanline (each logical line still occupies two RGBS lines on the wire)

```text
Active display:
  Timed loop outputs the current line buffer at 2x H (and the line again for 2x V)
  Remaining cycles + HBlank render the next logical line
  (apply bank, flip, FG color from the MAP row)

VBlank:
  Game logic (future)
  Instant screen load from cart SPI (nametable +/- CHR)
  Audio tick
  Pad read (future)
```

## Rendering model

See [`graphics.md`](graphics.md) for worlds, attr layout, and MAP compose.

Hot path assumptions:

- **MAP tiles only** (opaque cells: bit0 = black backdrop, bit1 = FG)
- No soft sprites yet
- No scroll registers
- Backdrop black simplifies the DAC (FG only needs 8 levels plus grounded black)

## Screen switches (not scrolling)

There is **no** tile scroll and **no** pixel scroll.

A screen change copies a new **384-byte** nametable from cart flash into RAM during VBlank. That is the intended navigation model (doors, map edges, menus).

## Kernel notes

- Mostly **C** in the open console firmware for game services
- Small **assembly** video kernel for the timed active-line loop (including 2x pixel/line emission)
- Cart SPI traffic stays in world/screen enter (VBlank). Active lines use the SRAM cache only ([`cache_architecture.md`](cache_architecture.md)).

## What full Retr01 does differently

Full Retr01 uses MCU-assisted fetch, VRAM islands, sprites, BG0, and scroll latches. Nano is a **software scanline kernel** plus an in-RAM screen buffer fed by a tiny SPI cart.
