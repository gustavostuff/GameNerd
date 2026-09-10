"""Phase 1 wiring manifest: MCU + crystal + RGBS resistor DAC + power + ISP.

No cartridge, pads, audio mix, AD725, or composite. See nano/README.md Phase 1.
"""

from __future__ import annotations

from typing import List

from . import pinmap as P
from .manifest import Connection


def build_phase1_manifest() -> List[Connection]:
    m: List[Connection] = []
    src = "nano/README.md Phase 1 + nano/docs/pinmap.md"

    # --- power inlet ---
    m += [
        Connection("VIN_RAW", "J_BARREL", P.BARREL_TIP, "D1", "1", src),
        Connection("GND", "J_BARREL", P.BARREL_SLEEVE, "GND", "GND", src),
        Connection("GND", "J_BARREL", P.BARREL_MP, "GND", "GND", src),
        Connection("+5V", "D1", "2", "Cbulk", "1", src),
        Connection("+5V", "Cbulk", "1", "+5V", "+5V", src),
        Connection("GND", "Cbulk", "2", "GND", "GND", src),
    ]

    # --- MCU power / AREF / AVCC / RESET ---
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

    # --- 20 MHz crystal ---
    m += [
        Connection("XTAL2", "U1284", P.M1284_XTAL2, "Y1", "1", src),
        Connection("XTAL1", "U1284", P.M1284_XTAL1, "Y1", "2", src),
        Connection("XTAL2", "Y1", "1", "Cxtala", "1", src),
        Connection("XTAL1", "Y1", "2", "Cxtalb", "1", src),
        Connection("GND", "Cxtala", "2", "GND", "GND", src),
        Connection("GND", "Cxtalb", "2", "GND", "GND", src),
    ]

    # --- ISP (USBasp shares SPI + RESET). No cart on these nets in Phase 1. ---
    m += [
        Connection("SPI_MISO", "J_ISP", P.ISP_MISO, "U1284", P.M1284_PB6, src),
        Connection("+5V", "J_ISP", P.ISP_VCC, "+5V", "+5V", src),
        Connection("SPI_SCK", "J_ISP", P.ISP_SCK, "U1284", P.M1284_PB7, src),
        Connection("SPI_MOSI", "J_ISP", P.ISP_MOSI, "U1284", P.M1284_PB5, src),
        Connection("RESET#", "J_ISP", P.ISP_RST, "U1284", P.M1284_RESET, src),
        Connection("GND", "J_ISP", P.ISP_GND, "GND", "GND", src),
    ]

    # --- J_PWR 2x2 ---
    m += [
        Connection("+5V", "J_PWR", P.PWR_5V, "+5V", "+5V", src),
        Connection("GND", "J_PWR", P.PWR_GND, "GND", "GND", src),
        Connection("RESET#", "J_PWR", P.PWR_RST, "U1284", P.M1284_RESET, src),
    ]

    # --- sync: HSYNC/VSYNC resistor-mix to CSYNC on J_AV ---
    m += [
        Connection("HSYNC", "U1284", P.M1284_PD0, "Rcsync_h", "1", src),
        Connection("VSYNC", "U1284", P.M1284_PD1, "Rcsync_v", "1", src),
        Connection("CSYNC", "Rcsync_h", "2", "J_AV", P.AV_CSYNC, src),
        Connection("CSYNC", "Rcsync_v", "2", "J_AV", P.AV_CSYNC, src),
    ]

    # --- FG0/1/2 -> R/G/B (8 colors), 470R series + 75R to VGND ---
    for bit, color, av_pin, rref, term in (
        (0, "R", P.AV_R, "Rfg_r", "R75r"),
        (1, "G", P.AV_G, "Rfg_g", "R75g"),
        (2, "B", P.AV_B, "Rfg_b", "R75b"),
    ):
        m += [
            Connection(f"FG{bit}", "U1284", P.M1284_FG[bit], rref, "1", src),
            Connection(color, rref, "2", "J_AV", av_pin, src),
            Connection(color, rref, "2", term, "1", src),
            Connection("GND", term, "2", "GND", "GND", src),
        ]
    m.append(Connection("GND", "J_AV", P.AV_VGND, "GND", "GND", src))
    # AUD unused in Phase 1. Tie AGND to GND so the header is quiet.
    m.append(Connection("GND", "J_AV", P.AV_AGND, "GND", "GND", src))

    return m
