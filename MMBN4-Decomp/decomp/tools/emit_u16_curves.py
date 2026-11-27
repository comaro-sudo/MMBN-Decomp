#!/usr/bin/env python3
"""Emit the 8 u16 step tables of dat_080A72C0 (code_06, RS 0x080A72E0-0x080A78A0, BM +0xC) in place.

dat_080A72C0 is a table of 8 pointers to u16 arrays (0x340, 0x60, 0x60, 0xC0 and 4 x 0x40
bytes) of slowly rising values (0, 0, ..., 1, 1, ..., 2, ...), read per frame. Each array
gets its dat_ label. No pointers inside; content RS == BM.

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_u16_curves.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
S, E, BASE = 0xA72E0, 0xA78A0, 0x08000000
LINE = f"    incrom 0x{S:X}, 0x{E - S:X}"


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    assert rs[S:E] == bm[S + 0xC:E + 0xC], "RS and BM differ"
    starts = sorted(struct.unpack_from("<I", rs, 0xA72C0 + 4 * i)[0] - BASE for i in range(8))
    assert starts[0] == S and all(S <= s < E for s in starts), [hex(s) for s in starts]
    path = DECOMP / "asm/code_06.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1
    out = []
    bounds = starts + [E]
    for a, b in zip(bounds, bounds[1:]):
        out += [f"    .global dat_{BASE + a:08X}", f"dat_{BASE + a:08X}:"]
        vals = struct.unpack_from(f"<{(b - a) // 2}H", rs, a)
        for i in range(0, len(vals), 16):
            out.append("    .hword " + ", ".join(str(v) for v in vals[i:i + 16]))
    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"emitted {len(starts)} u16 tables ({E - S} bytes)")


if __name__ == "__main__":
    main()
