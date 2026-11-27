#!/usr/bin/env python3
"""Emit the story-battle formation lists (code_09, RS 0x080FB47C-0x080FC138, BM +0xC) in place.

Each BN4_STORY_BATTLES record points to a formation list: 4-byte records
{u8 id, u8 col 1-6, u8 row 1-3, u8 kind (0 MegaMan, 1 enemy, 2+ obstacle)}, ended by
one 0xFF byte (lists are byte-packed, so most start unaligned). Format: Gemini,
notes/re06-encounters.md. The region was incrom, with two false pointer runs that
tools/emit_desc_tables.py had carved out of formation bytes; this replaces the whole span.
Every list that the asm references gets its dat_ label. Content is RS == BM.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_formations.py
"""
from pathlib import Path
import re

DECOMP = Path(__file__).resolve().parents[1]
S, E = 0xFB47C, 0xFC138
BASE = 0x08000000
MACROS = """
.macro formation id, col, row, kind
    .byte \\id, \\col, \\row, \\kind
.endm
.macro formation_end
    .byte 0xFF
.endm
"""


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    assert rs[S:E] == bm[S + 0xC:E + 0xC], "RS and BM differ"
    path = DECOMP / "asm/code_09.s"
    lines = path.read_text().splitlines()

    first = next(i for i, l in enumerate(lines) if l.strip() == f"incrom 0x{S:X}, 0x464")
    last = next(i for i, l in enumerate(lines) if l.strip() == f".global dat_{BASE + E:08X}")
    span = [l.strip() for l in lines[first:last]]
    expected = ["incrom 0xFB47C, 0x464", ".4byte dat_08010304", ".4byte dat_08010205",
                "incrom 0xFB8E8, 0x344", ".4byte 0x02000202", ".4byte 0x02010304",
                ".4byte 0x02010105", "incrom 0xFBC38, 0x500"]
    assert span == expected, span

    src = "\n".join(lines)
    refs = {int(m, 16) - BASE for m in re.findall(r"dat_(080F[BC][0-9A-F]{3})\b", src)
            if S <= int(m, 16) - BASE < E}

    out = []
    p = S
    while p < E:
        if p in refs:
            out += [f"    .global dat_{BASE + p:08X}", f"dat_{BASE + p:08X}:"]
        q = p
        while q < E and rs[q] != 0xFF:
            q += 4
        if q >= E:                       # trailing bytes before the next table
            out.append("    .byte " + ", ".join(f"0x{x:02X}" for x in rs[p:E]))
            p = E
            break
        for o in range(p, q, 4):
            i, c, row, k = rs[o:o + 4]
            out.append(f"    formation 0x{i:02X}, {c}, {row}, {k}")
        out.append("    formation_end")
        p = q + 1
    missing = [hex(x) for x in refs if f"dat_{BASE + x:08X}:" not in "\n".join(out)]
    assert not missing, f"references not at list starts: {missing}"

    lines[first:last] = out
    text = "\n".join(lines) + "\n"
    if ".macro formation " not in text:
        text = text.replace("    .thumb\n", "    .thumb\n" + MACROS, 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted formation lists 0x{BASE + S:08X}-0x{BASE + E:08X} ({E - S} bytes), {len(refs)} labels")


if __name__ == "__main__":
    main()
