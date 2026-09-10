# Nano schematic generator (SKiDL)

Python package that turns Retr01 Nano motherboard + cart wiring into KiCad netlists.
Mirrors `app/schematic_generator/` (parent Retr01): pin numbers from official KiCad
symbols, inline SKiDL parts, separate mobo / cart netlists.

## Code-to-copper

1. Docs SoT: `nano/docs/pinmap.md`, `nano/docs/hardware.md`
2. `python generate.py --check`
3. `python generate.py` -> `output/nano_mobo.net` + `output/nano_cart.net`
4. Import each netlist into its KiCad project (see [Netlist import](#netlist-import) below). Outline starts at 100x100 mm (provisional).
5. Route (Quilter optional). Bypass uses per-IC local `+5V_<refdes>` + 0R bridges.

## Netlist import

Nano and full Retr01 share **one** footprint library: [`hw/kicad/Retr01_Lib.pretty`](../../hw/kicad/Retr01_Lib.pretty). The netlist only carries a **name** (e.g. `Retr01_Lib:Jack_3.5mm_Switchcraft_35RAPC2BVN4_Vertical`); KiCad loads the actual `.kicad_mod` from the project **`fp-lib-table`**.

Each Nano KiCad project includes `fp-lib-table` pointing at that folder. After editing a footprint in `Retr01_Lib.pretty`, re-import or **Update Footprints from Library** so pcbnew picks up the change (placed footprints are copies until refreshed).

**Mobo** (`nano/nano_main_pcb/nano_main_pcb_v_01/`):

1. Open `nano_main_pcb_v_01.kicad_pro`.
2. Confirm **Preferences -> Manage Footprint Libraries** shows **Retr01_Lib** (no broken path).
3. **File -> Import -> Netlist** -> `nano/schematic_generator/output/nano_mobo.net`.
4. Enable **Exchange footprint** (or delete old J3/J4/J8/J9 before re-import if footprints were stale).
5. Place / route. Custom parts from netlist: **J3/J4** TRS, **J8/J9** RCA, **J_CART** uses stock PinSocket 2x8 (not Retr01_Lib).

**Cart** (`nano/nano_cart_pcb/Nano_Cart_PCB_v_01/`):

1. Same `fp-lib-table` -> **Retr01_Lib**.
2. Import `output/nano_cart.net`. **J16** -> `Retr01_Lib:Cart_Edge_2x8_P2.54mm`.

## KiCad symbol mapping

Same pattern as parent `retr01_schem/pinmap.py` / `kicad_pin_extract.json`:

| Nano MPN | KiCad lib | KiCad symbol | Notes |
|----------|-----------|--------------|-------|
| ATmega1284P | MCU_Microchip_ATmega | **ATmega1284P-P** | PDIP-40 (mobo THT) |
| AD725 | (inline) | AD725ARZ wide SOIC-16 | `Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm` |
| 25LC1024 | (inline / datasheet) | JEDEC SPI EEPROM | **PDIP-8** cart game image |
| 24C64 | Memory_EEPROM | **24LC64** (extends 24LC16) | **PDIP-8** |
| Passives | Device | R, C | |
| Barrel | Connector | Barrel_Jack_MountingPin | Footprint CUI PJ-063AH |
| Headers | Connector_Generic | Conn_02xN | 2x2 PWR, 2x3 ISP, 2x4 AV, 2x10 arcade |
| TRS / RCA | Retr01_Lib | 35RAPC2BVN4 / RCJ-01x | Same footprints as full Retr01 |

SKiDL still builds `Part(tool=SKIDL)` with **physical pad numbers**. Official KiCad
signal names are aliases (`PB5`, `~{CE}`, ...).

## Quick start

```bash
cd nano/schematic_generator
# reuse parent venv, or:
python -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

# Phase 1 lab (MCU + crystal + RGBS + ISP, no cart):
python generate_phase1.py --check
python generate_phase1.py
# -> output/nano_phase1.net

# Full mobo + cart (later phases / final form):
python generate.py --check
python generate.py
# -> output/nano_mobo.net + output/nano_cart.net
```

## Layout

```
nano/schematic_generator/
+-- generate.py              # full mobo + cart
+-- generate_phase1.py       # Phase 1 RGBS lab (no cart)
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
|   +-- phase1_bom.py
|   +-- phase1_manifest.py
|   +-- phase1_board.py
+-- library/          # symlink to hw/kicad/Retr01_Lib.pretty
+-- output/
```

Cart gold fingers: `Retr01_Lib:Cart_Edge_2x8_P2.54mm` (same pad recipe as 2x18, 8/side).
Mobo cart slot: PinSocket 2x8 stand-in until EDAC CAD lands.
