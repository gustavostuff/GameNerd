# Nano apps

Host-side tools for Retr01 Nano.

| App | Path | Notes |
|-----|------|-------|
| **Nano Studio** | [`studio/`](studio/) | Independent for now (started as a Studio fork). Built by `./build-all` as `bin/nano_studio` (`./nano_studio`). Nano caps: graphics, `.r01nano` / 128 KB, **Music+SFX**, 128x96 logical / 2x preview. Host Play via embedded emu. Host BGM/SFX overlay on Play. |
| **Nano Emu** | [`emu/`](emu/) | `.r01nano` picture + Host Play (WASD, Walk state, solid MAP). `./nano_emu`. No audio yet (lags Studio Play). |
| **Nano Sim** | [`sim/`](sim/) | IC/netlist board (islands, pins, wire settle). Nano parts only + SCR on canvas. Links emu core inside behavioral 1284. `./nano_sim`. Not a fullscreen emu. No host audio. |

**Independence now, shared code later:** Nano Studio / Emu / Sim do not need to stay wire-compatible with the full apps while Nano is designed. A later pass should extract shared modules so Nano and full tools stop duplicating forever.

Build and run details live in each app README. Nano console firmware is not here yet (design docs under [`../docs/`](../docs/)).
