"""Cartridge PCB wiring — 25LC1024 + 24C64 behind the shared 2x8 edge."""

from __future__ import annotations

from typing import Dict, List

from . import pinmap as P
from .manifest import Connection


def build_cart_manifest() -> List[Connection]:
    m: List[Connection] = []
    src = "nano/docs/hardware.md cart PCB"

    # Edge mirrors (same nets on A/B except B8 = detect)
    for net, n in (
        ("GND", 1),
        ("+5V", 2),
        ("SPI_SS#", 3),
        ("SPI_SCK", 4),
        ("SPI_MOSI", 5),
        ("SPI_MISO", 6),
        ("I2C_SDA", 7),
    ):
        m.append(Connection(net, "J16", P.cart_a(n), "J16", P.cart_b(n), src))

    m.append(Connection("I2C_SCL", "J16", P.cart_a(8), "U50", P.EE_SCL, src))
    m.append(Connection("CART_DET#", "J16", P.cart_b(8), "GND", "GND", src))

    m += [
        Connection("GND", "J16", P.cart_a(1), "GND", "GND", src),
        Connection("+5V", "J16", P.cart_a(2), "+5V", "+5V", src),
        Connection("SPI_SS#", "J16", P.cart_a(3), "U25", P.FLASH_CE, src),
        Connection("SPI_SCK", "J16", P.cart_a(4), "U25", P.FLASH_SCK, src),
        Connection("SPI_MOSI", "J16", P.cart_a(5), "U25", P.FLASH_SI, src),
        Connection("SPI_MISO", "J16", P.cart_a(6), "U25", P.FLASH_SO, src),
        Connection("I2C_SDA", "J16", P.cart_a(7), "U50", P.EE_SDA, src),
        # HOLD# / WP# tied on cart
        Connection("+5V", "U25", P.FLASH_HOLD, "+5V", "+5V", src),
        Connection("+5V", "U25", P.FLASH_WP, "+5V", "+5V", src),
        Connection("+5V", "U25", P.FLASH_VDD, "+5V", "+5V", src),
        Connection("GND", "U25", P.FLASH_VSS, "GND", "GND", src),
        # EEPROM address / WP
        Connection("GND", "U50", P.EE_A0, "GND", "GND", src),
        Connection("GND", "U50", P.EE_A1, "GND", "GND", src),
        Connection("GND", "U50", P.EE_A2, "GND", "GND", src),
        Connection("GND", "U50", P.EE_WP, "GND", "GND", src),
        Connection("+5V", "U50", P.EE_VCC, "+5V", "+5V", src),
        Connection("GND", "U50", P.EE_GND, "GND", "GND", src),
    ]
    return m


def j16_pin_to_net(connections: List[Connection]) -> Dict[str, str]:
    """Map edge pin number -> net name (one net per contact)."""
    out: Dict[str, str] = {}
    for c in connections:
        for ref, pin in ((c.a_refdes, c.a_pin), (c.b_refdes, c.b_pin)):
            if ref not in ("J16", "J_CART"):
                continue
            prev = out.get(pin)
            if prev is not None and prev != c.net:
                raise ValueError(f"edge pin {pin} has conflicting nets {prev!r} vs {c.net!r}")
            out[pin] = c.net
    return out
