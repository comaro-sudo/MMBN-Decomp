#!/usr/bin/env python3
"""Emit the event object-spawn lists of sub_080A203A (code_06, RS 0x080A1C40-0x080A2028) in place.

sub_080A203A (map group 0x92, between event flags 9:0x52 and 9:0x53) takes
dat_080A1C34[map] and walks 12-byte records {s16 x, s16 y, s16 z, u16 unk, ptr params}
until one whose ptr is 0, spawning object kind 0x16 at (x, y, z) << 16. The pointers lead
to small parameter blobs that follow each list; they're emitted as labelled bytes.
Pointer labels resolve per version (BM = RS + SEG_BM_DELTA).

Claude, 2026-11-27. Run from decomp/: python3 tools/emit_event_spawns.py
"""
from pathlib import Path
import struct

DECOMP = Path(__file__).resolve().parents[1]
S, E = 0xA1C40, 0xA2028
BASE = 0x08000000
DELTA = 0xC
LINE = f"    incrom 0x{S:X}, 0x{E - S:X}"
# Macros: include/data_macros.inc (loaded by include/macros.inc for every segment).


def main() -> None:
    rs = (DECOMP / "../roms/bn4_rs_us.gba").read_bytes()
    bm = (DECOMP / "../roms/bn4_bm_us.gba").read_bytes()
    path = DECOMP / "asm/code_06.s"
    text = path.read_text()
    assert text.count(LINE + "\n") == 1, "incrom line not found exactly once"

    heads = [struct.unpack_from("<I", rs, 0xA1C34 + 4 * i)[0] - BASE for i in range(3)]
    assert heads == [0xA1C40, 0xA1D30, 0xA1E6C], [hex(h) for h in heads]

    records, labels = {}, set(heads)
    for h in heads:
        p = h
        while True:
            x, y, z, u, ptr = struct.unpack_from("<hhhHI", rs, p)
            bx, by, bz, bu, bptr = struct.unpack_from("<hhhHI", bm, p + DELTA)
            if ptr == 0:
                assert rs[p:p + 12] == bytes(12) and bptr == 0
                records[p] = None
                break
            assert (x, y, z, u) == (bx, by, bz, bu) and bptr == ptr + DELTA, hex(p)
            assert S <= ptr - BASE < E, hex(ptr)
            records[p] = (x, y, z, u, ptr - BASE)
            labels.add(ptr - BASE)
            p += 12
    # every non-record byte must match between versions (blobs carry no pointers)
    out, p, blob = [], S, []

    def flush():
        if blob:
            out.append("    .byte " + ", ".join(f"0x{v:02X}" for v in blob))
            blob.clear()

    while p < E:
        if p in labels:
            flush()
            out += [f"    .global dat_{BASE + p:08X}", f"dat_{BASE + p:08X}:"]
        if p in records:
            flush()
            r = records[p]
            if r is None:
                out.append("    event_spawn_end")
            else:
                out.append(f"    event_spawn {r[0]}, {r[1]}, {r[2]}, 0x{r[3]:04X}, dat_{BASE + r[4]:08X}")
            p += 12
            continue
        assert rs[p] == bm[p + DELTA], hex(p)
        blob.append(rs[p])
        if len(blob) == 16:
            flush()
        p += 1
    flush()

    text = text.replace(LINE + "\n", "\n".join(out) + "\n", 1)
    path.write_text(text, encoding="utf-8", newline="\n")
    n = sum(1 for v in records.values() if v)
    print(f"emitted {n} spawn records in {len(heads)} lists, {len(labels)} labels")


if __name__ == "__main__":
    main()
