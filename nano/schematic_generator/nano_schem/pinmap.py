"""
Physical pin numbers for Retr01 Nano parts.

Authority for stock parts: KiCad 10 symbols under /usr/share/kicad/symbols/
(see kicad_pin_extract.json). ATmega1284P-P is PDIP-40 (motherboard is fully THT).
Cart game memory is Microchip **25LC1024** PDIP-8 (JEDEC SPI pinout). 24C64 uses 24LC64 / 24LC16.

Manifest connections address pins by these number strings.
PORT / net SoT: nano/docs/pinmap.md (wire by PORT name; DIP numbers match KiCad).
"""

from __future__ import annotations

from typing import Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# ATmega1284P PDIP-40 (KiCad ATmega1284P-P) - motherboard THT
# ---------------------------------------------------------------------------

# Power / clock / reset
M1284_RESET = "9"
M1284_VCC = ("10",)  # single VCC on DIP-40
M1284_GND = ("11", "31")
M1284_AVCC = "30"
M1284_AREF = "32"
M1284_XTAL2, M1284_XTAL1 = "12", "13"

# PORTB - SPI + FG + cart detect (nano/docs/pinmap.md)
M1284_PB0, M1284_PB1, M1284_PB2 = "1", "2", "3"  # FG0..FG2
M1284_PB3 = "4"  # CART_DET#
M1284_PB4 = "5"  # SPI_SS#
M1284_PB5, M1284_PB6, M1284_PB7 = "6", "7", "8"  # MOSI, MISO, SCK

# PORTC - I2C + P2 low
M1284_PC0, M1284_PC1 = "22", "23"  # SCL, SDA
M1284_PC2 = "24"
M1284_PC3 = "25"
M1284_PC4 = "26"
M1284_PC5 = "27"
M1284_PC6 = "28"
M1284_PC7 = "29"

# PORTD - sync, PWM, pad UART, P2 high
M1284_PD0, M1284_PD1 = "14", "15"  # HSYNC, VSYNC
M1284_PD2 = "16"  # PAD_DATA (RXD1 / open-drain UART - same as full Retr01)
M1284_PD3 = "17"  # unused bring-up (NC / future)
M1284_PD4, M1284_PD5 = "18", "19"  # PWM_SFX, PWM_MUSIC
M1284_PD6, M1284_PD7 = "20", "21"  # P2_D6, P2_D7 (moved off PD2/PD3 for TRS UART)

# PORTA - P1
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
    M1284_PD6,
    M1284_PD7,
)  # D0..D7
M1284_FG = (M1284_PB0, M1284_PB1, M1284_PB2)
M1284_PAD_DATA = M1284_PD2  # Tip/Ring bus on J3/J4

# ---------------------------------------------------------------------------
# 25LC1024 PDIP-8 (JEDEC SPI - same pad numbers as SST25VF010A SOIC-8)
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

# Switchcraft 35RAPC2BVN4 (same as full Retr01): Tip=4 Ring=2 Sleeve=1; 3+5 NC
TRS_TIP, TRS_RING, TRS_SLEEVE = "4", "2", "1"
TRS_NC = ("3", "5")

# CUI RCJ-01x RCA: pad 1 = tip, pad 2 = shell
RCA_TIP, RCA_SHELL = "1", "2"

# Abracon ACH half-size DIP-8 can (KiCad Oscillator_DIP-8 pads 1/4/5/8)
OSC_OE, OSC_GND, OSC_OUT, OSC_VDD = "1", "4", "5", "8"

# AD725 RGB->NTSC (AD725ARZ wide SOIC-16, soldered direct - no PA0006)
AD725_STND, AD725_AGND, AD725_4FSC, AD725_APOS = "1", "2", "3", "4"
AD725_CE, AD725_RIN, AD725_GIN, AD725_BIN = "5", "6", "7", "8"
AD725_CRMA, AD725_COMP, AD725_LUMA, AD725_YTRAP = "9", "10", "11", "12"
AD725_DGND, AD725_DPOS, AD725_VSYNC, AD725_HSYNC = "13", "14", "15", "16"

# Cart edge 16 - A1..A8 = 1..8, B1..B8 = 9..16
def cart_a(n: int) -> str:
    return str(n)


def cart_b(n: int) -> str:
    return str(8 + n)


# Double-row headers use KiCad PinHeader_2xN numbering (column pairs):
#   1  2
#   3  4
#   ...

# AVR ISP 2x3 (Atmel / USBasp style, PinHeader_2x03)
# 1 MISO | 2 VCC
# 3 SCK  | 4 MOSI
# 5 RST  | 6 GND
ISP_MISO, ISP_VCC = "1", "2"
ISP_SCK, ISP_MOSI = "3", "4"
ISP_RST, ISP_GND = "5", "6"

# J_PWR 2x2
# 1 +5V    | 2 GND
# 3 RESET# | 4 NC
PWR_5V, PWR_GND = "1", "2"
PWR_RST, PWR_NC = "3", "4"

# J_AV 2x4 - RGBS + AUD bring-up header (also feeds AD725 / RCA)
# 1 R     | 2 G
# 3 B     | 4 CSYNC
# 5 AUD   | 6 AGND
# 7 VGND  | 8 NC
AV_R, AV_G = "1", "2"
AV_B, AV_CSYNC = "3", "4"
AV_AUD, AV_AGND = "5", "6"
AV_VGND, AV_NC = "7", "8"

# J_PAD 2x10 - P1 left column, P2 right column (bit-paired)
#  1 P1_D0 |  2 P2_D0
#  ...
# 15 P1_D7 | 16 P2_D7
# 17 GND   | 18 GND
# 19 key   | 20 NC
def pad_p1(bit: int) -> str:
    """J_PAD pin for P1_D{bit} (0..7)."""
    return str(1 + 2 * bit)


def pad_p2(bit: int) -> str:
    """J_PAD pin for P2_D{bit} (0..7)."""
    return str(2 + 2 * bit)


PAD_GND_L, PAD_GND_R = "17", "18"
PAD_KEY, PAD_NC = "19", "20"


def _nums(n: int) -> List[str]:
    return [str(i) for i in range(1, n + 1)]


PIN_TEMPLATES: Dict[str, List[str]] = {
    "ATmega1284P": _nums(40),
    "25LC1024": _nums(8),
    "24C64": _nums(8),
    "AD725": _nums(16),
    "OSC_4FSC": ["1", "4", "5", "8"],
    "CART_EDGE_16": _nums(16),
    "CART_SOCKET_16": _nums(16),
    "BARREL_5V": ["1", "2", "MP"],
    "J_AV": _nums(8),
    "J_PAD": _nums(20),
    "J_PWR": _nums(4),
    "J_ISP": _nums(6),
    "TRS_P1": _nums(5),
    "TRS_P2": _nums(5),
    "AUDIO_OUT": _nums(2),
    "COMPOSITE_OUT": _nums(2),
    "XTAL_20M": _nums(2),
    "SCHOTTKY": _nums(2),
    "PPTC": _nums(2),
    "FERRITE": _nums(2),
    "L_YTRAP": _nums(2),
    "C_BULK": _nums(2),
    "C_10U": _nums(2),
    "C_100N": _nums(2),
    "C_22P": _nums(2),
    "C_10U_AUD": _nums(2),
    "R_0": _nums(2),
    "R_47": _nums(2),
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
    "25LC1024": {
        "1": "~{CS}",
        "2": "SO",
        "3": "~{WP}",
        "4": "VSS",
        "5": "SI",
        "6": "SCK",
        "7": "~{HOLD}",
        "8": "VCC",
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
    "AD725": {
        "1": "STND",
        "2": "AGND",
        "3": "4FSC",
        "4": "APOS",
        "5": "CE",
        "6": "RIN",
        "7": "GIN",
        "8": "BIN",
        "9": "CRMA",
        "10": "COMP",
        "11": "LUMA",
        "12": "YTRAP",
        "13": "DGND",
        "14": "DPOS",
        "15": "VSYNC",
        "16": "HSYNC",
    },
    "OSC_4FSC": {"1": "Tri-State", "4": "GND", "5": "OUT", "8": "Vcc"},
}


def power_pin_nums(mpn: str) -> Optional[Tuple[str, str]]:
    """Primary VCC/GND pad numbers for Quilter local-bypass helpers."""
    table = {
        "ATmega1284P": ("10", "11"),
        "25LC1024": (FLASH_VDD, FLASH_VSS),
        "24C64": (EE_VCC, EE_GND),
        "OSC_4FSC": (OSC_VDD, OSC_GND),
    }
    return table.get(mpn)
