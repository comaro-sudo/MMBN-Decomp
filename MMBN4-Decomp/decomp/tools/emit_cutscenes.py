#!/usr/bin/env python3
"""Integrate decoded cutscene/map-init story-progress dispatch tables from
code_04..code_05 directly into asm/code_04*.s and asm/code_05.s using
include/cutscene_macros.inc (cs_range / cs_jump).

Only replaces ranges that are strictly inside a shared (non-VERSION_BM)
`incrom` line AND whose RS and BM dispatch opcodes are 100% structurally
identical after segment BM-delta rebasing — guaranteeing zero new version
blocks and 100% byte-identical RS & BM ROMs.
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

from disasm_cutscene import decode_dispatch, load_rom

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000

SEG_INFO = [
    ("code_04",  0x0804F21C, 0x0805CF5C, 0x0C),
    ("code_04b", 0x0805CF5C, 0x080612C8, 0x18),
    ("code_04c", 0x080612C8, 0x0806765C, 0x14),
    ("code_04d", 0x0806765C, 0x080711E4, 0x0C),
    ("code_05",  0x080711E4, 0x08094758, 0x0C),
]


def seg_delta(addr: int) -> int | None:
    for _, s0, s1, d in SEG_INFO:
        if s0 <= addr < s1:
            return d
    return None


def collect_tables(rs: bytes, bm: bytes) -> dict[str, list[tuple[int, int, list[str]]]]:
    """Scan all of code_04..code_05 for RS+BM-identical story-progress dispatch
    chains (0x02 range ops ending in 0x01 unconditional jump, >= 2 ops)."""
    by_seg: dict[str, list[tuple[int, int, list[str]]]] = {s[0]: [] for s in SEG_INFO}

    for seg_name, s0, s1, d in SEG_INFO:
        cur = s0
        while cur < s1 - 12:
            if rs[cur - BASE] == 0x02:
                rs_lines, rs_end = decode_dispatch(rs, cur)
                if len(rs_lines) >= 2 and rs_lines[-1].strip().startswith("cs_jump"):
                    bm_lines, bm_end = decode_dispatch(bm, cur + d)
                    if len(bm_lines) == len(rs_lines) and (bm_end - (cur + d)) == (rs_end - cur):
                        ok = True
                        p = cur
                        for _ in rs_lines:
                            ro = p - BASE
                            bo = p + d - BASE
                            op = rs[ro]
                            if bm[bo] != op:
                                ok = False
                                break
                            if op == 0x02:
                                if rs[ro + 1:ro + 3] != bm[bo + 1:bo + 3]:
                                    ok = False
                                    break
                                rt = struct.unpack_from("<I", rs, ro + 3)[0]
                                bt = struct.unpack_from("<I", bm, bo + 3)[0]
                                td = seg_delta(rt)
                                if td is None or bt != rt + td:
                                    ok = False
                                    break
                                p += 7
                            else:
                                rt = struct.unpack_from("<I", rs, ro + 1)[0]
                                bt = struct.unpack_from("<I", bm, bo + 1)[0]
                                td = seg_delta(rt)
                                if td is None or bt != rt + td:
                                    ok = False
                                    break
                                p += 5
                        if ok:
                            by_seg[seg_name].append((cur, rs_end, rs_lines))
                            cur = rs_end
                            continue
            cur += 1
    return by_seg


def main() -> None:
    rs, bm = load_rom("rs"), load_rom("bm")
    by_seg = collect_tables(rs, bm)
    inc_re = re.compile(r"^(\s*)incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)(.*)$")

    total_tables = 0
    total_bytes = 0
    for seg_name, _, _, _ in SEG_INFO:
        tables = by_seg[seg_name]
        if not tables:
            continue
        path = DECOMP / "asm" / f"{seg_name}.s"
        lines = path.read_text().splitlines()
        out: list[str] = []
        in_vblock = 0
        seg_tables = 0
        seg_bytes = 0
        has_include = any('cutscene_macros.inc' in l for l in lines[:20])

        for line in lines:
            st = line.strip()
            if st.startswith(".if "):
                in_vblock += 1
            elif st.startswith(".endif"):
                in_vblock = max(0, in_vblock - 1)

            if not has_include and st == ".thumb":
                out.append(line)
                out.append('    .include "cutscene_macros.inc"')
                has_include = True
                continue

            m = inc_re.match(line)
            if m and in_vblock == 0:
                indent, off_s, sz_s, tail = m.groups()
                g_start = BASE + int(off_s, 16)
                g_end = g_start + int(sz_s, 16)
                hits = [t for t in tables if t[0] >= g_start and t[1] <= g_end]
                if hits:
                    cur = g_start
                    for t_start, t_end, t_lines in hits:
                        if t_start < cur:
                            continue  # skip overlap
                        if t_start > cur:
                            out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{t_start - cur:X}")
                        out.append(f"    @ map-init dispatch 0x{t_start:08X}..0x{t_end:08X}")
                        out.extend(t_lines)
                        cur = t_end
                        seg_tables += 1
                        seg_bytes += (t_end - t_start)
                    if cur < g_end:
                        out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{g_end - cur:X}")
                    continue

            out.append(line)

        if seg_tables:
            path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
        print(f"{seg_name:8s}: emitted {seg_tables:3d} tables ({seg_bytes:4d} bytes)")
        total_tables += seg_tables
        total_bytes += seg_bytes

    print(f"TOTAL emitted: {total_tables} tables, {total_bytes} bytes")


if __name__ == "__main__":
    main()
