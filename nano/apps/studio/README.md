# Retr01 Nano Studio

Visual authoring for Retr01 Nano worlds, screens, and **`.r01nano`** carts (128 KB / **25LC1024**).

Independent of full Retr01 Studio for now (started as a fork). Paths and caps are Nano. See [`../../docs/`](../../docs/) and [`../README.md`](../README.md).

**Stack:** C11 + SDL2 + FreeType (Proggy Tiny), `libretr01_studio_core` + shell + shared `retr01_nano_emu_core` for Play.

## What runs today

1. **Authoring (UI).** Edit worlds, tiles, palettes, soft entities / instances, Music + SFX planes.
2. **Export + Play.** **Ctrl+E** or **Play** packs `output/nano/<stem>.r01nano` (+ `_flash.bin` padded to 128 KB) and regenerates `output/nano/C/` as needed. **Play** embeds the Nano emu core so preview matches `./nano_emu` pixels.

Authoring state: `output/nano/<stem>.r01proj` (JSON). **`custom_logic.c`** is created on first export and never overwritten. Graphics contract: [`../../docs/graphics.md`](../../docs/graphics.md).

There is **no** Studio-only soft Play path. Preview always goes through export then shared Nano emu. **Sim is not involved.**

### Product caps (code SoT)

| Item | Value |
|------|--------|
| Playfield | **128x96** logical, preview **2x** to **256x192** |
| Present screens / world | **16** max (`R01_MAX_PRESENT_SCREENS`) |
| Cart | **`.r01nano`**, flash pad **128 KB** |
| Motion in Host Play | **Instant screen switch only** (no camera scroll) |
| Audio authoring | **1 Music** pulse lane + **SFX** plane (target: 2 HW PWM channels) |
| BG0 / hardware sprites | **Not product.** Leftover UI from the full-Studio fork may still appear. Do not treat as Nano features |

### Host Play audio matrix

| Runner | Picture + pads / move | Host BGM / SFX overlay |
|--------|------------------------|-------------------------|
| Nano Studio Play | Yes (via emu core) | Yes (host softsynth) |
| `./nano_emu` | Yes | **No** (audio lagging) |
| `./nano_sim` | Soft compose via emu core | **No** |

---

## Authoring (UI)

Fixed **640x360** or **1280x720** logical canvas (**Ctrl+Shift+R**). Present scale **Ctrl+1** / **Ctrl+2**. Top **Graphics | Audio** tabs. Proggy Tiny.

| Control | Behavior |
|---------|----------|
| **Worlds** | **8** world buttons. Sparse **16x16** screen map. Cap **16** present screens/world. White fill = default spawn |
| **Double-click** empty slot | Create screen |
| **Click** present | Select / edit target |
| **Ctrl+C / Ctrl+V** | Copy / paste selected screen |
| **Delete** / **Ctrl+click** | Remove present screen (prefer instance Delete when a sprite is selected) |
| **BG / Sprite layer** | Tile paint vs soft-entity instances |
| **Tile Sel / Paint** | Paint stamps armed tile+attr. **F+click** flood-fills |
| **Entities / place** | Drag catalog onto screen preview. Soft tiles, not OAM sprites |
| **Audio** | **Music** lane + **SFX** plane. Host softsynth preview only (not cart PWM protocol) |

PNG drop imports into the **active** world. Cart export packs **world 0** only.

---

## Play

**Play** (button or **Space**):

1. Runs the same **export** path as **Ctrl+E** (even if unsaved).
2. Shows a Studio-local boot wait UI while packing.
3. Embeds **nano emu** (`retr01_nano_emu_core`). Framebuffer matches `./nano_emu`.

| | |
|--|--|
| **Entry world** | Cart boots **world 0** |
| **Motion** | Instant screen switch. Spawn / warp snap. No smooth scroll |
| **Player** | World **`player_entity`**. Idle / Walk from `custom_logic.c` |
| **Other entities** | State 0 / frame 0 in Phase 1 Host Play |
| **Collision** | Anim-state hitbox vs `R01_ATTR_SOLID` on MAP attrs |
| **Warps** | **X** -> screen (0,0). **Y** -> screen (1,0). Test hooks |

Gameplay SoT: Nano emu Host Play + `custom_logic.c`. Move strategy: [`../../docs/movement.md`](../../docs/movement.md).

### `custom_logic.c` hooks

Created on first export. Never overwritten. Typical init:

```c
void r01_custom_on_init(R01GameCtx *ctx) {
    r01_player_anim_set_idle_state(ctx, 0);
    r01_player_anim_set_walk_all(ctx, 1);
    r01_entity_state_frame_delay_set(ctx, 0, 10);
    r01_entity_state_frame_delay_set(ctx, 1, 4);
    /* Nano Host Play ignores camera scroll. Dead-zone stubs may still exist in headers. */
}
```

See generated `output/nano/C/include/r01_*.h` for the engine API.

---

## Save / load (JSON)

**Ctrl+S** / **Ctrl+O** -> `output/nano/test.r01proj` by default. Quit does **not** auto-save.

| Field | Behavior |
|-------|----------|
| `version` | Current `R01_JSON_VER` |
| World data | Active world on save. Load applies to **world 0** |
| Worlds 1-7 | Session-only until multi-world JSON lands |

---

## PNG import

| Rule | Value |
|------|--------|
| Drop target | Active world, **BG bank 0** |
| Cell size | **128x96** px. PNG must be a multiple thereof |
| Grid | Sets world to **NxM** from atlas (max **16x16**) |
| Limits | <= **256** unique 8x8 tiles. <= **4** colors per PNG |
| Transparent cells | Skipped |

---

## Export

**Ctrl+E** writes under `output/nano/` (relative to launch cwd).

| Path | Contents |
|------|----------|
| `<stem>.r01proj` | Authoring JSON |
| `<stem>.r01nano` | Packed cart (**world 0**) |
| `<stem>_flash.bin` | Same image padded to **128 KB** |
| `C/base_game.c` | Regenerated tables / tick / vblank |
| `C/custom_logic.c` | **User file**. Template on first export |
| `C/include/*.h` | `R01GameCtx` + engine API |
| `ASM/**`, `data/*` | Sidecars for future on-target build. Not assembled during export today |

Contract: [`../../docs/cart_format.md`](../../docs/cart_format.md).

### Limitations (current)

- **World 0** only in cart export
- NPC instances: state 0 / frame 0 only in Phase 1 Host Play
- Leftover full-Studio UI (BG0 plane, camera dead-zone stubs) is **not** Nano product behavior
- No ca65 step in the default export path

---

## Build and run

From the repo root:

```bash
./build-all
./nano_studio output/nano/test.r01proj
```

Developer rebuild of this tree only:

```bash
cd nano/apps/studio
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
ctest --test-dir build --output-on-failure
./build/retr01_nano_studio
```

**Needs:** CMake, C compiler, SDL2, libpng, FreeType 2. Optional: X11 (clipboard PNG), `xclip` / `wl-clipboard`.

---

## Controls

| Action | Input |
|--------|--------|
| Play / pause | **Space** / **PLAY** |
| Move player | **WASD** / arrows |
| Warp test | **X** -> (0,0), **Y** -> (1,0) |
| Save / load | **Ctrl+S** / **Ctrl+O** |
| Export cart | **Ctrl+E** -> `output/nano/test.r01nano` |
| Toggle canvas | **Ctrl+Shift+R** |
| Present scale | **Ctrl+1** / **Ctrl+2** |

---

## Related docs

| Doc | Topic |
|-----|--------|
| [`../../docs/graphics.md`](../../docs/graphics.md) | Worlds, screens, attr, entities |
| [`../../docs/movement.md`](../../docs/movement.md) | Tile-enter Host Play move |
| [`../../docs/cart_format.md`](../../docs/cart_format.md) | `.r01proj` + `.r01nano` |
| [`../emu/README.md`](../emu/README.md) | Standalone Host Play |
| [`../sim/README.md`](../sim/README.md) | Board IC / netlist sim |
