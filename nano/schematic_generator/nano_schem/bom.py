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


# Footprints - motherboard + cart silicon THT. AD725ARZ is the only SMD IC (wide SOIC-16).
_DIP40 = "Package_DIP:DIP-40_W15.24mm"
_SOIC16W = "Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm"
_DIP8 = "Package_DIP:DIP-8_W7.62mm"
_EDGE16_MOBO = "Connector_PinSocket_2.54mm:PinSocket_2x08_P2.54mm_Vertical"
_EDGE16_CART = "Retr01_Lib:Cart_Edge_2x8_P2.54mm"
_BARREL = "Connector_BarrelJack:BarrelJack_CUI_PJ-063AH_Horizontal"
_HDR2X2 = "Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical"
_HDR2X3 = "Connector_PinHeader_2.54mm:PinHeader_2x03_P2.54mm_Vertical"
_HDR2X4 = "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical"
_HDR2X10 = "Connector_PinHeader_2.54mm:PinHeader_2x10_P2.54mm_Vertical"
_XTAL = "Crystal:Crystal_HC49-U_Vertical"
_OSC8 = "Oscillator:Oscillator_DIP-8"
_TRS = "Retr01_Lib:Jack_3.5mm_Switchcraft_35RAPC2BVN4_Vertical"
_RCA = "Retr01_Lib:CUI_RCJ-01x_Vertical"
_C_CER = "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm"
_C_ELEC = "Capacitor_THT:CP_Radial_D8.0mm_P3.50mm"
_R_AX = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"
_SCHOTTKY = "Diode_THT:D_DO-41_SOD81_P2.54mm_Vertical_CathodeUp"
_L_AX = "Inductor_THT:L_Axial_L11.0mm_D4.5mm_P5.08mm_Vertical_Fastron_MECC"

# board.py decoupling helpers import these names
_C0603 = _C_CER
_R0603 = _R_AX

MOBO_IC_COUNT = 1  # ATmega1284P only; AD725 is AV support (in_ic_count=False)
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
    # AD725ARZ = Analog Devices RW-16 wide SOIC (only SMD IC on Nano mobo)
    BomEntry(
        "U725",
        "AD725",
        "AD725ARZ RGB->NTSC wide SOIC-16 (direct SMD)",
        BoardId.MOBO,
        16,
        _SOIC16W,
        in_ic_count=False,
    ),
    BomEntry(
        "Y3",
        "OSC_4FSC",
        "Abracon ACH-14.31818MHZ-EK (AD725 4FSC)",
        BoardId.MOBO,
        8,
        _OSC8,
        in_ic_count=False,
        vcc_pin="8",
        gnd_pin="4",
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
    BomEntry("J_AV", "J_AV", "RGBS+AUD 2x4 bring-up", BoardId.MOBO, 8, _HDR2X4, in_ic_count=False),
    BomEntry("J_PAD", "J_PAD", "arcade 2P 2x10", BoardId.MOBO, 20, _HDR2X10, in_ic_count=False),
    BomEntry("J_PWR", "J_PWR", "bench +5V/GND/RESET 2x2", BoardId.MOBO, 4, _HDR2X2, in_ic_count=False),
    BomEntry("J_ISP", "J_ISP", "AVR ISP 2x3", BoardId.MOBO, 6, _HDR2X3, in_ic_count=False),
    BomEntry(
        "J3",
        "TRS_P1",
        "Switchcraft 35RAPC2BVN4 P1 (vertical)",
        BoardId.MOBO,
        5,
        _TRS,
        in_ic_count=False,
    ),
    BomEntry(
        "J4",
        "TRS_P2",
        "Switchcraft 35RAPC2BVN4 P2 (vertical)",
        BoardId.MOBO,
        5,
        _TRS,
        in_ic_count=False,
    ),
    BomEntry(
        "J8",
        "AUDIO_OUT",
        "CUI RCJ-012 black RCA (mono audio)",
        BoardId.MOBO,
        2,
        _RCA,
        in_ic_count=False,
    ),
    BomEntry(
        "J9",
        "COMPOSITE_OUT",
        "CUI RCJ-014 yellow RCA (composite)",
        BoardId.MOBO,
        2,
        _RCA,
        in_ic_count=False,
    ),
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
    # Analog video spur + AD725 tank / couple / term (also instantiated if listed here)
    BomEntry("FB2", "FERRITE", "analog video ferrite", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("Cva", "C_10U", "10uF analog spur", BoardId.MOBO, 2, _C_ELEC, in_ic_count=False),
    BomEntry("CinR", "C_100N", "AD725 RIN AC couple", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("CinG", "C_100N", "AD725 GIN AC couple", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("CinB", "C_100N", "AD725 BIN AC couple", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("R75C", "R_75", "composite series 75R", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("Lytrap", "L_YTRAP", "AD725 YTRAP ~68uH NTSC", BoardId.MOBO, 2, _L_AX, in_ic_count=False),
    BomEntry("Cytrap", "C_100N", "AD725 YTRAP resonate C", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("Cd725a", "C_100N", "AD725 APOS decouple", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("Cd725d", "C_100N", "AD725 DPOS decouple", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    # TRS aux pads
    BomEntry("F2", "PPTC", "TRS P1 VCC PPTC", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("F3", "PPTC", "TRS P2 VCC PPTC", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("Cpad1", "C_100N", "TRS P1 VCC after PPTC", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("Cpad2", "C_100N", "TRS P2 VCC after PPTC", BoardId.MOBO, 2, _C_CER, in_ic_count=False),
    BomEntry("Rdata1", "R_47", "TRS P1 DATA series", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("Rdata2", "R_47", "TRS P2 DATA series", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
    BomEntry("Rpu1", "R_4K7", "PAD_DATA pull-up", BoardId.MOBO, 2, _R_AX, in_ic_count=False),
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
        "25LC1024",
        "1 Mbit SPI EEPROM PDIP-8 (game image)",
        BoardId.CART,
        8,
        _DIP8,
        vcc_pin="8",
        gnd_pin="4",
    ),
    BomEntry(
        "U50",
        "24C64",
        "64 Kbit I2C EEPROM PDIP-8",
        BoardId.CART,
        8,
        _DIP8,
        vcc_pin="8",
        gnd_pin="4",
    ),
]


def entries_for_board(board: BoardId) -> List[BomEntry]:
    return [e for e in BOM if e.board == board]


def silicon_ic_entries(board: Optional[BoardId] = None) -> List[BomEntry]:
    rows = BOM if board is None else entries_for_board(board)
    return [e for e in rows if e.in_ic_count]
