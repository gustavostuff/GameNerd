# Movement

**Status: design + host Play default.** Soft entities use world **pixel** coords for integration and **tile** coords for draw / MAP collision. Picture stamps always use the tile cell (`tile_x * 8`, `tile_y * 8`) — no sub-tile sliding on screen.

## Coords

| Field | Role |
|-------|------|
| `pixel_x`, `pixel_y` | World pixels. Resting pose is local **(0, 0)** inside the occupied tile (top-left). |
| `tile_x`, `tile_y` | Occupied MAP cell. Derived when entering a tile; used for draw, solid probes, screen switch. |

Local pixel inside a tile: `(pixel_x % 8, pixel_y % 8)`.

## Collision

A tile is enterable only if:

1. It lies on a **present** screen (missing screen / world hole = blocked), and
2. That MAP cell is **not** `SOLID`.

Checked on every tile enter (press snap and boundary cross).

## Strategies

Host Play and future game code pick a **movement strategy** per entity (or globally). Only the default is implemented in nano emu today.

### `TILE_ENTER_PIXEL` (default)

**Feel:** first press jumps into the next cell immediately; continued hold walks that cell at **1 px / frame**.

**Rest:** local pixel **(0, 0)**.

**On direction press** (edge) while at local **(0, 0)** — if the adjacent tile is enterable, snap into it at the direction entry pixel:

| Direction | Destination local pixel `(col, row)` |
|-----------|--------------------------------------|
| Right | `(0, 0)` |
| Left | `(7, 0)` |
| Down | `(0, 0)` |
| Up | `(0, 7)` |

**While held** after that: each frame moves **1 px** along the held axis. Crossing into another tile uses the same entry pixels and the same solid / screen checks. If blocked, stay in the current tile (no enter).

**On release** of an axis: snap that axis’s pixel back to local **0** in the **current** tile (the cell you are in when you let go). The other axis is unchanged. Tile identity does not jump.

| Released | Reset |
|----------|--------|
| Left or Right | `pixel_x = tile_x * 8` (local col 0) |
| Up or Down | `pixel_y = tile_y * 8` (local row 0) |

So mid-tile progress only lasts while held; release returns that axis to rest origin so the next press can snap cleanly again.

**Pose / state on release:** keep the **last movement state** (and its CHR tile / flip). Do **not** switch back to `idle` when the stick is released. `idle` is the spawn / never-moved pose only. Same rule for any entity that uses directional states.

**Why the press snap:** from rest, Right/Down would otherwise only inch inside the current tile. The snap makes the first response a full cell enter so motion feels immediate; later frames are time-based at pixel rate.

### Future strategies (not implemented)

| Id (planned) | Intent |
|--------------|--------|
| `PIXEL_CONTINUOUS` | No press snap; every frame is ±1 px with boundary entry pixels only when crossing. |
| `TILE_GRID` | One full tile per press or per N frames; ignore sub-tile pixels for gameplay. |
| `PHYSICS` | Acceleration / momentum / gravity; tile enter rules TBD. |

Document new strategies here when they land; keep Host Play default as `TILE_ENTER_PIXEL` unless a cart/game opts out.

## Draw reminder

Regardless of strategy, **draw uses `tile_x` / `tile_y` only**. Pixel coords exist so step timing and later physics can live under the soft-tile picture.
