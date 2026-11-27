#!/usr/bin/env python3
"""Emit BN4_FLAG_PROPS (code_06, RS 0x080AEA00, BM +0xC, 74 x 16 B) in place.

sub_080AEEA0 picks record [obj+4] (x 0x10); its states use the record as:
  +0 u8 sprite list, +1 u8 sprite id (BN4_SPRITE_LOAD), +2 u8 anim, +3 u8 palette (0xFF none),
  +4 u32 sprite attribute (sub_08002FF8), +8 u16 flag tested, +0xA u16 / +0xC u16 flags set
  when it is set and cleared otherwise (sub_080AEF12), +0xE u16 (unused here).
No pointers; content RS == BM.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_flag_props.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
S, SIZE = 0xAEA00, 0x4A0
LINE = f"    incrom 0x{S:X}, 0x{SIZE:X}"
MACROS = """
.macro flag_prop list, sprite, anim, pal, attr, test, flag_a, flag_b, unk
    .byte \\list, \\sprite, \\anim, \\pal
    .word \\attr
    .hword \\test, \\flag_a, \\flag_b, \\unk
.endm
"""


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    assert rs[S:S + SIZE] == bm[S + 0xC:S + 0xC + SIZE], "RS and BM differ"
    path = DECOMP / "asm/code_06.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1
    out = []
    for i in range(SIZE // 16):
        lst, spr, anim, pal, attr, t, a, b, u = struct.unpack_from("<4BI4H", rs, S + 16 * i)
        out.append(f"    flag_prop 0x{lst:02X}, 0x{spr:02X}, {anim}, 0x{pal:02X}, 0x{attr:X}, "
                   f"0x{t:04X}, 0x{a:04X}, 0x{b:04X}, 0x{u:04X}   @ {i}")
    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    if ".macro flag_prop" not in text:
        text = text.replace("    .thumb\n", "    .thumb\n" + MACROS, 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted {SIZE // 16} flag_prop records")


if __name__ == "__main__":
    main()
