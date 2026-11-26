#!/usr/bin/env python3
"""Emit 4-byte-aligned runs of >= 4 consecutive RS+BM-identical ROM pointers
found inside shared `incrom` gaps across code_04..code_06.

Every emitted `.4byte dat_08XXXXXX` is verified with merge_bm.bm_value() so
zero new `.if VERSION_BM` blocks are added and RS & BM stay 100% byte-identical.
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

import merge_bm

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000

TARGET_SEGS = [
    ("code_00",  0x0800019C, 0x0800FF6A, 0x00),
    ("code_01",  0x0800FF6A, 0x08020CAC, 0x00),
    ("code_02",  0x08020CAC, 0x08034FC8, 0x04),
    ("code_03",  0x08034FC8, 0x0804F21C, 0x08),
    ("code_04",  0x0804F21C, 0x0805CF5C, 0x0C),
    ("code_04b", 0x0805CF5C, 0x080612C8, 0x18),
    ("code_04c", 0x080612C8, 0x0806765C, 0x14),
    ("code_04d", 0x0806765C, 0x080711E4, 0x0C),
    ("code_05",  0x080711E4, 0x08094758, 0x0C),
    ("code_06",  0x08094758, 0x080B01DE, 0x0C),
    ("code_07",  0x080B01DE, 0x080D00B8, 0x0C),
    ("code_08",  0x080D00B8, 0x080F0012, 0x0C),
    ("code_09",  0x080F0012, 0x08109EF0, 0x0C),
    ("code_10",  0x08109EF0, 0x08113CB8, 0x14),
]


def is_valid_rom_ptr(rs: bytes, bm: bytes, addr: int, d: int) -> str | None:
    rv = struct.unpack_from("<I", rs, addr - BASE)[0]
    bv = struct.unpack_from("<I", bm, addr + d - BASE)[0]
    if not (BASE + 0x200 <= rv < BASE + 0x7FFFF0):
        return None
    sym = f"dat_{rv:08X}"
    if merge_bm.bm_value(sym) == bv:
        return sym
    return None


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    inc_re = re.compile(r"^(\s*)incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)(.*)$")

    total_tables = 0
    total_bytes = 0
    for seg_name, _, seg_end, d in TARGET_SEGS:
        path = DECOMP / f"asm/{seg_name}.s"
        lines = path.read_text().splitlines()
        out: list[str] = []
        in_vb = 0
        seg_tables = 0
        seg_bytes = 0

        for line in lines:
            st = line.strip()
            if st.startswith(".if "):
                in_vb += 1
            elif st.startswith(".endif"):
                in_vb = max(0, in_vb - 1)
            m = inc_re.match(line)
            if m and in_vb == 0:
                indent, off_s, sz_s, _ = m.groups()
                g_start = BASE + int(off_s, 16)
                g_end = g_start + int(sz_s, 16)
                scan_end = min(g_end, seg_end)
                # Find 4-byte aligned pointer runs (>= 4 pointers = 16 bytes)
                runs: list[tuple[int, int, list[str]]] = []
                p = (g_start + 3) & ~3
                while p + 16 <= scan_end:
                    syms: list[str] = []
                    q = p
                    while q + 4 <= scan_end:
                        s = is_valid_rom_ptr(rs, bm, q, d)
                        if s is None:
                            break
                        syms.append(s)
                        q += 4
                    if len(syms) >= 4:
                        runs.append((p, q, syms))
                        p = q
                    else:
                        p = q + 4
                if runs:
                    cur = g_start
                    for r0, r1, syms in runs:
                        if r0 > cur:
                            out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{r0 - cur:X}")
                        out.append(f"    @ ptr table 0x{r0:08X}..0x{r1:08X} ({len(syms)} entries)")
                        for s in syms:
                            out.append(f"    .4byte {s}")
                        cur = r1
                        seg_tables += 1
                        seg_bytes += (r1 - r0)
                    if cur < g_end:
                        out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{g_end - cur:X}")
                    continue
            out.append(line)

        if seg_tables:
            path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
        print(f"{seg_name:8s}: emitted {seg_tables:4d} ptr tables ({seg_bytes:5d} bytes)")
        total_tables += seg_tables
        total_bytes += seg_bytes

    print(f"TOTAL emitted: {total_tables} ptr tables, {total_bytes} bytes")


if __name__ == "__main__":
    main()
