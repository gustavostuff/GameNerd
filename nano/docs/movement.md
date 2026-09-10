# Movement

**Status: design + host Play default.** Soft entities use world **pixel** coords for integration **and** sprite draw. **Tile** coords (`pixel / 8`) drive MAP collision and screen switch.

Studio export stores instance `world_x` / `world_y` as pixels (often tile-aligned from the editor snap). Host Play / emu spawn from that cart data, then move in pixels.

## Coords

| Field | Role |
|-------|------|
| `pixel_x`, `pixel_y` | World pixels. Motion + **sprite top-left**. |
| `tile_x`, `tile_y` | Occupied MAP cell (`pixel / 8`). SOLID probes and screen switch. |

Local pixel inside a tile: `(pixel_x % 8, pixel_y % 8)`.

## Collision

A tile is enterable only if:

1. It lies on a **present** screen (missing screen / world hole = blocked), and
2. That MAP cell is **not** `SOLID`.

Checked when a pixel step would cross into a new tile.

## Strategies

### `PIXEL_CONTINUOUS` (default)

**Feel:** hold a direction and the sprite slides **1 px / frame**. No press jump. No release snap. Crossing into the next cell only happens after 8 px and only if that cell is enterable.

This is the sprite-entity default.

### `TILE_ENTER_PIXEL` (legacy)

Press from local `(0,0)` jumps into the next tile. Release snaps that axis back to local 0. Kept for experiments, not the Host Play default.

### Future

| Id (planned) | Intent |
|--------------|--------|
| `TILE_GRID` | One full tile per press or per N frames |
| `PHYSICS` | Acceleration / momentum / gravity |

## Draw reminder

**Sprites draw at `pixel_x` / `pixel_y`.** Caps: [`graphics.md`](graphics.md) (24 on screen, 8 per scanline).
