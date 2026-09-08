"""
Physical pin numbers for Retr01 Nano parts.

Authority for stock parts: KiCad 10 symbols under /usr/share/kicad/symbols/
(see kicad_pin_extract.json). ATmega1284P-P is PDIP-40 (motherboard is fully THT).
SST25VF010A uses SST25VF080B SOIC-8 pin twin (cart). 24C64 uses 24LC64 / 24LC16.

Manifest connections address pins by these number strings.
PORT / net SoT: nano/docs/pinmap.md (wire by PORT name; DIP numbers match KiCad).
"""

from __future__ import annotations

from typing import Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# ATmega1284P PDIP-40 (KiCad ATmega1284P-P) — motherboard THT
# ---------------------------------------------------------------------------

# Power / clock / reset
M1284_RESET = "9"
M1284_VCC = ("10",)  # single VCC on DIP-40
M1284_GND = ("11", "31")
M1284_AVCC = "30"
M1284_AREF = "32"
M1284_XTAL2, M1284_XTAL1 = "12", "13"

# PORTB — SPI + FG + cart detect (nano/docs/pinmap.md)
M1284_PB0, M1284_PB1, M1284_PB2 = "1", "2", "3"  # FG0..FG2
M1284_PB3 = "4"  # CART_DET#
M1284_PB4 = "5"  # SPI_SS#
M1284_PB5, M1284_PB6, M1284_PB7 = "6", "7", "8"  # MOSI, MISO, SCK

# PORTC — I2C + P2 low
M1284_PC0, M1284_PC1 = "22", "23"  # SCL, SDA
M1284_PC2 = "24"
M1284_PC3 = "25"
M1284_PC4 = "26"
M1284_PC5 = "27"
M1284_PC6 = "28"
M1284_PC7 = "29"

# PORTD — sync, PWM, P2 high
M1284_PD0, M1284_PD1 = "14", "15"  # HSYNC, VSYNC
M1284_PD2, M1284_PD3 = "16", "17"  # P2_D6, P2_D7
M1284_PD4, M1284_PD5 = "18", "19"  # PWM_SFX, PWM_MUSIC
M1284_PD6, M1284_PD7 = "20", "21"  # unused bring-up (NC nets)

# PORTA — P1
M1284_PA0 = "40"
M1284_PA1 = "39"
M1284_PA2 = "38"
M1284_PA3 = "37"
M1284_PA4 = "36"
M1284_PA5 = "35"
M1284_PA6 = "34"
M1284_PA7 = "33"

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
# SST25VF010A / SST25VF080B SOIC-8 twin (cart — same pin numbers as DIP-8)
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
    "ATmega1284P": _nums(40),
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

# Pad number -> KiCad signal alias (ATmega1284P-P)
KICAD_ALIASES: Dict[str, Dict[str, str]] = {
    "ATmega1284P": {
        "1": "PB0",
        "2": "PB1",
        "3": "PB2",
        "4": "PB3",
        "5": "PB4",
        "6": "PB5",
        "7": "PB6",
        "8": "PB7",
        "9": "~{RESET}",
        "10": "VCC",
        "11": "GND",
        "12": "XTAL2",
        "13": "XTAL1",
        "14": "PD0",
        "15": "PD1",
        "16": "PD2",
        "17": "PD3",
        "18": "PD4",
        "19": "PD5",
        "20": "PD6",
        "21": "PD7",
        "22": "PC0",
        "23": "PC1",
        "24": "PC2",
        "25": "PC3",
        "26": "PC4",
        "27": "PC5",
        "28": "PC6",
        "29": "PC7",
        "30": "AVCC",
        "31": "GND",
        "32": "AREF",
        "33": "PA7",
        "34": "PA6",
        "35": "PA5",
        "36": "PA4",
        "37": "PA3",
        "38": "PA2",
        "39": "PA1",
        "40": "PA0",
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
        "ATmega1284P": ("10", "11"),
        "SST25VF010A": (FLASH_VDD, FLASH_VSS),
        "24C64": (EE_VCC, EE_GND),
    }
    return table.get(mpn)
