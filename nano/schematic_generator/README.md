# Nano schematic generator (SKiDL)

Python package that turns Retr01 Nano motherboard + cart wiring into KiCad netlists.
Mirrors `app/schematic_generator/` (parent Retr01): pin numbers from official KiCad
symbols, inline SKiDL parts, separate mobo / cart netlists.

## Code-to-copper

1. Docs SoT: `nano/docs/pinmap.md`, `nano/docs/hardware.md`
2. `python generate.py --check`
3. `python generate.py` -> `output/nano_mobo.net` + `output/nano_cart.net`
4. Import each netlist into its KiCad project. Outline starts at 100x100 mm (provisional).
5. Route (Quilter optional). Bypass uses per-IC local `+5V_<refdes>` + 0R bridges.

## KiCad symbol mapping

Same pattern as parent `retr01_schem/pinmap.py` / `kicad_pin_extract.json`:

| Nano MPN | KiCad lib | KiCad symbol | Notes |
|----------|-----------|--------------|-------|
| ATmega1284P | MCU_Microchip_ATmega | **ATmega1284P-P** | PDIP-40 (mobo THT) |
| 25LC1024 | (inline / datasheet) | JEDEC SPI EEPROM | **PDIP-8** cart game image |
| 24C64 | Memory_EEPROM | **24LC64** (extends 24LC16) | **PDIP-8** |
| Passives | Device | R, C | |
| Barrel | Connector | Barrel_Jack_MountingPin | Footprint CUI PJ-063AH |
| Headers | Connector_Generic | Conn_01xN / Conn_02x03 | |

SKiDL still builds `Part(tool=SKIDL)` with **physical pad numbers**. Official KiCad
signal names are aliases (`PB5`, `~{CE}`, …).

## Quick start

```bash
cd nano/schematic_generator
# reuse parent venv, or:
python -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

python generate.py --check
python generate.py
```

## Layout

```
nano/schematic_generator/
+-- generate.py
+-- requirements.txt
+-- nano_schem/
|   +-- bom.py
|   +-- pinmap.py
|   +-- kicad_pin_extract.json
|   +-- parts.py
|   +-- connect.py
|   +-- manifest.py
|   +-- cart_manifest.py
|   +-- board.py
+-- library/          # symlink to hw/kicad/Retr01_Lib.pretty
+-- output/
```

Cart gold fingers: `Retr01_Lib:Cart_Edge_2x8_P2.54mm` (same pad recipe as 2x18, 8/side).
Mobo cart slot: PinSocket 2x8 stand-in until EDAC CAD lands.
