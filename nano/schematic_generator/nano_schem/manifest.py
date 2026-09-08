"""Motherboard wiring manifest (stops at J_CART)."""

from __future__ import annotations

from dataclasses import dataclass
from typing import List

from . import pinmap as P


@dataclass(frozen=True)
class Connection:
    net: str
    a_refdes: str
    a_pin: str
    b_refdes: str
    b_pin: str
    source: str = ""


def build_manifest() -> List[Connection]:
    m: List[Connection] = []
    src = "nano/docs/pinmap.md + hardware.md"

    # --- power inlet ---
    m += [
        Connection("VIN_RAW", "J_BARREL", P.BARREL_TIP, "D1", "1", src),
        Connection("GND", "J_BARREL", P.BARREL_SLEEVE, "GND", "GND", src),
        Connection("GND", "J_BARREL", P.BARREL_MP, "GND", "GND", src),
        Connection("+5V", "D1", "2", "Cbulk", "1", src),
        Connection("+5V", "Cbulk", "1", "+5V", "+5V", src),
        Connection("GND", "Cbulk", "2", "GND", "GND", src),
    ]

    # --- MCU power / AREF / AVCC ---
    for pin in P.M1284_VCC:
        m.append(Connection("+5V", "U1284", pin, "+5V", "+5V", src))
    for pin in P.M1284_GND:
        m.append(Connection("GND", "U1284", pin, "GND", "GND", src))
    m += [
        Connection("+5V", "U1284", P.M1284_AVCC, "+5V", "+5V", src),
        Connection("AREF", "U1284", P.M1284_AREF, "Caref", "1", src),
        Connection("GND", "Caref", "2", "GND", "GND", src),
        Connection("RESET#", "U1284", P.M1284_RESET, "Rrst", "1", src),
        Connection("+5V", "Rrst", "2", "+5V", "+5V", src),
    ]

    # --- crystal ---
    m += [
        Connection("XTAL2", "U1284", P.M1284_XTAL2, "Y1", "1", src),
        Connection("XTAL1", "U1284", P.M1284_XTAL1, "Y1", "2", src),
        Connection("XTAL2", "Y1", "1", "Cxtala", "1", src),
        Connection("XTAL1", "Y1", "2", "Cxtalb", "1", src),
        Connection("GND", "Cxtala", "2", "GND", "GND", src),
        Connection("GND", "Cxtalb", "2", "GND", "GND", src),
    ]

    # --- SPI / I2C / cart detect to J_CART (mirrored A/B) ---
    edge = [
        ("GND", 1, 1),
        ("+5V", 2, 2),
        ("SPI_SS#", 3, 3),
        ("SPI_SCK", 4, 4),
        ("SPI_MOSI", 5, 5),
        ("SPI_MISO", 6, 6),
        ("I2C_SDA", 7, 7),
    ]
    for net, a, b in edge:
        m.append(Connection(net, "J_CART", P.cart_a(a), "J_CART", P.cart_b(b), src))
    m.append(Connection("I2C_SCL", "J_CART", P.cart_a(8), "U1284", P.M1284_PC0, src))
    m.append(Connection("CART_DET#", "J_CART", P.cart_b(8), "U1284", P.M1284_PB3, src))
    m.append(Connection("CART_DET#", "U1284", P.M1284_PB3, "Rdet", "1", src))
    m.append(Connection("+5V", "Rdet", "2", "+5V", "+5V", src))

    m += [
        Connection("SPI_SS#", "U1284", P.M1284_PB4, "J_CART", P.cart_a(3), src),
        Connection("SPI_SCK", "U1284", P.M1284_PB7, "J_CART", P.cart_a(4), src),
        Connection("SPI_MOSI", "U1284", P.M1284_PB5, "J_CART", P.cart_a(5), src),
        Connection("SPI_MISO", "U1284", P.M1284_PB6, "J_CART", P.cart_a(6), src),
        Connection("I2C_SDA", "U1284", P.M1284_PC1, "J_CART", P.cart_a(7), src),
        Connection("I2C_SDA", "U1284", P.M1284_PC1, "Rpu_sda", "1", src),
        Connection("I2C_SCL", "U1284", P.M1284_PC0, "Rpu_scl", "1", src),
        Connection("+5V", "Rpu_sda", "2", "+5V", "+5V", src),
        Connection("+5V", "Rpu_scl", "2", "+5V", "+5V", src),
        Connection("GND", "J_CART", P.cart_a(1), "GND", "GND", src),
        Connection("+5V", "J_CART", P.cart_a(2), "+5V", "+5V", src),
    ]

    # --- ISP (shares SPI + RESET) ---
    m += [
        Connection("SPI_MISO", "J_ISP", P.ISP_MISO, "U1284", P.M1284_PB6, src),
        Connection("+5V", "J_ISP", P.ISP_VCC, "+5V", "+5V", src),
        Connection("SPI_SCK", "J_ISP", P.ISP_SCK, "U1284", P.M1284_PB7, src),
        Connection("SPI_MOSI", "J_ISP", P.ISP_MOSI, "U1284", P.M1284_PB5, src),
        Connection("RESET#", "J_ISP", P.ISP_RST, "U1284", P.M1284_RESET, src),
        Connection("GND", "J_ISP", P.ISP_GND, "GND", "GND", src),
    ]

    # --- J_PWR ---
    m += [
        Connection("+5V", "J_PWR", "1", "+5V", "+5V", src),
        Connection("GND", "J_PWR", "2", "GND", "GND", src),
        Connection("RESET#", "J_PWR", "3", "U1284", P.M1284_RESET, src),
    ]

    # --- arcade P1 / P2 ---
    for i, pin in enumerate(P.M1284_P1):
        m.append(Connection(f"P1_D{i}", "U1284", pin, "J_P1", str(i + 1), src))
    m.append(Connection("GND", "J_P1", "9", "GND", "GND", src))
    for i, pin in enumerate(P.M1284_P2):
        m.append(Connection(f"P2_D{i}", "U1284", pin, "J_P2", str(i + 1), src))
    m.append(Connection("GND", "J_P2", "9", "GND", "GND", src))

    # --- sync: bring-up CSYNC from HSYNC (PD0); VSYNC still routed for FW XOR later ---
    m += [
        Connection("HSYNC", "U1284", P.M1284_PD0, "Rcsync_h", "1", src),
        Connection("VSYNC", "U1284", P.M1284_PD1, "Rcsync_v", "1", src),
        Connection("CSYNC", "Rcsync_h", "2", "J_AV", "4", src),
        Connection("CSYNC", "Rcsync_v", "2", "J_AV", "4", src),
    ]

    # --- FG: FG0/1/2 as R/G/B enables (8 primary colors), 470R + 75R ---
    for bit, color, rref, term in (
        (0, "R", "Rfg_r", "R75r"),
        (1, "G", "Rfg_g", "R75g"),
        (2, "B", "Rfg_b", "R75b"),
    ):
        m += [
            Connection(f"FG{bit}", "U1284", P.M1284_FG[bit], rref, "1", src),
            Connection(color, rref, "2", "J_AV", str(bit + 1), src),
            Connection(color, rref, "2", term, "1", src),
            Connection("VGND", term, "2", "J_AV", "7", src),
        ]
    m.append(Connection("VGND", "J_AV", "7", "GND", "GND", src))

    # --- PWM audio mix ---
    m += [
        Connection("PWM_SFX", "U1284", P.M1284_PD4, "Rmix_s", "1", src),
        Connection("PWM_MUSIC", "U1284", P.M1284_PD5, "Rmix_m", "1", src),
        Connection("AUD_MIX", "Rmix_s", "2", "Rmix_m", "2", src),
        Connection("AUD_MIX", "Rmix_m", "2", "Caud", "1", src),
        Connection("AUD", "Caud", "2", "J_AV", "5", src),
        Connection("AGND", "J_AV", "6", "GND", "GND", src),
    ]

    return m


def manifest_gaps() -> List[str]:
    return [
        "FG DAC is bring-up RGB-bit (FG0=R, FG1=G, FG2=B); final 8-color resistor network TBD",
        "CSYNC is resistor mix of HSYNC+VSYNC; prefer FW composite on one pin or XOR glue later",
        "EDAC 395-016 CAD not imported; J_CART uses PinSocket_2x08 stand-in",
        "Cart game chip is 25LC1024 SPI EEPROM (write/erase cmds differ from NOR flash)",
        "No TVS / PPTC / cart series R in BRINGUP profile yet",
        "Flasher PCB (cart socket + ISP) not generated yet",
        "Motherboard outline 100x100 mm is provisional (not in netlist)",
    ]
