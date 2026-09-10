"""Phase 1 lab BOM: ATmega + crystal + RGBS + power + ISP only (no cart)."""

from __future__ import annotations

from typing import List

from .bom import BomEntry, BoardId, _BARREL, _C_CER, _C_ELEC, _DIP40, _HDR2X2, _HDR2X3, _HDR2X4
from .bom import _R_AX, _SCHOTTKY, _XTAL

# Re-export footprints used by phase1_board passives
_C0603 = _C_CER
_R0603 = _R_AX

PHASE1_IC_COUNT = 1

# Electrical design for the Phase 1 proto / first PCB slice.
# No cart, pads, audio, AD725, or composite.
PHASE1_BOM: List[BomEntry] = [
    BomEntry(
        "U1284",
        "ATmega1284P",
        "console MCU PDIP-40 (Phase 1 lab)",
        BoardId.MOBO,
        40,
        _DIP40,
        vcc_pin="10",
        gnd_pin="11",
    ),
    BomEntry(
        "J_BARREL",
        "BARREL_5V",
        "CUI PJ-063AH 5V inlet (or screw-adapter jumpers to rails)",
        BoardId.MOBO,
        3,
        _BARREL,
        in_ic_count=False,
    ),
    BomEntry("J_AV", "J_AV", "RGBS 2x4 (AUD unused in Phase 1)", BoardId.MOBO, 8, _HDR2X4, in_ic_count=False),
    BomEntry("J_PWR", "J_PWR", "bench +5V/GND/RESET 2x2", BoardId.MOBO, 4, _HDR2X2, in_ic_count=False),
    BomEntry("J_ISP", "J_ISP", "AVR ISP 2x3 (USBasp)", BoardId.MOBO, 6, _HDR2X3, in_ic_count=False),
    BomEntry("Y1", "XTAL_20M", "20 MHz crystal", BoardId.MOBO, 2, _XTAL, in_ic_count=False),
    BomEntry("D1", "SCHOTTKY", "barrel reverse-polarity", BoardId.MOBO, 2, _SCHOTTKY, in_ic_count=False),
    BomEntry("Cbulk", "C_BULK", "barrel bulk ~220uF", BoardId.MOBO, 2, _C_ELEC, in_ic_count=False),
]


def phase1_silicon_ics() -> List[BomEntry]:
    return [e for e in PHASE1_BOM if e.in_ic_count]


def phase1_bom_entries() -> List[BomEntry]:
    return list(PHASE1_BOM)
