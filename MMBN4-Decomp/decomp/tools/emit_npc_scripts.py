#!/usr/bin/env python3
"""Decode and emit RS+BM-identical NpcRunScript (dat_080A85D0) bytecode
blocks in code_04..code_06 into asm/code_04*.s, asm/code_05.s, asm/code_06.s.

Seeds come from the Real World (dat_08029C94) and Cyber World (dat_08029E14)
map NPC pointer tables plus `csc_b1_ptr 0x46, 0x02, dat_*` (NpcSpawnList).
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
    ("code_04",  0x0804F21C, 0x0805CF5C, 0x0C),
    ("code_04b", 0x0805CF5C, 0x080612C8, 0x18),
    ("code_04c", 0x080612C8, 0x0806765C, 0x14),
    ("code_04d", 0x0806765C, 0x080711E4, 0x0C),
    ("code_05",  0x080711E4, 0x08094758, 0x0C),
    ("code_06",  0x08094758, 0x080B01DE, 0x0C),
]

# op -> (size, ptr_offsets, branch_offsets, is_term)
NPC_SPEC: dict[int, tuple[int, list[int], list[int], bool]] = {
    0x00: (1, [], [], True),
    0x02: (5, [1], [1], True),
    0x03: (1, [], [], True),
    0x04: (7, [3], [3], False),
    0x05: (7, [3], [3], False),
    0x06: (3, [], [], False),
    0x07: (3, [], [], False),
    0x08: (1, [], [], False),
    0x09: (1, [], [], False),
    0x0A: (2, [], [], False),
    0x0B: (2, [], [], False),
    0x0C: (4, [], [], False),
    0x0D: (1, [], [], False),
    0x0E: (1, [], [], False),
    0x0F: (2, [], [], False),
    0x10: (2, [], [], False),
    0x11: (3, [], [], False),
    0x12: (1, [], [], False),
    0x13: (1, [], [], False),
    0x14: (7, [], [], False),
    0x15: (4, [], [], False),
    0x16: (2, [], [], False),
    0x17: (2, [], [], False),
    0x18: (2, [], [], False),
    0x19: (1, [], [], False),
    0x1A: (1, [], [], False),
    0x1B: (1, [], [], False),
    0x1C: (5, [1], [], False),
    0x1D: (2, [], [], False),
    0x1E: (2, [], [], False),
    0x1F: (1, [], [], False),
    0x20: (1, [], [], False),
    0x21: (1, [], [], False),
    0x22: (1, [], [], False),
    0x23: (1, [], [], False),
    0x24: (1, [], [], False),
    0x25: (3, [], [], False),
    0x26: (1, [], [], False),
    0x27: (1, [], [], False),
    0x28: (3, [], [], False),
    0x29: (3, [], [], False),
    0x2A: (2, [], [], False),
    0x2B: (3, [], [], False),
    0x2D: (3, [], [], False),
    0x2E: (2, [], [], False),
    0x2F: (4, [], [], False),
    0x30: (1, [], [], False),
    0x31: (2, [], [], False),
    0x32: (5, [1], [], False),
    0x33: (2, [], [], False),
    0x34: (1, [], [], True),
    0x35: (5, [1], [], False),
    0x36: (5, [1], [1], True),
    0x37: (9, [5], [], False),
    0x38: (8, [4], [4], True),
    0x39: (2, [], [], False),
    0x3A: (1, [], [], True),
    0x3B: (7, [3], [3], False),
    0x3C: (3, [], [], False),
    0x3D: (7, [3], [3], False),
    0x3E: (3, [], [], False),
    0x40: (1, [], [], False),
    0x41: (1, [], [], False),
    0x42: (7, [3], [3], False),
    0x43: (7, [3], [3], False),
    0x44: (6, [2], [2], False),
    0x45: (3, [], [], False),
    0x46: (2, [], [], False),
    0x47: (3, [], [], False),
    0x48: (1, [], [], False),
    0x49: (1, [], [], False),
    0x4A: (4, [], [], False),
    0x4B: (6, [2], [2], False),
}


def in_target(addr: int) -> tuple[str, int] | None:
    for name, s0, s1, d in TARGET_SEGS:
        if s0 <= addr < s1:
            return (name, d)
    return None


def format_u32(rv: int, bv: int) -> str | None:
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
    ro, bo = addr - BASE, addr + d - BASE
    op = rs[ro]
    if op not in NPC_SPEC or bm[bo] != op:
        return None
    sz, ptrs, _, _ = NPC_SPEC[op]
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
        return (1, "    npc_end")
    if op == 0x02:
        return (5, f"    npc_jump {p_strs[1]}")
    if op == 0x03:
        return (1, "    npc_hide")
    if op == 0x04:
        return (7, f"    npc_jump_if_flag_set 0x{h(1):04X}, {p_strs[3]}")
    if op == 0x05:
        return (7, f"    npc_jump_if_flag_clear 0x{h(1):04X}, {p_strs[3]}")
    if op == 0x06:
        return (3, f"    npc_set_flag 0x{h(1):04X}")
    if op == 0x07:
        return (3, f"    npc_clear_flag 0x{h(1):04X}")
    if op == 0x14:
        return (7, f"    npc_set_pos 0x{h(1):04X}, 0x{h(3):04X}, 0x{h(5):04X}")
    if op == 0x16:
        return (2, f"    npc_set_facing 0x{b(1):02X}")
    if op == 0x17:
        return (2, f"    npc_set_sprite 0x{b(1):02X}")
    if op == 0x18:
        return (2, f"    npc_set_talk 0x{b(1):02X}")
    if op == 0x34:
        return (1, "    npc_stop")
    if op == 0x36:
        return (5, f"    npc_call {p_strs[1]}")
    if op == 0x38:
        return (8, f"    npc_call_ex 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, {p_strs[4]}")
    if op == 0x3A:
        return (1, "    npc_ret")
    if op == 0x3B:
        return (7, f"    npc_jump_if_phase 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}")
    if op == 0x42:
        return (7, f"    npc_jump_if_tourney_eq 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}")
    if op == 0x43:
        return (7, f"    npc_jump_if_tourney_ne 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}")

    if not ptrs:
        if sz == 1:
            return (1, f"    npc_op 0x{op:02X}")
        args = ", ".join(f"0x{b(i):02X}" for i in range(1, sz))
        return (sz, f"    npc_b{sz - 1} 0x{op:02X}, {args}")

    if ptrs == [1] and sz == 5:
        return (5, f"    npc_ptr 0x{op:02X}, {p_strs[1]}")
    if ptrs == [2] and sz == 6:
        return (6, f"    npc_b1_ptr 0x{op:02X}, 0x{b(1):02X}, {p_strs[2]}")
    if ptrs == [3] and sz == 7:
        return (7, f"    npc_b2_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}")
    if ptrs == [5] and sz == 9:
        return (9, f"    npc_b4_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, 0x{b(4):02X}, {p_strs[5]}")
    return None


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()

    # Gather NPC script seeds from RW (0x08029C94) and CW (0x08029E14) NPC list tables
    list_ptrs: set[int] = set()
    for tbl_start, tbl_end in [(0x08029C94, 0x08029E14), (0x08029E14, 0x08029F94)]:
        for p in range(tbl_start, tbl_end, 4):
            area_ptr = struct.unpack_from("<I", rs, p - BASE)[0]
            if BASE <= area_ptr < BASE + 0x120000:
                for k in range(16):
                    mp = struct.unpack_from("<I", rs, area_ptr - BASE + k * 4)[0]
                    if in_target(mp):
                        list_ptrs.add(mp)

    # Also gather NpcSpawnList targets from csc_b1_ptr 0x46, 0x02, dat_*
    for sn in ("code_05", "code_06"):
        txt = (DECOMP / f"asm/{sn}.s").read_text()
        for m in re.finditer(r"csc_b1_ptr\s+0x46,\s*0x02,\s*dat_([0-9A-Fa-f]{8})", txt):
            a = int(m.group(1), 16)
            if in_target(a):
                list_ptrs.add(a)

    # Also find any 0x000000FF-terminated NPC pointer list right after an existing ptr table
    # or inside shared incrom in code_04..code_06
    for sn, s0, s1, d in TARGET_SEGS:
        txt = (DECOMP / f"asm/{sn}.s").read_text()
        for m in re.finditer(r"^\s*incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)", txt, re.M):
            g0 = BASE + int(m.group(1), 16)
            g1 = g0 + int(m.group(2), 16)
            # Check for 0x000000FF followed by a valid NPC script opcode (0x08/0x04/0x05/0x3B/0x42)
            for p in range((g0 + 3) & ~3, g1 - 4, 4):
                if struct.unpack_from("<I", rs, p - BASE)[0] == 0x000000FF:
                    nxt = rs[p + 4 - BASE]
                    if nxt in (0x04, 0x05, 0x08, 0x17, 0x3B, 0x42, 0x43):
                        # Walk backward to find the list start (up to 32 pointers)
                        q = p - 4
                        while q >= s0 and (p - q) <= 128:
                            v = struct.unpack_from("<I", rs, q - BASE)[0]
                            if not in_target(v):
                                break
                            q -= 4
                        if q + 4 < p:
                            list_ptrs.add(q + 4)
                        else:
                            # Even if pointers were already emitted above, p+4 is an NPC script!
                            if p + 4 < g1:
                                list_ptrs.add(p)

    seeds: set[int] = set()
    npc_lists_by_seg: dict[str, list[tuple[int, int, list[str]]]] = {s[0]: [] for s in TARGET_SEGS}
    for mp in sorted(list_ptrs):
        ti = in_target(mp)
        if not ti:
            continue
        sn, d = ti
        q = mp
        entries: list[str] = []
        list_ok = True
        for _ in range(33):
            rv = struct.unpack_from("<I", rs, q - BASE)[0]
            bv = struct.unpack_from("<I", bm, q + d - BASE)[0]
            if rv == 0x000000FF:
                if bv != 0x000000FF:
                    list_ok = False
                else:
                    entries.append("    .4byte 0x000000FF   @ npc list end")
                q += 4
                break
            if in_target(rv):
                seeds.add(rv)
                s = format_u32(rv, bv)
                if s is None:
                    list_ok = False
                else:
                    entries.append(f"    .4byte {s}")
            else:
                list_ok = False
                break
            q += 4
        if list_ok and entries and entries[-1].startswith("    .4byte 0x000000FF"):
            npc_lists_by_seg[sn].append((mp, q, entries))
            # Also seed the script immediately following 0x000000FF if it starts with a common NPC op
            if in_target(q) and rs[q - BASE] in (0x04, 0x05, 0x08, 0x17, 0x3B, 0x42, 0x43):
                seeds.add(q)

    # Collect shared incrom ranges
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
    queue = sorted(seeds)
    blocks_by_seg: dict[str, list[tuple[int, int, list[str]]]] = {s[0]: [] for s in TARGET_SEGS}

    while queue:
        start = queue.pop()
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
            op = rs[cur - BASE]
            if op not in NPC_SPEC:
                ok = False
                break
            sz, _, branches, is_term = NPC_SPEC[op]
            for bo in branches:
                tgt = struct.unpack_from("<I", rs, cur - BASE + bo)[0]
                if not (0x0804F21C <= tgt < 0x080B01DE):
                    ok = False
                    break
            if not ok:
                break
            for bo in branches:
                tgt = struct.unpack_from("<I", rs, cur - BASE + bo)[0]
                if tgt not in visited and tgt not in seen_starts:
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
        blocks = sorted(blocks_by_seg[seg_name] + npc_lists_by_seg[seg_name], key=lambda x: x[0])
        if not blocks:
            continue
        path = DECOMP / f"asm/{seg_name}.s"
        lines = path.read_text().splitlines()
        out: list[str] = []
        in_vb = 0
        seg_blocks = 0
        seg_bytes = 0
        has_inc = any("npc_macros.inc" in l for l in lines[:25])
        for line in lines:
            st = line.strip()
            if st.startswith(".if "):
                in_vb += 1
            elif st.startswith(".endif"):
                in_vb = max(0, in_vb - 1)
            if not has_inc and st == ".thumb":
                out.append(line)
                out.append('    .include "npc_macros.inc"')
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
                        out.append(f"    @ npc script 0x{b0:08X}..0x{b1:08X}")
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
        print(f"{seg_name:8s}: emitted {seg_blocks:4d} npc blocks ({seg_bytes:5d} bytes)")
        total_blocks += seg_blocks
        total_bytes += seg_bytes

    print(f"TOTAL emitted: {total_blocks} npc blocks, {total_bytes} bytes")


if __name__ == "__main__":
    main()
