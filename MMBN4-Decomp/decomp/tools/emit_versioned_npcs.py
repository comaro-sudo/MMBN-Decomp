#!/usr/bin/env python3
"""Decode version-specific `.if VERSION_BM` / `.else` NPC list + NpcRunScript
blocks in code_04b.s (0x60264) and code_04c.s (0x6544C).

Uses `NPC_SPEC` from `emit_npc_scripts.py` and `include/npc_macros.inc` macros.
Because RS and BM have different lengths inside these two blocks, each branch
is decoded against its own ROM bytes using exact `.4byte 0x08XXXXXX` targets,
keeping both RS and BM 100% byte-identical.
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

from emit_npc_scripts import NPC_SPEC

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000


def format_single_npc_instr(rom: bytes, addr: int) -> tuple[int, str, list[int]] | None:
    ro = addr - BASE
    op = rom[ro]
    if op not in NPC_SPEC:
        return None
    sz, ptrs, branches, _ = NPC_SPEC[op]
    p_strs = {po: f"0x{struct.unpack_from('<I', rom, ro + po)[0]:08X}" for po in ptrs}
    br_tgts = [struct.unpack_from("<I", rom, ro + bo)[0] for bo in branches]
    b = lambda k: rom[ro + k]
    h = lambda k: struct.unpack_from("<H", rom, ro + k)[0]

    if op == 0x00:
        return (1, "    npc_end", br_tgts)
    if op == 0x02:
        return (5, f"    npc_jump {p_strs[1]}", br_tgts)
    if op == 0x03:
        return (1, "    npc_hide", br_tgts)
    if op == 0x04:
        return (7, f"    npc_jump_if_flag_set 0x{h(1):04X}, {p_strs[3]}", br_tgts)
    if op == 0x05:
        return (7, f"    npc_jump_if_flag_clear 0x{h(1):04X}, {p_strs[3]}", br_tgts)
    if op == 0x06:
        return (3, f"    npc_set_flag 0x{h(1):04X}", br_tgts)
    if op == 0x07:
        return (3, f"    npc_clear_flag 0x{h(1):04X}", br_tgts)
    if op == 0x14:
        return (7, f"    npc_set_pos 0x{h(1):04X}, 0x{h(3):04X}, 0x{h(5):04X}", br_tgts)
    if op == 0x16:
        return (2, f"    npc_set_facing 0x{b(1):02X}", br_tgts)
    if op == 0x17:
        return (2, f"    npc_set_sprite 0x{b(1):02X}", br_tgts)
    if op == 0x18:
        return (2, f"    npc_set_talk 0x{b(1):02X}", br_tgts)
    if op == 0x34:
        return (1, "    npc_stop", br_tgts)
    if op == 0x36:
        return (5, f"    npc_call {p_strs[1]}", br_tgts)
    if op == 0x38:
        return (8, f"    npc_call_ex 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, {p_strs[4]}", br_tgts)
    if op == 0x3A:
        return (1, "    npc_ret", br_tgts)
    if op == 0x3B:
        return (7, f"    npc_jump_if_phase 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}", br_tgts)
    if op == 0x42:
        return (7, f"    npc_jump_if_tourney_eq 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}", br_tgts)
    if op == 0x43:
        return (7, f"    npc_jump_if_tourney_ne 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}", br_tgts)

    if not ptrs:
        if sz == 1:
            return (1, f"    npc_op 0x{op:02X}", br_tgts)
        args = ", ".join(f"0x{b(i):02X}" for i in range(1, sz))
        if sz <= 4:
            return (sz, f"    npc_b{sz - 1} 0x{op:02X}, {args}", br_tgts)
        return (sz, f"    .byte 0x{op:02X}, {args}", br_tgts)
    if ptrs == [1] and sz == 5:
        return (5, f"    npc_ptr 0x{op:02X}, {p_strs[1]}", br_tgts)
    if ptrs == [2] and sz == 6:
        return (6, f"    npc_b1_ptr 0x{op:02X}, 0x{b(1):02X}, {p_strs[2]}", br_tgts)
    if ptrs == [3] and sz == 7:
        return (7, f"    npc_b2_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, {p_strs[3]}", br_tgts)
    if ptrs == [4] and sz == 8:
        return (8, f"    .byte 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}; .4byte {p_strs[4]}", br_tgts)
    if ptrs == [5] and sz == 9:
        return (9, f"    npc_b4_ptr 0x{op:02X}, 0x{b(1):02X}, 0x{b(2):02X}, 0x{b(3):02X}, 0x{b(4):02X}, {p_strs[5]}", br_tgts)
    return None


def decode_region(rom: bytes, start_off: int, size: int, is_bm: bool, bm_delta: int) -> tuple[list[str], int]:
    """Decode NPC lists + NPC script blocks within [start_off, start_off + size)."""
    r0 = BASE + start_off
    r1 = r0 + size
    # 1. Scan 4-byte aligned 0x000000FF-terminated NPC pointer lists inside [r0, r1)
    list_blocks: list[tuple[int, int, list[str]]] = []
    seeds: set[int] = set()
    pos = r0
    if pos % 4:
        pos += 4 - (pos % 4)
    while pos + 8 <= r1:
        w = struct.unpack_from("<I", rom, pos - BASE)[0]
        if r0 <= w < r1:
            p = pos
            ptrs = []
            while p + 4 <= r1:
                v = struct.unpack_from("<I", rom, p - BASE)[0]
                if v == 0x000000FF:
                    if ptrs:
                        lines = [f"    @ npc list 0x{pos:08X}..0x{p + 4:08X}"]
                        for x in ptrs:
                            lines.append(f"    .4byte 0x{x:08X}")
                            seeds.add(x)
                        lines.append("    .4byte 0x000000FF")
                        list_blocks.append((pos, p + 4, lines))
                        pos = p + 4
                    break
                if r0 <= v < r1:
                    ptrs.append(v)
                    p += 4
                else:
                    break
            else:
                pos += 4
                continue
            if p + 4 <= r1 and struct.unpack_from("<I", rom, p - BASE)[0] == 0x000000FF and ptrs:
                continue
        pos += 4

    # 2. BFS decode NPC scripts from list seeds (and follow internal branches)
    occupied: set[int] = set()
    for a, b, _ in list_blocks:
        for x in range(a, b):
            occupied.add(x)

    script_blocks: list[tuple[int, int, list[str]]] = []
    queue = sorted(seeds)
    seen_starts: set[int] = set()
    while queue:
        start = queue.pop(0)
        if start in occupied or start in seen_starts or not (r0 <= start < r1):
            continue
        seen_starts.add(start)
        cur = start
        decoded: list[tuple[int, int, str]] = []
        block_seen: set[int] = set()
        ok = True
        while len(decoded) < 250:
            if cur in occupied or cur in block_seen:
                break
            if not (r0 <= cur < r1):
                ok = False
                break
            block_seen.add(cur)
            res = format_single_npc_instr(rom, cur)
            if not res:
                ok = False
                break
            sz, line, brs = res
            if cur + sz > r1:
                ok = False
                break
            op = rom[cur - BASE]
            is_term = NPC_SPEC[op][3]
            for tgt in brs:
                if r0 <= tgt < r1 and tgt not in occupied and tgt not in seen_starts:
                    queue.append(tgt)
            decoded.append((cur, sz, line))
            cur += sz
            if is_term:
                break
        if ok and decoded:
            b0 = decoded[0][0]
            b1 = decoded[-1][0] + decoded[-1][1]
            if not any(x in occupied for x in range(b0, b1)):
                for x in range(b0, b1):
                    occupied.add(x)
                lines = [f"    @ npc script 0x{b0:08X}..0x{b1:08X}"] + [d[2] for d in decoded]
                script_blocks.append((b0, b1, lines))

    all_blocks = sorted(list_blocks + script_blocks, key=lambda x: x[0])
    out: list[str] = []
    cur = r0
    decoded_bytes = 0
    for b0, b1, blines in all_blocks:
        if b0 < cur:
            continue
        if b0 > cur:
            inc_off = (cur - BASE) - (bm_delta if is_bm else 0)
            out.append(f"    incrom 0x{inc_off:X}, 0x{b0 - cur:X}")
        out.extend(blines)
        decoded_bytes += (b1 - b0)
        cur = b1
    if cur < r1:
        inc_off = (cur - BASE) - (bm_delta if is_bm else 0)
        out.append(f"    incrom 0x{inc_off:X}, 0x{r1 - cur:X}")
    return out, decoded_bytes


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()

    targets = [
        ("code_04b", 0x60264, 0x1064, 0x1060, 0x18),
        ("code_04c", 0x6544C, 0x2210, 0x2208, 0x14),
    ]
    for seg, rs_off, rs_sz, bm_sz, delta in targets:
        path = DECOMP / f"asm/{seg}.s"
        txt = path.read_text(encoding="utf-8")
        old_block = (
            f"    .if VERSION_BM\n"
            f"    incrom 0x{rs_off:X}, 0x{bm_sz:X}\n"
            f"    .else\n"
            f"    incrom 0x{rs_off:X}, 0x{rs_sz:X}\n"
            f"    .endif"
        )
        assert old_block in txt, f"block not found in {seg}.s"
        bm_lines, bm_dec = decode_region(bm, rs_off + delta, bm_sz, True, delta)
        rs_lines, rs_dec = decode_region(rs, rs_off, rs_sz, False, 0)
        new_block = (
            "    .if VERSION_BM\n"
            + "\n".join(bm_lines) + "\n"
            + "    .else\n"
            + "\n".join(rs_lines) + "\n"
            + "    .endif"
        )
        txt = txt.replace(old_block, new_block, 1)
        path.write_text(txt, encoding="utf-8", newline="\n")
        print(f"{seg:8s}: decoded RS {rs_dec}/{rs_sz}B, BM {bm_dec}/{bm_sz}B")


if __name__ == "__main__":
    main()
