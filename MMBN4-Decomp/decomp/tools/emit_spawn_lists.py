#!/usr/bin/env python3
"""Emit every Object_SpawnList list (0x080034E4) that is still incrom, in place.

The lists are found from the code, not guessed: at each `bl sub_080034E4`, the list
comes from `ldr r0, =list` or from a table `ldr r1, =table; ldr r0, [r1, r0]`
(usually indexed by map number). Each table entry that parses as a list is emitted.
Record: {u8 kind, u8 arg, u16 unk, s32 x, s32 y, s32 z (16.16), u32 param}; a kind of
0xFF ends the list (4 bytes ff 00 00 00). Lists already emitted (not inside one incrom
line) are left alone.

Run it **before emit_cutscene_scripts.py** (tools/regen.sh does): that emitter treats
any referenced label as a possible cutscene and decoded these lists' field bytes
(05 06 00) as `csc_b1 0x05, 0x06; csc_end`. Once the lists are emitted it can't claim them.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_spawn_lists.py
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000
SEGS = ["code_00", "code_01", "code_02", "code_03", "code_04", "code_04b", "code_04c",
        "code_04d", "code_05", "code_06", "code_07", "code_08", "code_09"]
# Macros: include/data_macros.inc (loaded by include/macros.inc for every segment).


def fx(v: int) -> str:
    return f"({v >> 16} << 16)" if v & 0xFFFF == 0 else f"0x{v & 0xFFFFFFFF:08X}"


def is_rom(w: int) -> bool:
    return BASE <= w < BASE + 0x800000


def parse_list(rs: bytes, bm: bytes, off: int, delta: int):
    """Return the list's emitted lines and byte length, or None if it isn't one."""
    out, p = [], off
    for _ in range(64):
        if rs[p] == 0xFF:
            if rs[p:p + 4] != b"\xff\0\0\0" or bm[p + delta:p + delta + 4] != b"\xff\0\0\0":
                return None
            out.append("    obj_spawn_end")
            return out, p + 4 - off
        k, a, u, x, y, z, prm = struct.unpack_from("<BBHiiiI", rs, p)
        b = struct.unpack_from("<BBHiiiI", bm, p + delta)
        if b[:6] != (k, a, u, x, y, z):
            return None
        if prm == b[6] and not is_rom(prm):
            ps = f"0x{prm:X}"
        elif is_rom(prm) and b[6] == prm + delta:
            ps = f"dat_{prm:08X}"
        else:
            return None
        out.append(f"    obj_spawn 0x{k:02X}, 0x{a:02X}, 0x{u:04X}, {fx(x)}, {fx(y)}, {fx(z)}, {ps}")
        p += 20
    return None


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    texts = {s: (DECOMP / f"asm/{s}.s").read_text() for s in SEGS}
    deltas = {s: int(re.search(r"SEG_BM_DELTA,\s*(-?0x[0-9a-fA-F]+|\d+)", t).group(1), 0)
              for s, t in texts.items()}
    lit = re.compile(r"^(_[0-9A-F]{8}):\s*\n\s*\.word (?:dat_|0x)([0-9A-Fa-f]{8})", re.M)
    lits = {m.group(1): int(m.group(2), 16) for t in texts.values() for m in lit.finditer(t)}

    # 1. list starts, from the call sites
    starts: set[int] = set()
    for t in texts.values():
        lines = t.splitlines()
        for i, l in enumerate(lines):
            if l.split("@")[0].strip() != "bl sub_080034E4":   # ignore name comments
                continue
            prev = [x.split("@")[0].strip() for x in lines[max(0, i - 3):i]]
            if prev and prev[-1] == "ldr r0, [r1, r0]":
                m = next((re.match(r"ldr r1, (_[0-9A-F]{8})$", x) for x in reversed(prev[:-1])
                          if re.match(r"ldr r1, _", x)), None)
                if m and m.group(1) in lits:
                    tab = lits[m.group(1)] - BASE
                    for j in range(64):
                        w = struct.unpack_from("<I", rs, tab + 4 * j)[0]
                        if not is_rom(w):
                            break
                        starts.add(w - BASE)
            else:
                m = next((re.match(r"ldr r0, (_[0-9A-F]{8})$", x) for x in reversed(prev)
                          if re.match(r"ldr r0, _", x)), None)
                if m and m.group(1) in lits:
                    starts.add(lits[m.group(1)] - BASE)

    # 2. emit the ones that sit wholly inside one incrom line
    inc = re.compile(r"^(\s*)incrom 0x([0-9A-F]+), 0x([0-9A-F]+)$")
    total = 0
    for s in SEGS:
        lines = texts[s].splitlines()
        d = deltas[s]
        out, changed, n_lists = [], False, 0
        for line in lines:
            m = inc.match(line)
            if not m:
                out.append(line)
                continue
            off, size = int(m.group(2), 16), int(m.group(3), 16)
            here = sorted(a for a in starts if off <= a < off + size)
            cur, pieces = off, []
            for a in here:
                if a < cur:
                    continue
                r = parse_list(rs, bm, a, d)
                if not r or a + r[1] > off + size:
                    continue
                body, ln = r
                if a > cur:
                    pieces.append(f"{m.group(1)}incrom 0x{cur:X}, 0x{a - cur:X}")
                label = f"dat_{BASE + a:08X}"
                if f"\n{label}:" not in texts[s] and not (pieces and pieces[-1] == f"{label}:"):
                    pieces += [f"    .global {label}", f"{label}:"]
                pieces += body
                cur = a + ln
                n_lists += 1
                total += ln
            if not pieces:
                out.append(line)
                continue
            if cur < off + size:
                pieces.append(f"{m.group(1)}incrom 0x{cur:X}, 0x{off + size - cur:X}")
            out += pieces
            changed = True
        if changed:
            text = "\n".join(out) + "\n"
            texts[s] = text
            (DECOMP / f"asm/{s}.s").write_text(text, encoding="utf-8", newline="\n")
            print(f"{s:8s}: {n_lists} spawn lists")
    print(f"TOTAL: {total} bytes from {len(starts)} list pointers")


if __name__ == "__main__":
    main()
