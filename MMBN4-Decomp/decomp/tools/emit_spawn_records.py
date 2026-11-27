#!/usr/bin/env python3
"""Emit Object_SpawnList-format records (20 B) in place of given incrom blocks.

Record (Object_SpawnList, 0x080034E4): {u8 kind, u8 arg, u16 unk, s32 x, s32 y, s32 z
(16.16 fixed), u32 param}. Lists end with a kind of 0xFF (4 bytes ff 00 00 00);
fixed arrays (TERM = False) have no terminator.

Blocks:
- code_06 dat_0809CEC8 (RS 0x0809CEC8, 42 records, BM +0xC): cutscene actor spawn
  descriptors; sub_0802A760(n) returns entry n (x 20) and cutscene code keeps it at
  [r5, #0x34].

A param that is a ROM pointer must resolve as a dat_ label (checked against BM).
Claude, 2026-11-27. Run from decomp/: python3 tools/emit_spawn_records.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000
BLOCKS = [  # (segment, rs offset, size, bm delta, terminated)
    ("code_06", 0x9CEC8, 0x348, 0xC, False),
]
# Macros: include/data_macros.inc (loaded by include/macros.inc for every segment).


def fx(v: int) -> str:
    return f"({v >> 16} << 16)" if v & 0xFFFF == 0 else f"0x{v & 0xFFFFFFFF:08X}"


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    for seg, off, size, delta, term in BLOCKS:
        path = DECOMP / f"asm/{seg}.s"
        text = path.read_text()
        line = f"    incrom 0x{off:X}, 0x{size:X}"
        assert text.count(line + "\n") == 1, line
        out, p, end = [], off, off + size
        while p < end:
            if term and rs[p] == 0xFF:
                assert rs[p:p + 4] == b"\xff\0\0\0" == bm[p + delta:p + delta + 4], hex(p)
                out.append("    obj_spawn_end")
                p += 4
                continue
            k, a, u, x, y, z, prm = struct.unpack_from("<BBHiiiI", rs, p)
            bk, ba, bu, bx, by, bz, bprm = struct.unpack_from("<BBHiiiI", bm, p + delta)
            assert (k, a, u, x, y, z) == (bk, ba, bu, bx, by, bz), hex(p)
            if prm == bprm and not (BASE <= prm < BASE + 0x800000):
                ps = f"0x{prm:X}"
            else:
                assert bprm == prm + delta, f"param pointer at {p:#x} doesn't shift by the delta"
                ps = f"dat_{prm:08X}"
            out.append(f"    obj_spawn 0x{k:02X}, 0x{a:02X}, 0x{u:04X}, {fx(x)}, {fx(y)}, {fx(z)}, {ps}")
            p += 20
        assert p == end, f"block {off:#x} isn't a whole number of records"
        text = text.replace(line + "\n", "\n".join(out) + "\n", 1)
        path.write_text(text, encoding="utf-8", newline="\n")
        print(f"{seg} 0x{BASE + off:08X}: {len(out)} lines")


if __name__ == "__main__":
    main()
