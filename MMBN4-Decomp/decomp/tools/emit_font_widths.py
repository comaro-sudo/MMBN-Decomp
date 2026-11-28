#!/usr/bin/env python3
"""Emit BN4_FONT_WIDTHS (code_04, RS 0x080515E0-0x080523A8, BM +0xC) in place.

Two fonts x 441 u32 glyph widths in pixels (mostly 8, some 6/7). sub_08053B74 draws one
glyph: it takes font [r5+0x34] from the 2-pointer table dat_08053C18 (dat_080515E0,
dat_08051CC4), reads width = table[glyph], advances the text cursor by it and blits
11 rows. Content RS == BM.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_font_widths.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000
S, MID, E = 0x515E0, 0x51CC4, 0x515E0 + 0xDC8
LINE = "    incrom 0x515E0, 0xDC8"


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    assert rs[S:E] == bm[S + 0xC:E + 0xC], "RS and BM differ"
    path = DECOMP / "asm/code_04.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1, "incrom line not found exactly once"
    out = []
    for a, b, font in ((S, MID, 0), (MID, E, 1)):
        if a != S:  # the first table's label is the line before the incrom already
            out += [f"    .global dat_{BASE + a:08X}", f"dat_{BASE + a:08X}:"]
        out.append(f"    @ font {font}: {(b - a) // 4} glyph widths")
        vals = struct.unpack_from(f"<{(b - a) // 4}I", rs, a)
        for i in range(0, len(vals), 16):
            out.append("    .word " + ", ".join(str(v) for v in vals[i:i + 16]))
    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted 2 font width tables ({E - S} bytes)")


if __name__ == "__main__":
    main()
