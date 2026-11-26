#!/usr/bin/env python3
"""Emit structured pointer, DMA/VRAM transfer, and animated tile/palette frame
descriptor tables from remaining shared `incrom` regions in code_00..code_10.

Uses `merge_bm.bm_value()` to emit symbolic `sub_`/`dat_` labels wherever
RS and BM match the segment delta, and clean single-line `*_v` macros from
`include/desc_macros.inc` where RS and BM point to genuinely version-specific
compressed data/text/graphics archives.
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

import merge_bm

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000
AUDIO_TAIL_OFF = 0x113CB6  # code_10 audio/build-timestamp tail stays incrom

SEGS = [
    "code_00", "code_01", "code_02", "code_03",
    "code_04", "code_04b", "code_04c", "code_04d",
    "code_05", "code_06",
    "code_07", "code_08", "code_09", "code_10",
]


def is_rom(w: int) -> bool:
    return 0x08000000 <= (w & 0x7FFFFFFF) < 0x08800000


def is_ram(w: int) -> bool:
    return (
        (0x02000000 <= w < 0x02040000)
        or (0x03000000 <= w < 0x03008000)
        or (0x05000000 <= w < 0x05000400)
        or (0x06000000 <= w < 0x06018000)
    )


def load_known_funcs() -> set[int]:
    funcs: set[int] = set()
    f = DECOMP / "symbols/functions_rs.txt"
    if f.exists():
        for line in f.read_text().splitlines():
            s = line.strip()
            if s and not s.startswith("#"):
                funcs.add(int(s.split()[0], 16))
    return funcs


def fmt_rom(rv: int, bv: int, funcs: set[int]) -> str | tuple[str, str]:
    if (rv & 0x80000000) == 0 and BASE <= rv < BASE + 0x800000:
        if (rv & 1) and ((rv - 1) in funcs):
            sym = f"sub_{rv - 1:08X}"
            if merge_bm.bm_value(sym) == (bv - 1):
                return sym
        sym = f"dat_{rv:08X}"
        if merge_bm.bm_value(sym) == bv:
            return sym
    if rv == bv:
        return f"0x{rv:08X}"
    return (f"0x{rv:08X}", f"0x{bv:08X}")


def scan_incrom(rs: bytes, bm: bytes, off: int, sz: int, delta: int, funcs: set[int]):
    """Return list of (start_off, end_off, lines) runs inside [off, off+sz)."""
    if off % 4 != 0 or sz < 8:
        return []
    pos, end = off, off + sz
    runs: list[tuple[int, int, list[str]]] = []
    while pos + 8 <= end:
        w0 = struct.unpack_from("<I", rs, pos)[0]
        b0 = struct.unpack_from("<I", bm, pos + delta)[0]

        # 1. Pure ROM pointer table (>= 2 consecutive valid ROM ptrs)
        if is_rom(w0) and is_rom(b0) and (w0 >> 24) == (b0 >> 24):
            p = pos
            out: list[str] = []
            while p + 4 <= end:
                ar = struct.unpack_from("<I", rs, p)[0]
                ab = struct.unpack_from("<I", bm, p + delta)[0]
                if is_rom(ar) and is_rom(ab) and (ar >> 24) == (ab >> 24):
                    f = fmt_rom(ar, ab, funcs)
                    if isinstance(f, tuple):
                        out.append(f"    ptr_v {f[0]}, {f[1]}")
                    else:
                        out.append(f"    .4byte {f}")
                    p += 4
                else:
                    break
            if len(out) >= 2:
                runs.append((pos, p, out))
                pos = p
                continue

        # 2. Pure RAM pointer table (>= 3 consecutive identical RAM ptrs)
        if is_ram(w0) and w0 == b0:
            p = pos
            out = []
            while p + 4 <= end:
                ar = struct.unpack_from("<I", rs, p)[0]
                ab = struct.unpack_from("<I", bm, p + delta)[0]
                if is_ram(ar) and ar == ab:
                    out.append(f"    .4byte 0x{ar:08X}")
                    p += 4
                else:
                    break
            if len(out) >= 3:
                runs.append((pos, p, out))
                pos = p
                continue

        # 3. 12-byte DMA/VRAM transfer table {ram_dst, ctrl, rom_src} (>= 2 entries)
        if pos + 24 <= end and is_ram(w0) and w0 == b0:
            p = pos
            out = []
            while p + 12 <= end:
                a0, a1, a2 = struct.unpack_from("<3I", rs, p)
                c0, c1, c2 = struct.unpack_from("<3I", bm, p + delta)
                if is_ram(a0) and a0 == c0 and a1 == c1 and is_rom(a2) and is_rom(c2) and (a2 >> 24) == (c2 >> 24):
                    f = fmt_rom(a2, c2, funcs)
                    if isinstance(f, tuple):
                        out.append(f"    dma_desc_v 0x{a0:08X}, 0x{a1:08X}, {f[0]}, {f[1]}")
                    else:
                        out.append(f"    dma_desc 0x{a0:08X}, 0x{a1:08X}, {f}")
                    p += 12
                else:
                    break
            if len(out) >= 2:
                runs.append((pos, p, out))
                pos = p
                continue

        # 4. 8-byte {u32 val, rom_ptr} animated tile/palette stream (>= 3 entries)
        if pos + 24 <= end:
            p = pos
            out = []
            while p + 8 <= end:
                a0, a1 = struct.unpack_from("<2I", rs, p)
                c0, c1 = struct.unpack_from("<2I", bm, p + delta)
                if a0 == c0 and not is_rom(a0) and is_rom(a1) and is_rom(c1) and (a1 >> 24) == (c1 >> 24):
                    f = fmt_rom(a1, c1, funcs)
                    if isinstance(f, tuple):
                        out.append(f"    anim_frame_v 0x{a0:08X}, {f[0]}, {f[1]}")
                    else:
                        out.append(f"    anim_frame 0x{a0:08X}, {f}")
                    p += 8
                else:
                    break
            if len(out) >= 3:
                runs.append((pos, p, out))
                pos = p
                continue

        pos += 4

    return runs


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    funcs = load_known_funcs()
    inc_re = re.compile(r"^(\s*)incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)(.*)$")

    tot_tables = 0
    tot_bytes = 0
    for seg in SEGS:
        path = DECOMP / f"asm/{seg}.s"
        lines = path.read_text().splitlines()
        dm = re.search(r"SEG_BM_DELTA,\s*(-?0x[0-9a-fA-F]+|\d+)", "\n".join(lines[:20]))
        delta = int(dm.group(1), 0) if dm else 0
        has_inc = any("desc_macros.inc" in l for l in lines[:25])

        out_lines: list[str] = []
        in_vb = 0
        seg_tables = 0
        seg_bytes = 0
        need_inc = False

        for line in lines:
            st = line.strip()
            if st.startswith(".if "):
                in_vb += 1
            elif st.startswith(".endif"):
                in_vb = max(0, in_vb - 1)
            m = inc_re.match(line)
            if m and in_vb == 0:
                indent, off_s, sz_s, _ = m.groups()
                off = int(off_s, 16)
                sz = int(sz_s, 16)
                if seg == "code_10" and off >= AUDIO_TAIL_OFF:
                    out_lines.append(line)
                    continue
                runs = scan_incrom(rs, bm, off, sz, delta, funcs)
                if runs:
                    cur = off
                    end = off + sz
                    for r0, r1, rlines in runs:
                        if r0 > cur:
                            out_lines.append(f"{indent}incrom 0x{cur:X}, 0x{r0 - cur:X}")
                        out_lines.extend(rlines)
                        cur = r1
                        seg_tables += 1
                        seg_bytes += (r1 - r0)
                        if any("_v " in l or "dma_desc" in l or "anim_frame" in l for l in rlines):
                            need_inc = True
                    if cur < end:
                        out_lines.append(f"{indent}incrom 0x{cur:X}, 0x{end - cur:X}")
                    continue
            out_lines.append(line)

        if seg_tables:
            if need_inc and not has_inc:
                for i, l in enumerate(out_lines[:25]):
                    if l.strip() == ".thumb":
                        out_lines.insert(i + 1, '    .include "desc_macros.inc"')
                        break
            path.write_text("\n".join(out_lines) + "\n", encoding="utf-8", newline="\n")
        print(f"{seg:8s}: emitted {seg_tables:4d} desc tables ({seg_bytes:5d} bytes)")
        tot_tables += seg_tables
        tot_bytes += seg_bytes

    print(f"TOTAL emitted: {tot_tables} desc tables, {tot_bytes} bytes")


if __name__ == "__main__":
    main()
