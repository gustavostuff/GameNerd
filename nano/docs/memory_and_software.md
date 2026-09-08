# Memory and software

**Status: design.** Budgets are planning numbers for ATmega1284P.

## Chip resources

| Resource | Size | Planned use |
|----------|------|-------------|
| Flash | 128 KB | Game code, CHR tiles, maps, music tables |
| Internal SRAM | 16 KB | Line buffers, nametable, vars, stack |
| EEPROM | 4 KB | Saves, machine config |

## SRAM sketch

| Block | Size | Notes |
|-------|------|-------|
| Double line buffer | 32 bytes | 16 + 16 at 1 bpp |
| Nametable (ex. 16x12) | ~192 bytes | Fits 128x96 with 8x8 tiles |
| Pad mirror | 2 bytes | P1 + P2 |
| Game heap / BSS / stack | remainder | Keep stack headroom for IRQ / kernel |

Flash layout (conceptual):

```text
[ reset / vectors ]
[ video kernel (asm) ]
[ C runtime + SDK ]
[ game code ]
[ CHR banks ]
[ map / screen data ]
[ tone tables ]
```

No external cart image. Flashing the MCU **is** installing the game.

## Input software contract

Match the **spirit** of full Retr01 pad ports (not the cart `$FExx` bus):

| Player | Byte | Bit meaning (1 = pressed) |
|--------|------|---------------------------|
| P1 | pad0 | bit0 Right, bit1 Left, bit2 Down, bit3 Up, bit4 X, bit5 Y, bit6 Coin/Select, bit7 Start |
| P2 | pad1 | same layout |

Hardware is arcade GPIO into MCU pins (see [`hardware.md`](hardware.md)). Firmware packs pins into these bytes each VBlank (or each frame).

## Audio software

- One hardware **PWM** channel
- Square / simple tones
- Frequency / duty updated in **VBlank** so the active-line kernel stays undisturbed

Music data can be compact step tables in Flash (Nano is not the full Retr01 APU tracker).

## C SDK direction (TBD)

Exact host toolchain is open. Product direction:

- **C SDK** for game authors
- Thin HAL: `nano_init`, vblank hook, nametable poke, scroll/screen API, `nano_pad_read`, `nano_pwm_set`
- Assembly video kernel linked as a fixed object
- Make-based **avr-gcc** is the likely first host path (not locked)

Authoring beyond hand-written C (Studio Nano profile, tile editors) is **TBD**.

## Performance outlook

With 1 bpp, 8x8 tiles, no sprites, and double line buffers:

- Stable 60 Hz at 128x96 should be **comfortable** at 20 MHz
- Meaningful CPU time remains in VBlank for game logic
- Pushing vertical resolution costs kernel cycles first, game time second

## Validation game

After the kernel + nametable + pads + PWM path exist, ship a tiny loop (move a "player" tile, switch screens, beep) before any larger title.
