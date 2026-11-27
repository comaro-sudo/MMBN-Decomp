#!/usr/bin/env python3
"""Emit BN4_ENEMY_HELD_CHIPS (code_09, RS 0x080F8A80 / BM 0x080F8A8C) in place of its incrom line.

244 records x 16 halfwords, indexed by enemy id (sub_080F8A70: base + id * 0x20).
At battle start, sub_080F576C picks one random entry (RNG & 0x1E) for up to four
enemy objects and stores it at obj+0x2A (0xFFFF = none). Each halfword is a chip
and its code: id = v & 0x1FF, code = v >> 9 (0-25 = A-Z, 26 = *).
The table is identical in RS and BM (BM = RS + SEG_BM_DELTA).

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_enemy_held_chips.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
OFF, SIZE, REC = 0xF8A80, 0x1E80, 0x20
LINE = f"    incrom 0x{OFF:X}, 0x{SIZE:X}"
MACROS = """
.macro chip_code id, code
    .hword (\\id) | ((\\code) << 9)
.endm
"""


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    path = DECOMP / "asm/code_09.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1, "incrom line not found exactly once"
    delta = 0xC
    assert rs[OFF:OFF + SIZE] == bm[OFF + delta:OFF + delta + SIZE], "RS and BM differ"

    out = []
    for i in range(SIZE // REC):
        vals = struct.unpack_from("<16H", rs, OFF + i * REC)
        if all(v == 0xFFFF for v in vals):
            out.append(f"    .fill 16, 2, 0xFFFF   @ enemy 0x{i:02X}: none")
            continue
        out.append(f"    @ enemy 0x{i:02X}")
        for v in vals:
            if v == 0xFFFF:
                out.append("    .hword 0xFFFF")
            else:
                out.append(f"    chip_code 0x{v & 0x1FF:03X}, {v >> 9}")
    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    if ".macro chip_code" not in text:
        text = text.replace("    .thumb\n", "    .thumb\n" + MACROS, 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted {SIZE // REC} records ({SIZE} bytes)")


if __name__ == "__main__":
    main()
