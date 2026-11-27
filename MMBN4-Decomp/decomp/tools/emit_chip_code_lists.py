#!/usr/bin/env python3
"""Emit BN4_CHIP_CODE_LISTS (code_03, RS 0x0804CFF8-0x0804DAF0, BM +8) in place.

468 six-byte records {u16 chip id, u8 code[4]} (0-25 = A-Z, 26 = *, 0xFF = none), in
4 ascending lists (new list where the id drops). The 12-byte descriptor table at
dat_0804DE0C points to them (with a second pointer per entry, to 0x0804CF94/99);
what the four lists are used for is not traced yet. The 4 bytes before the records
(0x0804CFF4) end the small tables dat_0804CFF0/CFF2 that sub_0804DB6C reads; they
stay as bytes. Content is RS == BM.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_chip_code_lists.py
"""
from pathlib import Path
import re

DECOMP = Path(__file__).resolve().parents[1]
S, R, E = 0x4CFF4, 0x4CFF8, 0x4DAF0
BASE = 0x08000000
LINE = "    incrom 0x4CFF4, 0xAFC"
# Macros: include/data_macros.inc (loaded by include/macros.inc for every segment).


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    assert rs[S:E] == bm[S + 8:E + 8], "RS and BM differ"
    path = DECOMP / "asm/code_03.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1
    refs = {int(m, 16) - BASE for m in re.findall(r"dat_(0804[CD][0-9A-F]{3})\b", text)
            if R <= int(m, 16) - BASE < E}
    out = ["    .byte " + ", ".join(f"0x{x:02X}" for x in rs[S:R])]
    prev = -1
    for o in range(R, E, 6):
        cid = rs[o] | rs[o + 1] << 8
        if o in refs:
            out += [f"    .global dat_{BASE + o:08X}", f"dat_{BASE + o:08X}:"]
        elif cid <= prev:
            out.append("    @ next list")
        prev = cid
        c = ", ".join(f"0x{x:02X}" for x in rs[o + 2:o + 6])
        out.append(f"    chip_codes 0x{cid:03X}, {c}")
    bad = [hex(x) for x in refs if (x - R) % 6]
    assert not bad, f"references inside a record: {bad}"
    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted {(E - R) // 6} records, {len(refs)} labels")


if __name__ == "__main__":
    main()
