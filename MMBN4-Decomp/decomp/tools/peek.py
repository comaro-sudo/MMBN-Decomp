#!/usr/bin/env python3
"""Quick disassembly listing for exploration.

    python3 tools/peek.py rs 0x080000C0 0x08000260 [arm|thumb]
"""
import sys
from pathlib import Path

from capstone import CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB, Cs

ROOT = Path(__file__).resolve().parents[2]
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
BASE = 0x08000000


def main() -> None:
    v, start, end = sys.argv[1], int(sys.argv[2], 0), int(sys.argv[3], 0)
    mode = sys.argv[4] if len(sys.argv) > 4 else "arm"
    d = ROMS[v].read_bytes()
    md = Cs(CS_ARCH_ARM, CS_MODE_ARM if mode == "arm" else CS_MODE_THUMB)
    a = start
    step = 4 if mode == "arm" else 2
    while a < end:
        insn = next(md.disasm(d[a - BASE:a - BASE + 4], a), None)
        if insn is None:
            w = d[a - BASE:a - BASE + step]
            print(f"{a:08X}: {w.hex():8}  .data")
            a += step
            continue
        print(f"{a:08X}: {insn.bytes.hex():8}  {insn.mnemonic:8} {insn.op_str}")
        a += insn.size


if __name__ == "__main__":
    main()
