# Nano apps

Host-side tools for Retr01 Nano.

| App | Path | Notes |
|-----|------|-------|
| **Nano Studio** | [`studio/`](studio/) | Independent for now (started as a Studio fork). Built by `./build-all` as `bin/nano_studio` (`./nano_studio`). Trimmed toward Nano constraints (graphics, cart, **2 audio channels**, 128x96 logical / 2x preview). |
| **Nano Emu** | [`emu/`](emu/) | `.r01nano` picture + Host Play (WASD, Walk state, solid MAP). `./nano_emu`. No audio yet. |
| **Nano Sim** | (root `./nano_sim`) | Placeholder. Independent of full `./sim` for now. |

**Independence now, shared code later:** Nano Studio / Emu / Sim do not need to stay wire-compatible with the full apps while Nano is designed. A later pass should extract shared modules so Nano and full tools stop duplicating forever.

Build and run details live in each app README. Nano console firmware is not here yet (design docs under [`../docs/`](../docs/)).
