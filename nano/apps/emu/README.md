# Retr01 Nano Emulator

Software picture emulator for **`.r01nano`** carts. Independent of full [`app/emu`](../../../app/emu/) for now (see [`nano/README.md`](../../README.md)).

**v1 scope:** load cart → world 0 CHR + spawn MAP → Host Play (pixel integrate / **tile draw**, Walk state, solid MAP) → stamp soft entities → compose **128×96** → present **256×192** (2×). No audio yet.

Studio **Ctrl+E** also writes `output/nano/C/` (`base_game.c`, `custom_logic.c`, headers) — same export contract as full Retr01. Host Play mirrors that API (Idle=0, Walk=1 from `custom_logic`).

## Build / run

From the repo root:

```bash
./build-all
./nano_emu output/nano/test.r01nano
```

Developer rebuild of this tree only:

```bash
cd nano/apps/emu
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/retr01_nano_emu ../../../output/nano/test.r01nano
```

Export a cart from Nano Studio first (`Ctrl+E` → `output/nano/test.r01nano`).

**Controls:** Esc / close window = quit. WASD = move. `R` = reset. `Ctrl+1/2/3` = window scale.

## Layout

| Path | Role |
|------|------|
| `include/retr01_nano_emu/types.h` | Nano cart / screen constants |
| `include/retr01_nano_emu/cart.h` | `.r01nano` parser |
| `include/retr01_nano_emu/video.h` | CHR + MAP compose + 2× FB |
| `include/retr01_nano_emu/machine.h` | Boot / frame |
| `src/main.c` | SDL host (picture only) |
| `tests/` | Cart + boot smoke tests |

Contract: [`nano/docs/cart_format.md`](../../docs/cart_format.md), [`nano/docs/graphics.md`](../../docs/graphics.md), [`nano/docs/video.md`](../../docs/video.md).
