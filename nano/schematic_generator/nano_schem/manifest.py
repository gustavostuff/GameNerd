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

    # --- J_PWR 2x2 ---
    m += [
        Connection("+5V", "J_PWR", P.PWR_5V, "+5V", "+5V", src),
        Connection("GND", "J_PWR", P.PWR_GND, "GND", "GND", src),
        Connection("RESET#", "J_PWR", P.PWR_RST, "U1284", P.M1284_RESET, src),
    ]

    # --- arcade 2P on one 2x10 (J_PAD): P1 left col, P2 right col ---
    for i, pin in enumerate(P.M1284_P1):
        m.append(Connection(f"P1_D{i}", "U1284", pin, "J_PAD", P.pad_p1(i), src))
    for i, pin in enumerate(P.M1284_P2):
        m.append(Connection(f"P2_D{i}", "U1284", pin, "J_PAD", P.pad_p2(i), src))
    m += [
        Connection("GND", "J_PAD", P.PAD_GND_L, "GND", "GND", src),
        Connection("GND", "J_PAD", P.PAD_GND_R, "GND", "GND", src),
    ]

    # --- sync: bring-up CSYNC from HSYNC (PD0); VSYNC still routed for FW XOR later ---
    m += [
        Connection("HSYNC", "U1284", P.M1284_PD0, "Rcsync_h", "1", src),
        Connection("VSYNC", "U1284", P.M1284_PD1, "Rcsync_v", "1", src),
        Connection("CSYNC", "Rcsync_h", "2", "J_AV", P.AV_CSYNC, src),
        Connection("CSYNC", "Rcsync_v", "2", "J_AV", P.AV_CSYNC, src),
    ]

    # --- FG: FG0/1/2 as R/G/B enables (8 primary colors), 470R + 75R ---
    for bit, color, av_pin, rref, term in (
        (0, "R", P.AV_R, "Rfg_r", "R75r"),
        (1, "G", P.AV_G, "Rfg_g", "R75g"),
        (2, "B", P.AV_B, "Rfg_b", "R75b"),
    ):
        m += [
            Connection(f"FG{bit}", "U1284", P.M1284_FG[bit], rref, "1", src),
            Connection(color, rref, "2", "J_AV", av_pin, src),
            Connection(color, rref, "2", term, "1", src),
            Connection("VGND", term, "2", "J_AV", P.AV_VGND, src),
        ]
    m.append(Connection("VGND", "J_AV", P.AV_VGND, "GND", "GND", src))

    # --- PWM audio mix → J_AV + J8 mono RCA ---
    m += [
        Connection("PWM_SFX", "U1284", P.M1284_PD4, "Rmix_s", "1", src),
        Connection("PWM_MUSIC", "U1284", P.M1284_PD5, "Rmix_m", "1", src),
        Connection("AUD_MIX", "Rmix_s", "2", "Rmix_m", "2", src),
        Connection("AUD_MIX", "Rmix_m", "2", "Caud", "1", src),
        Connection("AUD", "Caud", "2", "J_AV", P.AV_AUD, src),
        Connection("AGND", "J_AV", P.AV_AGND, "GND", "GND", src),
        Connection("AUD", "Caud", "2", "J8", P.RCA_TIP, "docs/passive_rf_etc.md RCJ audio"),
        Connection("GND", "J8", P.RCA_SHELL, "GND", "GND", "docs/passive_rf_etc.md RCJ audio"),
    ]

    # --- Analog spur for AD725 ---
    av = "docs/passive_rf_etc.md AD725"
    m += [
        Connection("+5V", "FB2", "1", "+5V", "+5V", av),
        Connection("+5V_ANALOG", "FB2", "2", "Cva", "1", av),
        Connection("GND", "Cva", "2", "GND", "GND", av),
    ]

    # --- AD725 composite (NTSC): tap R/G/B guns + CSYNC; COMP -> 75R -> J9 ---
    m += [
        Connection("R", "CinR", "1", "J_AV", P.AV_R, av),
        Connection("AD725_RIN", "CinR", "2", "U725", P.AD725_RIN, av),
        Connection("G", "CinG", "1", "J_AV", P.AV_G, av),
        Connection("AD725_GIN", "CinG", "2", "U725", P.AD725_GIN, av),
        Connection("B", "CinB", "1", "J_AV", P.AV_B, av),
        Connection("AD725_BIN", "CinB", "2", "U725", P.AD725_BIN, av),
        Connection("CSYNC", "J_AV", P.AV_CSYNC, "U725", P.AD725_HSYNC, av),
        Connection("+5V", "U725", P.AD725_VSYNC, "+5V", "+5V", av),
        Connection("+5V", "U725", P.AD725_STND, "+5V", "+5V", av),
        Connection("+5V", "U725", P.AD725_CE, "+5V", "+5V", av),
        Connection("+5V_ANALOG", "U725", P.AD725_APOS, "+5V_ANALOG", "+5V_ANALOG", av),
        Connection("+5V_ANALOG", "U725", P.AD725_DPOS, "+5V_ANALOG", "+5V_ANALOG", av),
        Connection("GND", "U725", P.AD725_AGND, "GND", "GND", av),
        Connection("GND", "U725", P.AD725_DGND, "GND", "GND", av),
        Connection("+5V_ANALOG", "Cd725a", "1", "U725", P.AD725_APOS, av),
        Connection("GND", "Cd725a", "2", "GND", "GND", av),
        Connection("+5V_ANALOG", "Cd725d", "1", "U725", P.AD725_DPOS, av),
        Connection("GND", "Cd725d", "2", "GND", "GND", av),
        Connection("FSC4", "Y3", P.OSC_OUT, "U725", P.AD725_4FSC, av),
        Connection("+5V", "Y3", P.OSC_VDD, "+5V", "+5V", av),
        Connection("+5V", "Y3", P.OSC_OE, "+5V", "+5V", av),
        Connection("GND", "Y3", P.OSC_GND, "GND", "GND", av),
        Connection("YTRAP", "U725", P.AD725_YTRAP, "Lytrap", "1", av),
        Connection("YTRAP_MID", "Lytrap", "2", "Cytrap", "1", av),
        Connection("GND", "Cytrap", "2", "GND", "GND", av),
        Connection("COMP_RAW", "U725", P.AD725_COMP, "R75C", "1", av),
        Connection("COMPOSITE_OUT", "R75C", "2", "J9", P.RCA_TIP, av),
        Connection("GND", "J9", P.RCA_SHELL, "GND", "GND", av),
    ]

    # --- TRS aux pads (Retr01-C protocol): Tip=VCC, Ring=DATA, Sleeve=GND ---
    ctrl = "docs/controllers.md"
    passives = "docs/passive_rf_etc.md"
    m += [
        Connection("+5V", "F2", "1", "+5V", "+5V", ctrl),
        Connection("PAD_VCC_P1", "F2", "2", "J3", P.TRS_TIP, ctrl),
        Connection("PAD_VCC_P1", "Cpad1", "1", "J3", P.TRS_TIP, passives),
        Connection("GND", "Cpad1", "2", "GND", "GND", passives),
        Connection("GND", "J3", P.TRS_SLEEVE, "GND", "GND", ctrl),
        Connection("+5V", "F3", "1", "+5V", "+5V", ctrl),
        Connection("PAD_VCC_P2", "F3", "2", "J4", P.TRS_TIP, ctrl),
        Connection("PAD_VCC_P2", "Cpad2", "1", "J4", P.TRS_TIP, passives),
        Connection("GND", "Cpad2", "2", "GND", "GND", passives),
        Connection("GND", "J4", P.TRS_SLEEVE, "GND", "GND", ctrl),
        Connection("+5V", "Rpu1", "1", "+5V", "+5V", ctrl),
        Connection("PAD_DATA", "Rpu1", "2", "U1284", P.M1284_PAD_DATA, ctrl),
        Connection("PAD_DATA", "U1284", P.M1284_PAD_DATA, "Rdata1", "1", passives),
        Connection("PAD_DATA_P1", "Rdata1", "2", "J3", P.TRS_RING, passives),
        Connection("PAD_DATA", "U1284", P.M1284_PAD_DATA, "Rdata2", "1", passives),
        Connection("PAD_DATA_P2", "Rdata2", "2", "J4", P.TRS_RING, passives),
    ]

    return m


def manifest_gaps() -> List[str]:
    return [
        "FG DAC is bring-up RGB-bit (FG0=R, FG1=G, FG2=B); final 8-color resistor network TBD",
        "CSYNC is resistor mix of HSYNC+VSYNC; prefer FW composite on one pin or XOR glue later",
        "AD725 levels: bench-tune R/G/B into 0-714 mV AC-coupled (parent passive_rf_etc.md)",
        "EDAC 395-016 CAD not imported; J_CART uses PinSocket_2x08 stand-in",
        "Cart game chip is 25LC1024 SPI EEPROM (write/erase cmds differ from NOR flash)",
        "No TVS packs on TRS/cart in bring-up profile yet (parent --full-esd)",
        "PPTC/ferrite footprints are DIN0207 hole stand-ins until THT MPNs lock",
        "Flasher PCB (cart socket + ISP) not generated yet",
        "Motherboard outline 100x100 mm is provisional (AV + TRS may grow outline)",
        "Console FW must poll J3/J4 on PAD_DATA (PD2) like Retr01-C; arcade J_PAD stays GPIO",
    ]
