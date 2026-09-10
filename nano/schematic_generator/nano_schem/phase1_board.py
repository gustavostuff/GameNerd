"""Phase 1 board assembly and netlist export."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List

from .connect import apply_connections, rail_net
from .parts import make_part, make_passive, skidl_available
from .phase1_bom import (
    PHASE1_IC_COUNT,
    _C0603,
    _R0603,
    phase1_bom_entries,
    phase1_silicon_ics,
)
from .phase1_manifest import build_phase1_manifest
from .pinmap import power_pin_nums


def add_phase1_passives(parts: Dict[str, object]) -> None:
    if not skidl_available():
        return
    for ref, mpn in (
        ("Cxtala", "C_22P"),
        ("Cxtalb", "C_22P"),
        ("Caref", "C_100N"),
        ("Rrst", "R_10K"),
        ("Rcsync_h", "R_1K"),
        ("Rcsync_v", "R_1K"),
        ("Rfg_r", "R_470"),
        ("Rfg_g", "R_470"),
        ("Rfg_b", "R_470"),
        ("R75r", "R_75"),
        ("R75g", "R_75"),
        ("R75b", "R_75"),
    ):
        parts[ref] = make_passive(mpn, ref, _C0603 if mpn.startswith("C_") else _R0603)


def add_phase1_decoupling(parts: Dict[str, object], nets: Dict[str, object]) -> Dict[str, object]:
    if not skidl_available():
        return nets

    from skidl import Net

    vcc = nets.get("+5V") or rail_net("+5V")
    gnd = nets.get("GND") or rail_net("GND")
    vcc.name = "+5V"
    gnd.name = "GND"
    nets["+5V"] = vcc
    nets["GND"] = gnd

    for n, entry in enumerate(phase1_silicon_ics(), start=1):
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


def instantiate_phase1_bom() -> Dict[str, object]:
    from .parts import add_library_paths

    add_library_paths()
    parts: Dict[str, object] = {}
    for entry in phase1_bom_entries():
        if entry.dip_pins <= 0:
            continue
        parts[entry.refdes] = make_part(entry)
    return parts


def build_phase1() -> Dict[str, Any]:
    manifest = build_phase1_manifest()
    parts = instantiate_phase1_bom()
    add_phase1_passives(parts)
    nets = apply_connections(parts, manifest)
    nets = add_phase1_decoupling(parts, nets)
    return {"parts": parts, "nets": nets, "manifest": manifest}


def export_phase1_manifest_json(path: Path) -> None:
    manifest = build_phase1_manifest()
    payload = [
        {
            "net": c.net,
            "a": {"refdes": c.a_refdes, "pin": c.a_pin},
            "b": {"refdes": c.b_refdes, "pin": c.b_pin},
            "source": c.source,
        }
        for c in manifest
    ]
    path.write_text(
        json.dumps({"board": "phase1", "connections": payload, "gaps": []}, indent=2) + "\n"
    )


def validate_phase1_bom() -> List[str]:
    errors: List[str] = []
    ics = phase1_silicon_ics()
    if len(ics) != PHASE1_IC_COUNT:
        errors.append(f"expected {PHASE1_IC_COUNT} Phase 1 silicon IC(s), got {len(ics)}")
    refs = {e.refdes for e in phase1_bom_entries()}
    for need in ("U1284", "Y1", "J_BARREL", "J_ISP", "J_AV", "D1", "Cbulk"):
        if need not in refs:
            errors.append(f"Phase 1 BOM missing {need}")
    # Cart must not appear
    for banned in ("J_CART", "U25", "U50", "U725", "J_PAD", "J8", "J9", "J3", "J4"):
        if banned in refs:
            errors.append(f"Phase 1 BOM must not include {banned}")
    return errors


def generate_phase1_netlist(out_dir: Path, basename: str = "nano_phase1") -> Path:
    if not skidl_available():
        raise RuntimeError("skidl is not installed; pip install -r requirements.txt")

    from skidl import ERC, generate_netlist, reset

    from .board import connect_unused_pins_to_nc

    reset()
    build_phase1()
    connect_unused_pins_to_nc()
    for name in ("GND", "+5V"):
        try:
            n = rail_net(name)
            n.name = name
        except Exception:
            continue
    out_dir.mkdir(parents=True, exist_ok=True)
    net_path = out_dir / f"{basename}.net"
    generate_netlist(file=str(net_path))
    ERC()
    return net_path
