# Retr01 Nano Simulator

**Board IC / netlist sim** for Retr01 Nano. Foundation for SKiDL -> Quilter, not a second fullscreen emulator window.

## Is ~60 FPS "because Nano is simple?"

**Yes, now, with one caveat.**

After fixing multi-field catchup, each UI frame advances **exactly one RGBS field** (~232 scanline steps). Scanline work is cheap on this tiny netlist, so the host finishes a field within one vsync easily. Full Retr01 Sim stays ~1 FPS because each step still does heavy beam/VRAM settle across ~23 ICs.

| Still soft (OK for now) | Pin-accurate / board-true |
|-------------------------|---------------------------|
| Soft compose via emu core (= FW SRAM compose) | Nets in [`nano/docs/pinmap.md`](../../docs/pinmap.md) |
| No AVR instruction ISA | SPI/I2C/PWM/XTAL/pad/sync on real PORT names |
| No pixel-timed FG GPIO DAC shift | 25LC1024 + 24C64 cart entities, WP#/HOLD# tied |
| I2C protocol stub | 1 step = 1 RGBS line + VBlank SPI byte clocks |
| No host BGM/SFX (Studio Play has that overlay) | Pads / move via behavioral 1284 + emu core |

## Board

| Island | Parts |
|--------|--------|
| **VIDEO** | `SCREEN_SINK` 256x192 |
| **MCU** | `PWR5V`, crystal stand-in (`OSC8M` entity @ 20 MHz timing), `ATMEGA1284P` PDIP-40, `PADS` P1+P2, `PWM2CH` |
| **CART** | `25LC1024` + `24C64` |

MCU package on the PCB is **PDIP-40**. Sim draws the same PORT names (wire by net, not DIP number).

**SCR scaling:** the sink glyph is **128x96**. The **256x192** RGBS buffer is drawn with an **integer** fit (exact 2:1 downscale). That avoids uneven nearest-neighbor pixels from board snap rounding.

## Build / run

```bash
./build-all
./nano_sim output/nano/test.r01nano
```

**Controls:** SPACE pause. WASD+G = P1. arrows+,.= P2. Esc. RMB pan. Ctrl+1/2/3 scale.
