#!/usr/bin/env python3
"""Report code-region ownership in the generated disassembly.

    python3 tools/coverage.py [rs|bm]

Each code segment is either represented by GNU-as source or retained as an
``incrom`` range that reads from the player's baserom at build time. This tool
reports the exact byte count of each category; it never reads ROM data.
"""
from __future__ import annotations

import re
import sys
import tomllib
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
INCROM_RE = re.compile(r"^\s*incrom\s+0x[0-9A-Fa-f]+\s*,\s*0x([0-9A-Fa-f]+)\b")


def asm_path(version: str, name: str) -> Path:
    return DECOMP / "asm" / ("" if version == "rs" else version) / f"{name}.s"


def incrom_bytes(path: Path) -> int:
    total = 0
    for line in path.read_text().splitlines():
        match = INCROM_RE.match(line)
        if match:
            total += int(match.group(1), 16)
    return total


def main() -> None:
    version = sys.argv[1] if len(sys.argv) > 1 else "rs"
    if version not in {"rs", "bm"}:
        sys.exit("usage: coverage.py [rs|bm]")
    config = tomllib.loads((DECOMP / "config" / f"segments_{version}.toml").read_text())
    rows = []
    for segment in config["segment"]:
        if segment["kind"] != "code":
            continue
        path = asm_path(version, segment["name"])
        if not path.exists():
            sys.exit(f"missing generated source: {path.relative_to(DECOMP)}")
        total = segment["end"] - segment["start"]
        raw = incrom_bytes(path)
        if raw > total:
            sys.exit(f"incrom ranges exceed {segment['name']} by {raw - total:#x} bytes")
        rows.append((segment["name"], total, total - raw, raw))

    print(f"{version.upper()} code ownership (generated source vs. incrom)")
    print(f"{'segment':<12} {'total':>10} {'source':>10} {'incrom':>10} {'source %':>9}")
    for name, total, owned, raw in rows:
        print(f"{name:<12} {total:10d} {owned:10d} {raw:10d} {owned / total * 100:8.1f}%")
    total = sum(row[1] for row in rows)
    owned = sum(row[2] for row in rows)
    raw = sum(row[3] for row in rows)
    print(f"{'TOTAL':<12} {total:10d} {owned:10d} {raw:10d} {owned / total * 100:8.1f}%")


if __name__ == "__main__":
    main()
