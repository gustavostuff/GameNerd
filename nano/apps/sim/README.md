# Retr01 Nano Simulator

**Board IC / netlist sim** for Retr01 Nano — not a second emulator window.

| vs | Nano Sim |
|----|----------|
| **Full Sim** | Same idea (entities, pins, islands, wire settle, SDL board UI). Nano is a **tiny** netlist — no 6502 / PLD / OAM / parallel SST39. |
| **Nano Emu** | Picture + Host Play only. Sim **owns** the motherboard model; emu core is linked inside the behavioral 1284 firmware services (compose / play / `custom_logic`). |
| **AVR ISA** | **Not yet** — 1284 stands in for open console firmware (VBlank compose, SPI MAP refill, pads). |

## Board (v1)

| Island | Parts |
|--------|--------|
| **VIDEO** | `SCREEN_SINK` 256×192 RGBS |
| **MCU** | `PWR5V`, `OSC8M`, `ATMEGA1284P` (Nano role), `PADS`, `PWM2CH` |
| **CART** | `SST25VF010A` SPI flash + `24C64` I2C EEPROM |

Wiring (explicit settle, not a SPICE net): power rail → chips; PHI2 → MCU CLK; SPI SS#/SCK/MOSI/MISO; I2C SDA/SCL; VSYNC → sink.

Cart `.r01nano` loads into SST25. MCU boots via `r01ne_machine_boot_mem`. Each board step = one coarse VBlank (Host Play + soft compose + optional 384 B SPI MAP refill model).

## Build / run

```bash
./build-all
./nano_sim output/nano/test.r01nano
```

You should see **islands and DIP packages**, with the game picture on **SCR1** — not a fullscreen emu viewport.

**Controls:** SPACE pause · WASD + G pads · Esc quit · RMB/MMB pan · click/drag ICs · Ctrl+1/2/3 scale

## Layout

| Path | Role |
|------|------|
| `src/board.c` | Netlist recipe + `wire_*` + group step |
| `chips/atmega1284p_nano.c` | Nano console MCU entity |
| `chips/sst25vf010a.c` | SPI cart flash entity |
| `chips/video_sink.c` | 256×192 display entity |
| `src/ui.c` | Board canvas (islands / pins / SCR / arcade) |
