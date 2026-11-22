#!/usr/bin/env python3
"""Match functions between two versions by masked byte fingerprints.

    python3 tools/fingerprint.py rs bm

Fingerprint = the function body [start, end) from symbols/functions_<v>.txt
with every Thumb BL pair masked (call targets move between versions;
conditional branches and PC-relative loads are relative, so they stay).
Unique hash on both sides = a match. Writes symbols/map_<a>_<b>.txt
(addr_a addr_b size) and prints the address-shift regions, which is
what ports symbols from one version to the other.
"""
from __future__ import annotations

import hashlib
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DECOMP = ROOT / "decomp"
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
BASE = 0x08000000
MIN_SIZE = 12


def load(v: str):
    rom = ROMS[v].read_bytes()
    funcs = []
    for line in (DECOMP / "symbols" / f"functions_{v}.txt").read_text().splitlines():
        if line.startswith("#"):
            continue
        a, mode, size, _src, ok = line.split()
        a, size = int(a, 16), int(size)
        if mode == "T" and ok == "1" and size >= MIN_SIZE:
            funcs.append((a, size))
    return rom, funcs


def fingerprint(rom: bytes, a: int, size: int) -> str:
    body = bytearray(rom[a - BASE:a - BASE + size])
    i = 0
    while i + 4 <= len(body):
        h1, h2 = struct.unpack_from("<HH", body, i)
        if (h1 & 0xF800) == 0xF000 and (h2 & 0xF800) == 0xF800:
            body[i:i + 4] = b"\xff\xff\xff\xff"
            i += 4
        else:
            i += 2
    return hashlib.sha1(bytes(body)).hexdigest()


def table(rom, funcs):
    by = defaultdict(list)
    for a, size in funcs:
        by[fingerprint(rom, a, size)].append((a, size))
    return by


def main() -> None:
    va, vb = (sys.argv[1:3] + ["rs", "bm"])[:2] if len(sys.argv) >= 3 else ("rs", "bm")
    rom_a, fa = load(va)
    rom_b, fb = load(vb)
    ta, tb = table(rom_a, fa), table(rom_b, fb)

    matches = []
    for h, la in ta.items():
        lb = tb.get(h)
        if lb and len(la) == 1 and len(lb) == 1:
            matches.append((la[0][0], lb[0][0], la[0][1]))
    matches.sort()

    out = DECOMP / "symbols" / f"map_{va}_{vb}.txt"
    with out.open("w", encoding="utf-8", newline="\n") as f:
        f.write(f"# {va}_addr {vb}_addr size   (unique masked-fingerprint matches; tools/fingerprint.py)\n")
        for a, b, s in matches:
            f.write(f"{a:08X} {b:08X} {s}\n")

    bytes_a = sum(s for _, s in fa)
    bytes_m = sum(s for _, _, s in matches)
    print(f"{va}: {len(fa)} clean functions >= {MIN_SIZE} bytes ({bytes_a} bytes); "
          f"{vb}: {len(fb)}")
    print(f"unique matches: {len(matches)} functions ({len(matches) / len(fa) * 100:.1f}% of {va}), "
          f"{bytes_m} bytes ({bytes_m / bytes_a * 100:.1f}%)")
    amb = sum(1 for h, la in ta.items() if h in tb and (len(la) > 1 or len(tb[h]) > 1))
    print(f"ambiguous hashes (duplicates, e.g. tiny stubs): {amb}; {va}-only: "
          f"{sum(1 for h in ta if h not in tb)} hashes")

    # Shift regions: runs of matches with the same delta.
    regions, cur = [], None
    for a, b, s in matches:
        d = b - a
        if cur and cur["delta"] == d:
            cur["end"], cur["n"] = a + s, cur["n"] + 1
        else:
            cur = {"start": a, "end": a + s, "delta": d, "n": 1}
            regions.append(cur)
    big = [r for r in regions if r["n"] >= 5]
    print(f"shift regions (>=5 consecutive matches with one delta): {len(big)} "
          f"covering {sum(r['n'] for r in big)} functions")
    for r in big[:40]:
        print(f"  {va} 0x{r['start']:08X}-0x{r['end']:08X}  delta {r['delta']:+#x}  ({r['n']} functions)")
    if len(big) > 40:
        print(f"  ... {len(big) - 40} more")
    print(f"wrote {out.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
