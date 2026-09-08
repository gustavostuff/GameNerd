"""
Physical pin numbers for Retr01 Nano parts.

Authority for stock parts: KiCad 10 symbols under /usr/share/kicad/symbols/
(see kicad_pin_extract.json). ATmega1284P-A extends ATmega164A-A (TQFP-44).
SST25VF010A uses SST25VF080B SOIC-8 pin twin. 24C64 uses 24LC64 / 24LC16.

Manifest connections address pins by these number strings.
PORT / net SoT: nano/docs/pinmap.md (wire by PORT, not DIP number).
"""

from __future__ import annotations

from typing import Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# ATmega1284P TQFP-44 (KiCad ATmega1284P-A / ATmega164A-A)
# ---------------------------------------------------------------------------

# Power / clock / reset
M1284_RESET = "4"
M1284_VCC = ("5", "17", "38")
M1284_GND = ("6", "18", "28", "39")
M1284_AVCC = "27"
M1284_AREF = "29"
M1284_XTAL2, M1284_XTAL1 = "7", "8"

# PORTB — SPI + FG + cart detect (nano/docs/pinmap.md)
M1284_PB0, M1284_PB1, M1284_PB2 = "40", "41", "42"  # FG0..FG2
M1284_PB3 = "43"  # CART_DET#
M1284_PB4 = "44"  # SPI_SS#
M1284_PB5, M1284_PB6, M1284_PB7 = "1", "2", "3"  # MOSI, MISO, SCK

# PORTC — I2C + P2 low
M1284_PC0, M1284_PC1 = "19", "20"  # SCL, SDA
M1284_PC2 = "21"
M1284_PC3 = "22"
M1284_PC4 = "23"
M1284_PC5 = "24"
M1284_PC6 = "25"
M1284_PC7 = "26"

# PORTD — sync, PWM, P2 high
M1284_PD0, M1284_PD1 = "9", "10"  # HSYNC, VSYNC
M1284_PD2, M1284_PD3 = "11", "12"  # P2_D6, P2_D7
M1284_PD4, M1284_PD5 = "13", "14"  # PWM_SFX, PWM_MUSIC
M1284_PD6, M1284_PD7 = "15", "16"  # unused bring-up (NC nets)

# PORTA — P1
M1284_PA0 = "37"
M1284_PA1 = "36"
M1284_PA2 = "35"
M1284_PA3 = "34"
M1284_PA4 = "33"
M1284_PA5 = "32"
M1284_PA6 = "31"
M1284_PA7 = "30"

M1284_P1 = (
    M1284_PA0,
    M1284_PA1,
    M1284_PA2,
    M1284_PA3,
    M1284_PA4,
    M1284_PA5,
    M1284_PA6,
    M1284_PA7,
)  # D0..D7
M1284_P2 = (
    M1284_PC2,
    M1284_PC3,
    M1284_PC4,
    M1284_PC5,
    M1284_PC6,
    M1284_PC7,
    M1284_PD2,
    M1284_PD3,
)  # D0..D7
M1284_FG = (M1284_PB0, M1284_PB1, M1284_PB2)

# ---------------------------------------------------------------------------
# SST25VF010A / SST25VF080B SOIC-8 twin
# ---------------------------------------------------------------------------

FLASH_CE, FLASH_SO, FLASH_WP, FLASH_VSS = "1", "2", "3", "4"
FLASH_SI, FLASH_SCK, FLASH_HOLD, FLASH_VDD = "5", "6", "7", "8"

# ---------------------------------------------------------------------------
# 24C64 / 24LC64
# ---------------------------------------------------------------------------

EE_A0, EE_A1, EE_A2 = "1", "2", "3"
EE_GND = "4"
EE_SDA, EE_SCL = "5", "6"
EE_WP = "7"
EE_VCC = "8"

# ---------------------------------------------------------------------------
# Passives / connectors
# ---------------------------------------------------------------------------

R1, R2 = "1", "2"
C1, C2 = "1", "2"

# CUI PJ-063AH / KiCad Barrel_Jack_MountingPin
BARREL_TIP, BARREL_SLEEVE, BARREL_MP = "1", "2", "MP"

# Cart edge 16 — A1..A8 = 1..8, B1..B8 = 9..16
def cart_a(n: int) -> str:
    return str(n)


def cart_b(n: int) -> str:
    return str(8 + n)


# AVR ISP 2x3 (Atmel / USBasp style, PinHeader_2x03)
# 1 MISO | 2 VCC
# 3 SCK  | 4 MOSI
# 5 RST  | 6 GND
ISP_MISO, ISP_VCC = "1", "2"
ISP_SCK, ISP_MOSI = "3", "4"
ISP_RST, ISP_GND = "5", "6"


def _nums(n: int) -> List[str]:
    return [str(i) for i in range(1, n + 1)]


PIN_TEMPLATES: Dict[str, List[str]] = {
    "ATmega1284P": _nums(44),
    "SST25VF010A": _nums(8),
    "24C64": _nums(8),
    "CART_EDGE_16": _nums(16),
    "CART_SOCKET_16": _nums(16),
    "BARREL_5V": ["1", "2", "MP"],
    "J_AV": _nums(8),
    "J_P1": _nums(10),
    "J_P2": _nums(10),
    "J_PWR": _nums(4),
    "J_ISP": _nums(6),
    "XTAL_20M": _nums(2),
    "SCHOTTKY": _nums(2),
    "C_BULK": _nums(2),
    "C_10U": _nums(2),
    "C_100N": _nums(2),
    "C_22P": _nums(2),
    "C_10U_AUD": _nums(2),
    "R_0": _nums(2),
    "R_75": _nums(2),
    "R_220": _nums(2),
    "R_470": _nums(2),
    "R_1K": _nums(2),
    "R_4K7": _nums(2),
    "R_10K": _nums(2),
}

# Pad number -> KiCad signal alias (readability in ERC / netlist)
KICAD_ALIASES: Dict[str, Dict[str, str]] = {
    "ATmega1284P": {
        "1": "PB5",
        "2": "PB6",
        "3": "PB7",
        "4": "~{RESET}",
        "5": "VCC",
        "6": "GND",
        "7": "XTAL2",
        "8": "XTAL1",
        "9": "PD0",
        "10": "PD1",
        "11": "PD2",
        "12": "PD3",
        "13": "PD4",
        "14": "PD5",
        "15": "PD6",
        "16": "PD7",
        "17": "VCC",
        "18": "GND",
        "19": "PC0",
        "20": "PC1",
        "21": "PC2",
        "22": "PC3",
        "23": "PC4",
        "24": "PC5",
        "25": "PC6",
        "26": "PC7",
        "27": "AVCC",
        "28": "GND",
        "29": "AREF",
        "30": "PA7",
        "31": "PA6",
        "32": "PA5",
        "33": "PA4",
        "34": "PA3",
        "35": "PA2",
        "36": "PA1",
        "37": "PA0",
        "38": "VCC",
        "39": "GND",
        "40": "PB0",
        "41": "PB1",
        "42": "PB2",
        "43": "PB3",
        "44": "PB4",
    },
    "SST25VF010A": {
        "1": "~{CE}",
        "2": "SO",
        "3": "~{WP}",
        "4": "VSS",
        "5": "SI",
        "6": "SCK",
        "7": "~{HOLD}",
        "8": "VDD",
    },
    "24C64": {
        "1": "A0",
        "2": "A1",
        "3": "A2",
        "4": "GND",
        "5": "SDA",
        "6": "SCL",
        "7": "WP",
        "8": "VCC",
    },
}


def power_pin_nums(mpn: str) -> Optional[Tuple[str, str]]:
    """Primary VCC/GND pad numbers for Quilter local-bypass helpers."""
    table = {
        "ATmega1284P": ("5", "6"),
        "SST25VF010A": (FLASH_VDD, FLASH_VSS),
        "24C64": (EE_VCC, EE_GND),
    }
    return table.get(mpn)
