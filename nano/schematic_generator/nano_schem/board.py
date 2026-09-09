"""Top-level Nano motherboard and cartridge assemblies."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List

from .bom import BoardId, CART_IC_COUNT, MOBO_IC_COUNT, _C0603, _R0603, silicon_ic_entries
from .cart_manifest import build_cart_manifest, j16_pin_to_net
from .manifest import build_manifest, manifest_gaps
from .parts import instantiate_bom, make_passive, skidl_available
from .pinmap import power_pin_nums


def export_manifest_json(path: Path, *, board: BoardId = BoardId.MOBO) -> None:
    if board == BoardId.CART:
        manifest = build_cart_manifest()
        gaps: List[str] = []
    else:
        manifest = build_manifest()
        gaps = manifest_gaps()
    payload = [
        {
            "net": c.net,
            "a": {"refdes": c.a_refdes, "pin": c.a_pin},
            "b": {"refdes": c.b_refdes, "pin": c.b_pin},
            "source": c.source,
        }
        for c in manifest
    ]
    path.write_text(json.dumps({"board": board.value, "connections": payload, "gaps": gaps}, indent=2) + "\n")


def add_mobo_passives(parts: Dict[str, object]) -> None:
    """Crystal caps, pull-ups, FG/PWM/CSYNC discretes (not in static BOM list)."""
    if not skidl_available():
        return
    for ref, mpn in (
        ("Cxtala", "C_22P"),
        ("Cxtalb", "C_22P"),
        ("Caref", "C_100N"),
        ("Rrst", "R_10K"),
        ("Rdet", "R_10K"),
        ("Rpu_sda", "R_4K7"),
        ("Rpu_scl", "R_4K7"),
        ("Rcsync_h", "R_1K"),
        ("Rcsync_v", "R_1K"),
        ("Rfg_r", "R_470"),
        ("Rfg_g", "R_470"),
        ("Rfg_b", "R_470"),
        ("R75r", "R_75"),
        ("R75g", "R_75"),
        ("R75b", "R_75"),
        ("Rmix_s", "R_1K"),
        ("Rmix_m", "R_1K"),
        ("Caud", "C_10U_AUD"),
    ):
        parts[ref] = make_passive(mpn, ref, _C0603 if mpn.startswith("C_") else _R0603)


def add_decoupling(parts: Dict[str, object], nets: Dict[str, object], board: BoardId) -> Dict[str, object]:
    if not skidl_available():
        return nets

    from skidl import Net

    from .connect import rail_net

    vcc = nets.get("+5V") or rail_net("+5V")
    gnd = nets.get("GND") or rail_net("GND")
    vcc.name = "+5V"
    gnd.name = "GND"
    nets["+5V"] = vcc
    nets["GND"] = gnd

    for n, entry in enumerate(silicon_ic_entries(board), start=1):
        pins = power_pin_nums(entry.mpn)
        if pins is None:
            continue
        vcc_name, gnd_name = pins
        part = parts.get(entry.refdes)
        if part is None:
            continue
        cap_ref = f"CD{n}"
        bridge_ref = f"RD{n}"
        local_name = f"+5V_{entry.refdes}"
        try:
            vp = part[vcc_name]
            gp = part[gnd_name]
        except Exception:
            continue
        local = Net(local_name)
        local.name = local_name
        nets[local_name] = local
        cap = make_passive("C_100N", cap_ref, _C0603)
        bridge = make_passive("R_0", bridge_ref, _R0603)
        parts[cap_ref] = cap
        parts[bridge_ref] = bridge
        try:
            vp.disconnect()
        except Exception:
            pass
        vp += local
        bridge["1"] += vcc
        bridge["2"] += local
        cap["1"] += local
        cap["2"] += gnd
        gp += gnd
    return nets


def build_board() -> Dict[str, Any]:
    from .connect import apply_connections

    manifest = build_manifest()
    parts = instantiate_bom(BoardId.MOBO)
    add_mobo_passives(parts)
    nets = apply_connections(parts, manifest)
    nets = add_decoupling(parts, nets, BoardId.MOBO)
    return {"parts": parts, "nets": nets, "manifest": manifest}


def build_cart() -> Dict[str, Any]:
    from .connect import apply_connections

    manifest = build_cart_manifest()
    parts = instantiate_bom(BoardId.CART)
    nets = apply_connections(parts, manifest)
    nets = add_decoupling(parts, nets, BoardId.CART)
    return {"parts": parts, "nets": nets, "manifest": manifest}


def connect_unused_pins_to_nc() -> None:
    import builtins

    from skidl import Net

    circuit = builtins.default_circuit
    for part in circuit.parts:
        ref = getattr(part, "ref", "U")
        for pin in part.pins:
            if pin.nets:
                continue
            dummy = Net(f"NC_{ref}_{pin.num}")
            pin += dummy


def _force_rail_net_names() -> None:
    from .connect import rail_net

    for name in ("GND", "+5V", "+5V_ANALOG"):
        try:
            n = rail_net(name)
            n.name = name
        except Exception:
            continue


def generate_netlist(out_dir: Path, basename: str) -> Path:
    if not skidl_available():
        raise RuntimeError("skidl is not installed; pip install -r requirements.txt")

    from skidl import ERC, generate_netlist, reset

    reset()
    if "cart" in basename:
        build_cart()
    else:
        build_board()
    connect_unused_pins_to_nc()
    _force_rail_net_names()
    out_dir.mkdir(parents=True, exist_ok=True)
    net_path = out_dir / f"{basename}.net"
    generate_netlist(file=str(net_path))
    ERC()
    return net_path


def generate_both_netlists(out_dir: Path) -> Dict[str, Path]:
    mobo = generate_netlist(out_dir, "nano_mobo")
    cart = generate_netlist(out_dir, "nano_cart")
    return {"mobo": mobo, "cart": cart}


def validate_edge_contract() -> List[str]:
    """Every J_CART pin net on the motherboard must match the cart J16 edge."""
    errors: List[str] = []
    try:
        # Build synthetic lists using J_CART vs J16 with same pin numbers
        from .manifest import Connection

        mobo_conns = []
        for c in build_manifest():
            if c.a_refdes == "J_CART":
                mobo_conns.append(Connection(c.net, "J_CART", c.a_pin, c.b_refdes, c.b_pin, c.source))
            if c.b_refdes == "J_CART":
                mobo_conns.append(Connection(c.net, "J_CART", c.b_pin, c.a_refdes, c.a_pin, c.source))
        mobo = j16_pin_to_net(mobo_conns)
        cart = j16_pin_to_net(build_cart_manifest())
    except ValueError as e:
        return [str(e)]
    for pin in sorted(set(mobo) | set(cart), key=lambda p: int(p) if p.isdigit() else p):
        if pin not in mobo:
            errors.append(f"edge pin {pin} on cart missing on mobo")
        elif pin not in cart:
            errors.append(f"edge pin {pin} on mobo missing on cart")
        elif mobo[pin] != cart[pin]:
            errors.append(f"edge pin {pin}: mobo {mobo[pin]!r} != cart {cart[pin]!r}")
    return errors


def validate_bom_counts() -> List[str]:
    errors: List[str] = []
    mobo_ics = silicon_ic_entries(BoardId.MOBO)
    cart_ics = silicon_ic_entries(BoardId.CART)
    if len(mobo_ics) != MOBO_IC_COUNT:
        errors.append(f"expected {MOBO_IC_COUNT} motherboard silicon ICs, got {len(mobo_ics)}")
    if len(cart_ics) != CART_IC_COUNT:
        errors.append(f"expected {CART_IC_COUNT} cart silicon ICs, got {len(cart_ics)}")
    errors.extend(validate_edge_contract())
    return errors
