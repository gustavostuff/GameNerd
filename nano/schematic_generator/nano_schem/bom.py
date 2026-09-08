"""Nano motherboard + cart BOM entries."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import List, Optional


class BoardId(Enum):
    MOBO = "mobo"
    CART = "cart"


@dataclass(frozen=True)
class BomEntry:
    refdes: str
    mpn: str
    description: str
    board: BoardId
    dip_pins: int
    footprint: str
    in_ic_count: bool = True
    vcc_pin: Optional[str] = None
    gnd_pin: Optional[str] = None


# Footprints — motherboard is fully THT. Cart silicon is SOIC-8 (cute cart density).
_DIP40 = "Package_DIP:DIP-40_W15.24mm"
_SOIC8 = "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm"
_EDGE16_MOBO = "Connector_PinSocket_2.54mm:PinSocket_2x08_P2.54mm_Vertical"
_EDGE16_CART = "Retr01_Lib:Cart_Edge_2x8_P2.54mm"
_BARREL = "Connector_BarrelJack:BarrelJack_CUI_PJ-063AH_Horizontal"
_HDR8 = "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical"
_HDR10 = "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical"
_HDR4 = "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical"
_HDR2X3 = "Connector_PinHeader_2.54mm:PinHeader_2x03_P2.54mm_Vertical"
_XTAL = "Crystal:Crystal_HC49-U_Vertical"
_C_CER = "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm"
_C_ELEC = "Capacitor_THT:CP_Radial_D8.0mm_P3.50mm"
_R_AX = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"
_SCHOTTKY = "Diode_THT:D_DO-41_SOD81_P2.54mm_Vertical_CathodeUp"

# board.py decoupling helpers import these names
_C0603 = _C_CER
_R0603 = _R_AX

MOBO_IC_COUNT = 1
CART_IC_COUNT = 2

BOM: List[BomEntry] = [
    # --- motherboard silicon ---
    BomEntry(
        "U1284",
        "ATmega1284P",
        "console MCU PDIP-40 (THT)",
        BoardId.MOBO,
        40,
        _DIP40,
        vcc_pin="10",
        gnd_pin="11",
    ),
    # --- motherboard connectors ---
    BomEntry(
        "J_BARREL",
        "BARREL_5V",
        "CUI PJ-063AH 5V inlet",
        BoardId.MOBO,
        3,
        _BARREL,
        in_ic_count=False,
    ),
    BomEntry(
        "J_CART",
        "CART_SOCKET_16",
        "2x8 cart edge female stand-in",
        BoardId.MOBO,
        16,
        _EDGE16_MOBO,
        in_ic_count=False,
    ),
    BomEntry("J_AV", "J_AV", "RGBS+AUD 1x8", BoardId.MOBO, 8, _HDR8, in_ic_count=False),
    BomEntry("J_P1", "J_P1", "arcade P1 1x10", BoardId.MOBO, 10, _HDR10, in_ic_count=False),
    BomEntry("J_P2", "J_P2", "arcade P2 1x10", BoardId.MOBO, 10, _HDR10, in_ic_count=False),
    BomEntry("J_PWR", "J_PWR", "bench +5V/GND/RESET 1x4", BoardId.MOBO, 4, _HDR4, in_ic_count=False),
    BomEntry("J_ISP", "J_ISP", "AVR ISP 2x3", BoardId.MOBO, 6, _HDR2X3, in_ic_count=False),
    BomEntry(
        "Y1",
        "XTAL_20M",
        "20 MHz crystal",
        BoardId.MOBO,
        2,
        _XTAL,
        in_ic_count=False,
    ),
    BomEntry(
        "D1",
        "SCHOTTKY",
        "barrel reverse-polarity",
        BoardId.MOBO,
        2,
        _SCHOTTKY,
        in_ic_count=False,
    ),
    BomEntry(
        "Cbulk",
        "C_BULK",
        "barrel bulk ~220uF",
        BoardId.MOBO,
        2,
        _C_ELEC,
        in_ic_count=False,
    ),
    # --- cart ---
    BomEntry(
        "J16",
        "CART_EDGE_16",
        "cart gold fingers 2x8",
        BoardId.CART,
        16,
        _EDGE16_CART,
        in_ic_count=False,
    ),
    BomEntry(
        "U25",
        "SST25VF010A",
        "1 Mbit SPI flash",
        BoardId.CART,
        8,
        _SOIC8,
        vcc_pin="8",
        gnd_pin="4",
    ),
    BomEntry(
        "U50",
        "24C64",
        "64 Kbit I2C EEPROM",
        BoardId.CART,
        8,
        _SOIC8,
        vcc_pin="8",
        gnd_pin="4",
    ),
]


def entries_for_board(board: BoardId) -> List[BomEntry]:
    return [e for e in BOM if e.board == board]


def silicon_ic_entries(board: Optional[BoardId] = None) -> List[BomEntry]:
    rows = BOM if board is None else entries_for_board(board)
    return [e for e in rows if e.in_ic_count]
