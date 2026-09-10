#!/usr/bin/env python3
"""Generate Retr01 Nano Phase 1 lab netlist (RGBS bring-up, no cart)."""

from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

_KICAD_SYM = "/usr/share/kicad/symbols"
os.environ.setdefault("KICAD_SYMBOL_DIR", _KICAD_SYM)
os.environ.setdefault("KICAD10_SYMBOL_DIR", _KICAD_SYM)
for _v in ("KICAD6_SYMBOL_DIR", "KICAD7_SYMBOL_DIR", "KICAD8_SYMBOL_DIR", "KICAD9_SYMBOL_DIR"):
    os.environ.setdefault(_v, _KICAD_SYM)

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from nano_schem.phase1_board import (  # noqa: E402
    export_phase1_manifest_json,
    generate_phase1_netlist,
    validate_phase1_bom,
)
from nano_schem.phase1_manifest import build_phase1_manifest  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Retr01 Nano Phase 1 SKiDL generator (MCU + crystal + RGBS + ISP)"
    )
    ap.add_argument(
        "--out",
        type=Path,
        default=ROOT / "output",
        help="output directory for netlist and manifest JSON",
    )
    ap.add_argument("--manifest-only", action="store_true", help="write wiring manifest JSON only")
    ap.add_argument("--check", action="store_true", help="validate Phase 1 BOM and exit")
    args = ap.parse_args()

    errs = validate_phase1_bom()
    if errs:
        for e in errs:
            print(f"ERROR: {e}", file=sys.stderr)
        return 1

    if args.check:
        print(f"OK: Phase 1 BOM ({len(build_phase1_manifest())} connections)")
        return 0

    args.out.mkdir(parents=True, exist_ok=True)
    export_phase1_manifest_json(args.out / "nano_phase1_wiring_manifest.json")

    if args.manifest_only:
        print(f"wrote {args.out / 'nano_phase1_wiring_manifest.json'}")
        return 0

    net_path = generate_phase1_netlist(args.out)
    print(f"wrote {net_path}")
    print(f"wrote {args.out / 'nano_phase1_wiring_manifest.json'}")
    print("Phase 1: no cart netlist. Import nano_phase1.net into a lab KiCad project or proto from the nets.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
