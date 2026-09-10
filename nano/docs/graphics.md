# Graphics

**Status: design + emu MAP compose.** Ahead of console firmware. This is the Nano picture model.

## Playfield

| Item | Value |
|------|-------|
| Active display | **128x96** logical pixels (**256x192** on RGBS after 2x) |
| Tile size | **8x8** |
| Tiles per screen | **16x12** |
| Pixel format | **1 bpp** in CHR |
| Backdrop | Always **black** (software and resistor DAC assume this) |

Some authored layouts may leave empty bands inside 128x96. That is fine.

## Worlds and screens

Similar spirit to full Retr01, smaller caps:

| Cap | Value |
|-----|-------|
| Worlds | Up to **8** |
| Present screens per world | Up to **16** |
| Virtual screen grid | **16x16** |
| BG CHR banks per world | **4** |

Screens are placed sparsely on the 16x16 grid (not every cell filled). Crossing into another present screen uses an **instant switch**, not a scroll.

## Nametable cell

Each cell is two bytes (same idea as the parent, different attr):

| Offset | Size | Meaning |
|--------|------|---------|
| +0 | 1 | Tile index within the bank selected by attr |
| +1 | 1 | Attribute byte (below) |

One screen in RAM: 16x12 x 2 = **384 bytes**.

## Attribute byte

```text
BG attr byte
7 6 5 4 3 2 1 0
| | | | | | |_|__ BANK 0-3
| | | | |_|______ FLIP_H (bit2), FLIP_V (bit3)
| |_|_|__________ FG color 0-7
|________________ SOLID (authoring / future collision; video ignores)
```

| Field | Bits | Role |
|-------|------|------|
| BANK | 1:0 | Which of the **4** BG banks for this tile |
| FLIP_H | 2 | Mirror horizontally when drawing |
| FLIP_V | 3 | Mirror vertically when drawing |
| FG | 6:4 | Foreground color index (**8** board colors) |
| SOLID | 7 | Paint metadata for later game logic. Video ignores this bit |

Tile **1** bits are drawn with the attr FG color. Tile **0** bits are backdrop **black**. There is no second background layer.

## CHR banks

- Up to **4** banks per world on the cart
- 1 bpp patterns (8 bytes per 8x8 tile)
- Bank count and tiles per bank are sized to fit **25LC1024** (128 KB) with maps and music

Preferred runtime: on world enter, cache **all four** CHR banks in MCU SRAM (see [`cache_architecture.md`](cache_architecture.md)). Screen switches then SPI-load MAP only.

## Motion model (no scrolling)

Scrolling is **out**.

On a screen change (door, edge warp, menu):

1. Stop using the old in-RAM nametable for display after the current frame.
2. During **VBlank**, SPI-read the new screen's **384-byte** nametable (and any CHR misses) from cart flash into RAM.
3. Next active frame draws the new screen.

**VBlank fit:** One nametable load from SPI is comfortable. World CHR (about 8 KB planning) loads across one or more VBlanks when entering a world. Details: [`cache_architecture.md`](cache_architecture.md).

## Picture compose (current host)

Host / emu compose is **MAP only**:

```text
1. In-RAM screen nametable (cart MAP load)
2. Draw all 16x12 opaque MAP tiles into the logical 128x96 buffer
3. Scale 2x to the RGBS FB (256x192)
```

Sprites / soft entities are **not** in the picture path yet. They will be rebuilt from this MAP-only base.

## Compared to full Retr01 attr

Parent BG attr uses bank, pal 0-3, flips, solid, anim. Nano uses bank, flips, **3-bit FG color**, solid. No anim bit in v1. No hardware palettes via Color PROM: the **8** FG colors are board resistor levels.
