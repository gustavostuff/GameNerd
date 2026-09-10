# Graphics

**Status: design + emu Host Play.** Ahead of console firmware. This is the Nano picture model.

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
|________________ SOLID (world collision, software)
```

| Field | Bits | Role |
|-------|------|------|
| BANK | 1:0 | Which of the **4** BG banks for this tile |
| FLIP_H | 2 | Mirror horizontally when drawing |
| FLIP_V | 3 | Mirror vertically when drawing |
| FG | 6:4 | Foreground color index (**8** board colors) |
| SOLID | 7 | Collision for rocks, trees, labyrinth walls, etc. Video may ignore this bit |

Tile **1** bits are drawn with the attr FG color. Tile **0** bits are backdrop **black**. There is no second background layer.

## CHR banks

- Up to **4** banks per world on the cart
- 1 bpp patterns (8 bytes per 8x8 tile if packed like a classic planar tile)
- Bank count and tiles per bank are sized to fit **25LC1024** (128 KB) with maps and music. Exact tiles/bank freeze with the cart image format later.

Preferred runtime: on world enter, cache **all four** CHR banks in MCU SRAM (see [`cache_architecture.md`](cache_architecture.md)). Screen switches then SPI-load MAP only.

## Motion model (no scrolling)

Scrolling is **out**.

On a screen change (door, edge warp, menu):

1. Stop using the old in-RAM nametable for display after the current frame.
2. During **VBlank**, SPI-read the new screen's **384-byte** nametable (and any CHR misses) from cart flash into RAM.
3. Next active frame draws the new screen.

**VBlank fit:** One nametable load from SPI is comfortable. World CHR (about 8 KB planning) loads across one or more VBlanks when entering a world. Details: [`cache_architecture.md`](cache_architecture.md).

## Entities and display sprites

Up to **64** entities live in MCU RAM for game logic.

Each entity (minimum fields):

| Field | Size | Notes |
|-------|------|-------|
| pixel_x, pixel_y | world / screen pixels | Motion integration **and** sprite draw origin |
| tile_x, tile_y | derived (`pixel / 8`) | MAP SOLID, screen presence, enter rules |
| tile / state | pattern index | Current **8x8** CHR (swappable / anim state) |
| color | FG 0-7 | Board resistor colors |

Movement strategies use pixel coords. Default is **`PIXEL_CONTINUOUS`** (1 px/frame, no press/release tile snap). See [`movement.md`](movement.md).

### Display sprites (picture)

Entities expand to a **display sprite list** for the video path (v1: one entity -> one 8x8 sprite).

| Cap | Value |
|-----|-------|
| Max sprites on screen | **24** |
| Max sprites per scanline | **8** (overflow dropped by priority) |
| Size | **8x8**, 1 bpp CHR |
| Draw position | `pixel_x`, `pixel_y` (screen-relative) |
| Transparency | Bit **1** = FG color. Bit **0** leaves MAP (or lower sprites) |

**Player** is always a display sprite (priority slot 0): drawn at `player_px` / `player_py`, not snapped to the tile cell.

### Draw priority

```text
1. MAP (opaque cells: bit0 = black backdrop)
2. Sprites low -> high (player first), transparent 0-bits
```

Collision against the world still uses MAP attr **SOLID** on **tile** coords. Sprite pixels do not punch MAP collision.

### Pipeline sketch (each frame)

```text
1. In-RAM screen nametable (cart MAP load)
2. Expand active entities (+ player, lasers) -> up to 24 sprites
3. Per line (or host full-frame): MAP row, then up to 8 sprite strips
4. Scanline kernel shifts the composed line (2x on RGBS)
```

Host Play / Emu compose the full logical frame in software with the same caps.

## Compared to full Retr01 attr

Parent BG attr uses bank, pal 0-3, flips, solid, anim. Nano uses bank, flips, **3-bit FG color**, solid. No anim bit in v1. No hardware palettes via Color PROM: the **8** FG colors are board resistor levels.
