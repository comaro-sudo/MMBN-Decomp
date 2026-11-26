#!/usr/bin/env python3
"""Decode and emit RS+BM-identical BN4_CUTSCENE_COMMANDS (dat_0802C140)
cutscene scripts in code_05 and code_06 into asm/code_05.s and asm/code_06.s.

Verifies every relocatable `dat_08XXXXXX` target with merge_bm.bm_value() so
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
    ("code_05", 0x080711E4, 0x08094758, 0x0C),
    ("code_06", 0x08094758, 0x080B01DE, 0x0C),
]

# op -> (size, ptr_offsets, branch_offsets, is_term)
CS_SPEC: dict[int, tuple[int | None, list[int], list[int], bool]] = {
    0x00: (1, [], [], True),
    0x01: (1, [], [], True),
    0x02: (3, [], [], False),
    0x03: (4, [], [], False),
    0x04: (2, [], [], False),
    0x05: (2, [], [], False),
    0x06: (1, [], [], False),
    0x07: (1, [], [], False),
    0x08: (1, [], [], False),
    0x09: (3, [], [], False),
    0x0A: (2, [], [], False),
    0x0B: (1, [], [], False),
    0x0C: (1, [], [], False),
    0x0D: (3, [], [], False),
    0x0E: (3, [], [], False),
    0x0F: (1, [], [], False),
    0x10: (1, [], [], True),
    0x11: (6, [2], [2], False),
    0x12: (2, [], [], False),
    0x13: (5, [1], [], False),
    0x14: (5, [1], [1], True),
    0x15: (7, [3], [3], False),
    0x16: (8, [4], [4], False),
    0x17: (8, [4], [4], False),
    0x18: (8, [4], [4], False),
    0x19: (8, [4], [4], False),
    0x1A: (None, [], [], False),
    0x1B: (7, [3], [3], False),
    0x1C: (8, [4], [4], False),
    0x1D: (10, [6], [6], False),
    0x1E: (6, [2], [2], False),
    0x1F: (6, [2], [2], False),
    0x20: (6, [2], [2], False),
    0x21: (6, [2], [2], False),
    0x22: (6, [2], [2], False),
    0x23: (6, [2], [2], False),
    0x24: (4, [], [], False),
    0x25: (3, [], [], False),
    0x26: (4, [], [], False),
    0x27: (4, [], [], False),
    0x28: (4, [], [], False),
    0x29: (4, [], [], False),
    0x2A: (5, [1], [], False),
    0x2B: (5, [1], [], False),
    0x2C: (6, [1], [], False),
    0x2D: (7, [1], [], False),
    0x2E: (9, [1, 5], [], False),
    0x2F: (3, [], [], False),
    0x30: (3, [], [], False),
    0x31: (4, [], [], False),
    0x32: (3, [], [], False),
    0x33: (5, [1], [], False),
    0x34: (5, [1], [], False),
    0x35: (2, [], [], False),
    0x36: (None, [], [], False),
    0x37: (1, [], [], False),
    0x38: (3, [], [], False),
    0x39: (None, [], [], False),
    0x3A: (5, [1], [], False),
    0x3B: (2, [], [], False),
    0x3C: (4, [], [], False),
    0x3D: (10, [], [], False),
    0x3E: (5, [], [], False),
    0x3F: (3, [], [], False),
    0x40: (5, [1], [], False),
    0x41: (None, [], [], False),
    0x42: (None, [], [], False),
    0x43: (4, [], [], False),
    0x44: (3, [], [], False),
    0x45: (None, [], [], False),
    0x46: (None, [], [], False),
    0x47: (5, [1], [], False),
    0x48: (None, [], [], False),
    0x49: (3, [], [], False),
    0x4A: (3, [], [], False),
    0x4B: (5, [], [], False),
    0x4C: (3, [], [], False),
    0x4D: (1, [], [], False),
    0x4E: (4, [], [], False),
    0x4F: (3, [], [], False),
    0x50: (None, [], [], False),
    0x51: (3, [], [], False),
    0x52: (2, [], [], False),
    0x53: (2, [], [], False),
    0x54: (4, [], [], False),
    0x55: (2, [], [], False),
    0x56: (13, [1, 5, 9], [], False),
    0x57: (1, [], [], False),
    0x58: (4, [], [], False),
    0x59: (2, [], [], False),
    0x5A: (1, [], [], False),
    0x5B: (1, [], [], False),
    0x5C: (1, [], [], False),
    0x5D: (1, [], [], False),
    0x5E: (2, [], [], False),
    0x5F: (1, [], [], False),
    0x60: (1, [], [], False),
    0x61: (1, [], [], False),
    0x62: (1, [], [], False),
    0x63: (1, [], [], False),
    0x64: (3, [], [], False),
    0x65: (4, [], [], False),
    0x66: (3, [], [], False),
}


def in_target(addr: int) -> tuple[str, int] | None:
    for name, s0, s1, d in TARGET_SEGS:
        if s0 <= addr < s1:
            return (name, d)
    return None


def decode_one(rom: bytes, addr: int):
    off = addr - BASE
    op = rom[off]
    if op not in CS_SPEC:
        return None
    sz, ptrs, branches, is_term = CS_SPEC[op]
    if op == 0x36:
        sz = 3 if rom[off + 1] == 0xFF else 2
    elif op == 0x41:
        b1 = rom[off + 1]
        sz = 5 if b1 == 0 else (2 if b1 == 1 else 6)
    elif op == 0x42:
        sz = 2 if rom[off + 1] == 1 else 4
    elif op == 0x45:
        sub = rom[off + 1] >> 4
        if sub in (0, 1):
            sz, ptrs = 13, [9]
        elif sub == 2:
            sz, ptrs = 14, [10]
        elif sub in (3, 4):
            sz, ptrs = 2, []
        else:
            return None
    elif op == 0x46:
        b1 = rom[off + 1]
        if b1 == 1:
            sz, ptrs = 3, []
        elif b1 in (3, 4, 5):
            sz, ptrs = 2, []
        else:
            sz, ptrs = 6, [2]
    elif op == 0x48:
        b1 = rom[off + 1]
        if b1 & 0x40:
            sz, ptrs = 2, []
        else:
            sz, ptrs = 7, ([] if (b1 & 0x80) else [3])
    elif op == 0x50:
        if rom[off + 1] == 0:
            sz, ptrs = 6, [2]
        else:
            sz, ptrs = 2, []
    elif sz is None:
        return None
    return (op, sz, ptrs, branches, is_term)


def format_u32(rv: int, bv: int) -> str | tuple[str, str] | None:
    if BASE <= rv < BASE + 0x800000:
        sym = f"dat_{rv:08X}"
        if merge_bm.bm_value(sym) == bv:
            return sym
        if rv == bv:
            return f"0x{rv:08X}"
    if rv == bv:
        return f"0x{rv:08X}"
    # Version-specific ROM data/compressed-archive pointers (0x08120000..0x08800000
    # or bit-31 compressed 0x88120000..0x88800000).
    if ((rv >> 24) == (bv >> 24)
            and 0x08120000 <= (rv & 0x7FFFFFFF) < 0x08800000
            and 0x08120000 <= (bv & 0x7FFFFFFF) < 0x08800000):
        return (f"0x{rv:08X}", f"0x{bv:08X}")
    return None


def format_instr(rs: bytes, bm: bytes, addr: int, d: int) -> tuple[int, str] | None:
    info = decode_one(rs, addr)
    if not info:
        return None
    op, sz, ptrs, _, _ = info
    ro, bo = addr - BASE, addr + d - BASE
    if bm[bo] != op:
        return None
    pbytes = {po + k for po in ptrs for k in range(4)}
    for i in range(sz):
        if i not in pbytes and rs[ro + i] != bm[bo + i]:
            return None

    p_strs: dict[int, str] = {}
    for po in ptrs:
        rv = struct.unpack_from("<I", rs, ro + po)[0]
        bv = struct.unpack_from("<I", bm, bo + po)[0]
        s = format_u32(rv, bv)
        if s is None:
            return None
        p_strs[po] = s

    b = lambda k: rs[ro + k]
    h = lambda k: struct.unpack_from("<H", rs, ro + k)[0]

    if op == 0x00:
        return (1, "    csc_end")
    if op == 0x01:
        return (1, "    csc_end_alt")
    if op == 0x10:
        return (1, "    csc_thread_end")
    if op == 0x11:
        if isinstance(p_strs[2], tuple):
            return None
        return (6, f"    csc_spawn 0x{b(1):02X}, {p_strs[2]}")
    if op == 0x14:
        if isinstance(p_strs[1], tuple):
            return None
        return (5, f"    csc_jump {p_strs[1]}")
    if op == 0x45:
        if sz == 2:
            return (2, f"    csc_b1 0x45, 0x{b(1):02X}")
        if any(isinstance(v, tuple) for v in p_strs.values()):
            return None
        if sz == 13:
            return (13, f"    csc_45_13 0x{b(1):02X}, 0x{b(2):02X}, 0x{h(3):04X}, 0x{h(5):04X}, 0x{h(7):04X}, {p_strs[9]}")
        if sz == 14:
            return (14, f"    csc_45_14 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, 0x{h(4):04X}, 0x{h(6):04X}, 0x{h(8):04X}, {p_strs[10]}")

    if not ptrs:
        if sz == 1:
            return (1, f"    csc_op 0x{op:02X}")
        args = ", ".join(f"0x{b(i):02X}" for i in range(1, sz))
        if sz in (2, 3, 4, 5, 6, 10):
            return (sz, f"    csc_b{sz - 1} 0x{op:02X}, {args}")
        # Fallback for 7-byte non-ptr (0x48 with 0x80 set)
        return (sz, f"    .byte 0x{op:02X}, {args}")

    if ptrs == [1]:
        if sz == 5:
            if isinstance(p_strs[1], tuple):
                rs_p, bm_p = p_strs[1]
                return (5, f"    csc_ptr_v 0x{op:02X}, {rs_p}, {bm_p}")
            return (5, f"    csc_ptr 0x{op:02X}, {p_strs[1]}")
        if isinstance(p_strs[1], tuple):
            return None
        if sz == 6:
            return (6, f"    csc_ptr_b1 0x{op:02X}, {p_strs[1]}, 0x{b(5):02X}")
        if sz == 7:
            return (7, f"    csc_ptr_b2 0x{op:02X}, {p_strs[1]}, 0x{b(5):02X}, 0x{b(6):02X}")
    if ptrs == [2] and sz == 6:
        if isinstance(p_strs[2], tuple):
            rs_p, bm_p = p_strs[2]
            return (6, f"    csc_b1_ptr_v 0x{op:02X}, 0x{b(1):02X}, {rs_p}, {bm_p}")
        return (6, f"    csc_b1_ptr 0x{op:02X}, 0x{b(1):02X}, {p_strs[2]}")
    if any(isinstance(v, tuple) for v in p_strs.values()):
        return None
    if ptrs == [3] and sz == 7:
        return (7, f"    csc_b2_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}")
    if ptrs == [4] and sz == 8:
        return (8, f"    csc_b3_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, {p_strs[4]}")
    if ptrs == [6] and sz == 10:
        return (10, f"    csc_b5_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, 0x{b(4):02X}, 0x{b(5):02X}, {p_strs[6]}")
    if ptrs == [1, 5] and sz == 9:
        return (9, f"    csc_ptr2 0x{op:02X}, {p_strs[1]}, {p_strs[5]}")
    if ptrs == [1, 5, 9] and sz == 13:
        return (13, f"    csc_ptr3 0x{op:02X}, {p_strs[1]}, {p_strs[5]}, {p_strs[9]}")
    return None


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()

    # Gather cutscene script seeds from ms_start_cutscene + all asm/*.s dat_ refs
    direct_seeds: set[int] = set()
    for s in ["code_04", "code_04b", "code_04c", "code_04d", "code_05"]:
        txt = (DECOMP / f"asm/{s}.s").read_text()
        for m in re.finditer(r"ms_start_cutscene\s+(?:dat_|0x)([0-9A-Fa-f]{8})", txt):
            a = int(m.group(1), 16)
            if in_target(a):
                direct_seeds.add(a)

    ref_seeds: set[int] = set()
    for p in sorted((DECOMP / "asm").glob("*.s")):
        txt = p.read_text()
        for m in re.finditer(r"\bdat_([0-9A-Fa-f]{8})\b", txt):
            a = int(m.group(1), 16)
            if in_target(a) and a not in direct_seeds:
                ref_seeds.add(a)

    # Collect shared incrom ranges in code_05 and code_06
    inc_re = re.compile(r"^(\s*)incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)(.*)$")
    shared_incroms: list[tuple[int, int, str, int]] = []
    for seg_name, _, _, d in TARGET_SEGS:
        in_vb = 0
        for line in (DECOMP / f"asm/{seg_name}.s").read_text().splitlines():
            st = line.strip()
            if st.startswith(".if "):
                in_vb += 1
            elif st.startswith(".endif"):
                in_vb = max(0, in_vb - 1)
            elif in_vb == 0:
                m = inc_re.match(line)
                if m:
                    s = BASE + int(m.group(2), 16)
                    shared_incroms.append((s, s + int(m.group(3), 16), seg_name, d))

    def in_shared_incrom(s: int, e: int) -> bool:
        return any(s >= r0 and e <= r1 for r0, r1, _, _ in shared_incroms)

    visited: set[int] = set()
    seen_starts: set[int] = set()
    queue = [(a, 1) for a in sorted(direct_seeds)] + [(a, 2) for a in sorted(ref_seeds)]
    blocks_by_seg: dict[str, list[tuple[int, int, list[str]]]] = {s[0]: [] for s in TARGET_SEGS}

    while queue:
        start, min_ops = queue.pop()
        if start in visited or start in seen_starts:
            continue
        seen_starts.add(start)
        ti = in_target(start)
        if not ti:
            continue
        seg_name, d = ti
        cur = start
        decoded: list[tuple[int, int, str]] = []
        ok = True
        all_fmt_ok = True
        block_seen: set[int] = set()
        while len(decoded) < 200:
            if cur in visited or cur in block_seen:
                break
            block_seen.add(cur)
            t2 = in_target(cur)
            if not t2 or t2[0] != seg_name:
                ok = False
                break
            info = decode_one(rs, cur)
            if not info:
                ok = False
                break
            op, sz, _, branches, is_term = info
            for bo in branches:
                tgt = struct.unpack_from("<I", rs, cur - BASE + bo)[0]
                if not (0x080711E4 <= tgt < 0x080B01DE):
                    ok = False
                    break
            if not ok:
                break
            for bo in branches:
                tgt = struct.unpack_from("<I", rs, cur - BASE + bo)[0]
                if tgt not in visited and tgt not in seen_starts:
                    queue.append((tgt, min_ops))
            fmt = format_instr(rs, bm, cur, d)
            if fmt is None:
                all_fmt_ok = False
            elif all_fmt_ok:
                decoded.append((cur, fmt[0], fmt[1]))
            cur += sz
            if is_term:
                break

        if ok and all_fmt_ok and len(decoded) >= min_ops:
            b0 = decoded[0][0]
            b1 = decoded[-1][0] + decoded[-1][1]
            if in_shared_incrom(b0, b1):
                for a, sz, _ in decoded:
                    for x in range(a, a + sz):
                        visited.add(x)
                blocks_by_seg[seg_name].append((b0, b1, [x[2] for x in decoded]))

    total_blocks = 0
    total_bytes = 0
    for seg_name, _, _, _ in TARGET_SEGS:
        blocks = sorted(blocks_by_seg[seg_name], key=lambda x: x[0])
        if not blocks:
            continue
        path = DECOMP / f"asm/{seg_name}.s"
        lines = path.read_text().splitlines()
        out: list[str] = []
        in_vb = 0
        seg_blocks = 0
        seg_bytes = 0
        has_inc = any("cutscene_macros.inc" in l for l in lines[:20])
        for line in lines:
            st = line.strip()
            if st.startswith(".if "):
                in_vb += 1
            elif st.startswith(".endif"):
                in_vb = max(0, in_vb - 1)
            if not has_inc and st == ".thumb":
                out.append(line)
                out.append('    .include "cutscene_macros.inc"')
                has_inc = True
                continue
            m = inc_re.match(line)
            if m and in_vb == 0:
                indent, off_s, sz_s, _ = m.groups()
                g_start = BASE + int(off_s, 16)
                g_end = g_start + int(sz_s, 16)
                hits = [b for b in blocks if b[0] >= g_start and b[1] <= g_end]
                if hits:
                    cur = g_start
                    for b0, b1, b_lines in hits:
                        if b0 < cur:
                            continue
                        if b0 > cur:
                            out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{b0 - cur:X}")
                        out.append(f"    @ cutscene script 0x{b0:08X}..0x{b1:08X}")
                        out.extend(b_lines)
                        cur = b1
                        seg_blocks += 1
                        seg_bytes += (b1 - b0)
                    if cur < g_end:
                        out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{g_end - cur:X}")
                    continue
            out.append(line)
        if seg_blocks:
            path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
        print(f"{seg_name:8s}: emitted {seg_blocks:4d} cutscene blocks ({seg_bytes:5d} bytes)")
        total_blocks += seg_blocks
        total_bytes += seg_bytes

    print(f"TOTAL emitted: {total_blocks} cutscene blocks, {total_bytes} bytes")


if __name__ == "__main__":
    main()
