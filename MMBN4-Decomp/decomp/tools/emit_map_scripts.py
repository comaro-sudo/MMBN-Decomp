#!/usr/bin/env python3
"""Decode and emit RS+BM-identical map_scripts_commands (dat_0802A7D0) action
bytecode bodies in code_04..code_05 into asm/code_04*.s and asm/code_05.s.

Uses merge_bm.bm_value() to verify that every emitted relocatable symbol
(`dat_08XXXXXX`) resolves to the exact Blue Moon ROM address, guaranteeing
zero new `.if VERSION_BM` blocks and 100% byte-identical RS & BM ROMs.
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

import merge_bm

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000

TARGET_SEGS = [
    ("code_04",  0x0804F21C, 0x0805CF5C, 0x0C),
    ("code_04b", 0x0805CF5C, 0x080612C8, 0x18),
    ("code_04c", 0x080612C8, 0x0806765C, 0x14),
    ("code_04d", 0x0806765C, 0x080711E4, 0x0C),
    ("code_05",  0x080711E4, 0x08094758, 0x0C),
]

# op -> (size, ptr_offsets, branch_offsets, is_term)
OP_SPEC: dict[int, tuple[int | None, list[int], list[int], bool]] = {
    0x00: (1, [], [], True),
    0x01: (5, [1], [1], True),
    0x02: (7, [3], [3], False),
    0x03: (8, [4], [4], False),
    0x04: (8, [4], [4], False),
    0x05: (8, [4], [4], False),
    0x06: (8, [4], [4], False),
    0x07: (None, [], [], False),
    0x08: (8, [4], [4], False),
    0x09: (10, [6], [6], False),
    0x0A: (6, [2], [2], False),
    0x0B: (6, [2], [2], False),
    0x0C: (6, [2], [2], False),
    0x0D: (6, [2], [2], False),
    0x0E: (6, [2], [2], False),
    0x0F: (6, [2], [2], False),
    0x10: (6, [2], [2], False),
    0x11: (6, [2], [2], False),
    0x12: (7, [3], [3], False),
    0x13: (7, [3], [3], False),
    0x14: (7, [3], [3], False),
    0x15: (7, [3], [3], False),
    0x16: (7, [3], [3], False),
    0x17: (5, [1], [], False),
    0x18: (5, [1], [], False),
    0x19: (5, [1], [], False),
    0x1A: (4, [], [], False),
    0x1B: (3, [], [], False),
    0x1C: (4, [], [], False),
    0x1D: (4, [], [], False),
    0x1E: (4, [], [], False),
    0x1F: (4, [], [], False),
    0x20: (5, [1], [], False),
    0x21: (5, [1], [], False),
    0x22: (9, [1, 5], [], False),
    0x23: (9, [1, 5], [], False),
    0x24: (6, [1], [], False),
    0x25: (7, [1], [], False),
    0x26: (9, [1, 5], [], False),
    0x27: (3, [], [], False),
    0x28: (3, [], [], False),
    0x29: (5, [1], [], False),
    0x2A: (5, [1], [], False),
    0x2B: (2, [], [], False),
    0x2C: (3, [], [], False),
    0x2D: (3, [], [], False),
    0x2E: (5, [], [], False),
    0x2F: (3, [], [], False),
    0x30: (1, [], [], False),
    0x31: (4, [], [], False),
    0x32: (3, [], [], False),
    0x33: (2, [], [], False),
    0x34: (13, [1, 5, 9], [], False),
    0x35: (1, [], [], False),
    0x36: (None, [], [], False),
    0x37: (2, [], [], False),
    0x38: (1, [], [], False),
    0x39: (1, [], [], False),
    0x3A: (1, [], [], False),
    0x3B: (1, [], [], False),
    0x3C: (1, [], [], False),
    0x3D: (None, [], [], False),
    0x3E: (4, [], [], False),
    0x3F: (3, [], [], False),
}


def in_target(addr: int) -> tuple[str, int] | None:
    for name, s0, s1, d in TARGET_SEGS:
        if s0 <= addr < s1:
            return (name, d)
    return None


def decode_one(rom: bytes, addr: int):
    off = addr - BASE
    op = rom[off]
    if op not in OP_SPEC:
        return None
    sz, ptrs, branches, is_term = OP_SPEC[op]
    if op == 0x36:
        b1 = rom[off + 1]
        if b1 == 1:
            sz, ptrs, branches = 2, [], []
        else:
            sz, ptrs, branches = 6, [2], [2]
    elif op == 0x3D:
        b1 = rom[off + 1]
        if b1 == 1:
            sz, ptrs, branches = 3, [], []
        else:
            sz, ptrs, branches = 6, [2], []
    elif op == 0x07:
        return None  # rare variable-length memory compare; skip
    return (op, sz, ptrs, branches, is_term)


def format_u32_operand(rv: int, bv: int) -> str | None:
    """Return a relocatable symbol (`dat_08XXXXXX`) or literal (`0x...`) that
    assembles identically to `rv` in RS and `bv` in BM, or None if incompatible."""
    if BASE <= rv < BASE + 0x800000:
        sym = f"dat_{rv:08X}"
        if merge_bm.bm_value(sym) == bv:
            return sym
        if rv == bv:
            return f"0x{rv:08X}"
        return None
    if rv == bv:
        return f"0x{rv:08X}"
    return None


def format_instr(rs: bytes, bm: bytes, addr: int, d: int) -> tuple[int, str] | None:
    info = decode_one(rs, addr)
    if not info:
        return None
    op, sz, ptrs, _, _ = info
    ro, bo = addr - BASE, addr + d - BASE
    if bm[bo] != op:
        return None
    ptr_bytes = {po + k for po in ptrs for k in range(4)}
    for i in range(sz):
        if i not in ptr_bytes and rs[ro + i] != bm[bo + i]:
            return None

    # Format all 32-bit pointer/value operands
    p_strs: dict[int, str] = {}
    for po in ptrs:
        rv = struct.unpack_from("<I", rs, ro + po)[0]
        bv = struct.unpack_from("<I", bm, bo + po)[0]
        s = format_u32_operand(rv, bv)
        if s is None:
            return None
        p_strs[po] = s

    b = lambda k: rs[ro + k]
    h = lambda k: struct.unpack_from("<H", rs, ro + k)[0]

    if op == 0x00:
        line = "    ms_end"
    elif op == 0x01:
        line = f"    ms_jump {p_strs[1]}"
    elif op == 0x02:
        line = f"    cs_range 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}"
    elif op == 0x03:
        line = f"    ms_jump_if_flag_set 0x{b(1):02X}, 0x{h(2):04X}, {p_strs[4]}"
    elif op == 0x04:
        line = f"    ms_jump_if_flag_clear 0x{b(1):02X}, 0x{h(2):04X}, {p_strs[4]}"
    elif op == 0x05:
        line = f"    ms_jump_if_flag2_set 0x{b(1):02X}, 0x{h(2):04X}, {p_strs[4]}"
    elif op == 0x06:
        line = f"    ms_jump_if_flag2_clear 0x{b(1):02X}, 0x{h(2):04X}, {p_strs[4]}"
    elif op == 0x08:
        line = f"    ms_jump_if_item 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, {p_strs[4]}"
    elif op == 0x09:
        line = f"    ms_jump_if_chip 0x{h(1):04X}, 0x{b(3):02X}, 0x{b(4):02X}, 0x{b(5):02X}, {p_strs[6]}"
    elif 0x0A <= op <= 0x11:
        line = f"    ms_jump_if_u8 0x{op:02X}, 0x{b(1):02X}, {p_strs[2]}"
    elif 0x12 <= op <= 0x15:
        line = f"    ms_jump_if_u16 0x{op:02X}, 0x{h(1):04X}, {p_strs[3]}"
    elif op == 0x16:
        line = f"    ms_jump_if_u8_pair 0x16, 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}"
    elif op in (0x17, 0x18, 0x19, 0x20, 0x21, 0x29, 0x2A):
        line = f"    ms_ptr 0x{op:02X}, {p_strs[1]}"
    elif op in (0x1A, 0x31):
        line = f"    ms_u8x3 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}"
    elif op in (0x1B, 0x27, 0x28, 0x2F, 0x32):
        line = f"    ms_u8x2 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}"
    elif op == 0x1C:
        line = f"    ms_set_flag 0x{b(1):02X}, 0x{h(2):04X}"
    elif op == 0x1D:
        line = f"    ms_clear_flag 0x{b(1):02X}, 0x{h(2):04X}"
    elif op == 0x1E:
        line = f"    ms_set_flag2 0x{b(1):02X}, 0x{h(2):04X}"
    elif op == 0x1F:
        line = f"    ms_clear_flag2 0x{b(1):02X}, 0x{h(2):04X}"
    elif op == 0x22:
        line = f"    ms_set_ptr_pair {p_strs[1]}, {p_strs[5]}"
    elif op == 0x23:
        line = f"    ms_start_cutscene {p_strs[1]}, {p_strs[5]}"
    elif op == 0x24:
        line = f"    ms_write_u8 {p_strs[1]}, 0x{b(5):02X}"
    elif op == 0x25:
        line = f"    ms_write_u16 {p_strs[1]}, 0x{h(5):04X}"
    elif op == 0x26:
        line = f"    ms_write_u32 {p_strs[1]}, {p_strs[5]}"
    elif op in (0x2B, 0x33, 0x37):
        line = f"    ms_u8 0x{op:02X}, 0x{b(1):02X}"
    elif op in (0x2C, 0x2D, 0x3F):
        line = f"    ms_u16 0x{op:02X}, 0x{h(1):04X}"
    elif op == 0x2E:
        line = f"    ms_u16_u8x2 0x2E, 0x{h(1):04X}, 0x{b(3):02X}, 0x{b(4):02X}"
    elif op in (0x30, 0x35, 0x38, 0x39, 0x3A, 0x3B, 0x3C):
        line = f"    ms_op 0x{op:02X}"
    elif op == 0x34:
        line = f"    ms_ptr3 0x34, {p_strs[1]}, {p_strs[5]}, {p_strs[9]}"
    elif op == 0x36:
        if b(1) == 1:
            line = "    ms_clear_cont"
        else:
            line = f"    ms_set_cont 0x{b(1):02X}, {p_strs[2]}"
    elif op == 0x3D:
        if b(1) == 1:
            line = f"    ms_3d_u8 0x{b(2):02X}"
        else:
            line = f"    ms_3d_ptr 0x{b(1):02X}, {p_strs[2]}"
    elif op == 0x3E:
        line = f"    ms_u16_u8 0x3E, 0x{h(1):04X}, 0x{b(3):02X}"
    else:
        return None
    return (sz, line)


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()

    # Gather seeds from cs_range/cs_jump, ms_* branches, and .word dat_ references
    seeds: set[int] = set()
    for seg_name, _, _, _ in TARGET_SEGS:
        txt = (DECOMP / "asm" / f"{seg_name}.s").read_text()
        for m in re.finditer(r"\b(?:cs_|ms_)\w+\b.*?\b(?:dat_|unk_)([0-9A-Fa-f]{8})", txt):
            a = int(m.group(1), 16)
            if in_target(a):
                seeds.add(a)
        for m in re.finditer(r"\bcs_(?:range|jump)\b.*?0x([0-9A-Fa-f]{8})", txt):
            a = int(m.group(1), 16)
            if in_target(a):
                seeds.add(a)
        for m in re.finditer(r"\.word\s+(?:dat_|unk_)([0-9A-Fa-f]{8})", txt):
            a = int(m.group(1), 16)
            if in_target(a):
                seeds.add(a)

    # Collect shared incrom ranges per segment
    inc_re = re.compile(r"^(\s*)incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)(.*)$")
    shared_incroms: list[tuple[int, int, str, int]] = []
    for seg_name, _, _, d in TARGET_SEGS:
        in_vb = 0
        for line in (DECOMP / "asm" / f"{seg_name}.s").read_text().splitlines():
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

    def in_shared_incrom(s: int, e: int) -> tuple[str, int] | None:
        for r0, r1, sn, d in shared_incroms:
            if s >= r0 and e <= r1:
                return (sn, d)
        return None

    # Trace reachable blocks
    visited: set[int] = set()
    queue = sorted(seeds)
    blocks_by_seg: dict[str, list[tuple[int, int, list[str]]]] = {s[0]: [] for s in TARGET_SEGS}

    while queue:
        start = queue.pop()
        if start in visited:
            continue
        tinfo = in_target(start)
        if not tinfo:
            continue
        seg_name, d = tinfo
        cur = start
        decoded: list[tuple[int, int, str]] = []
        ok = True
        all_fmt_ok = True
        while True:
            if cur in visited:
                break
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
                if not (0x0804F21C <= tgt < 0x08094758):
                    ok = False
                    break
            if not ok:
                break
            # Always follow branch targets even if this block has a version-specific op
            for bo in branches:
                tgt = struct.unpack_from("<I", rs, cur - BASE + bo)[0]
                if tgt not in visited:
                    queue.append(tgt)
            fmt = format_instr(rs, bm, cur, d)
            if fmt is None:
                all_fmt_ok = False
            elif all_fmt_ok:
                decoded.append((cur, fmt[0], fmt[1]))
            cur += sz
            if is_term:
                break

        if ok and all_fmt_ok and decoded:
            b_start = decoded[0][0]
            b_end = decoded[-1][0] + decoded[-1][1]
            if in_shared_incrom(b_start, b_end):
                for a, sz, _ in decoded:
                    for x in range(a, a + sz):
                        visited.add(x)
                blocks_by_seg[seg_name].append((b_start, b_end, [x[2] for x in decoded]))

    # Emit into asm/code_04*.s and asm/code_05.s
    total_blocks = 0
    total_bytes = 0
    for seg_name, _, _, _ in TARGET_SEGS:
        blocks = sorted(blocks_by_seg[seg_name], key=lambda x: x[0])
        if not blocks:
            continue
        path = DECOMP / "asm" / f"{seg_name}.s"
        lines = path.read_text().splitlines()
        out: list[str] = []
        in_vb = 0
        seg_blocks = 0
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
                hits = [b for b in blocks if b[0] >= g_start and b[1] <= g_end]
                if hits:
                    cur = g_start
                    for b_start, b_end, b_lines in hits:
                        if b_start < cur:
                            continue
                        if b_start > cur:
                            out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{b_start - cur:X}")
                        out.append(f"    @ map script 0x{b_start:08X}..0x{b_end:08X}")
                        out.extend(b_lines)
                        cur = b_end
                        seg_blocks += 1
                        seg_bytes += (b_end - b_start)
                    if cur < g_end:
                        out.append(f"{indent}incrom 0x{cur - BASE:X}, 0x{g_end - cur:X}")
                    continue
            out.append(line)
        if seg_blocks:
            path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
        print(f"{seg_name:8s}: emitted {seg_blocks:4d} script blocks ({seg_bytes:5d} bytes)")
        total_blocks += seg_blocks
        total_bytes += seg_bytes

    print(f"TOTAL emitted: {total_blocks} script blocks, {total_bytes} bytes")


if __name__ == "__main__":
    main()
