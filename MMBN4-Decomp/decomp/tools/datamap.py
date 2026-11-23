#!/usr/bin/env python3
"""Map RS -> BM address shifts across the whole ROM by block alignment.

    python3 tools/datamap.py

Slides 256-byte probe blocks of RS (every 4 KB) and looks them up in BM within
+-64 KB. Consecutive probes with one delta form a region. Writes
symbols/shift_rs_bm.txt: rs_start rs_end delta (probes) — the address map used
to build Blue Moon from the Red Sun source and to port symbols.
"""
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
ROOT = DECOMP.parent
BASE = 0x08000000
PROBE, STEP, WINDOW = 256, 0x1000, 0x10000


def main() -> None:
    rs = (ROOT / "roms/bn4_rs_us.gba").read_bytes()
    bm = (ROOT / "roms/bn4_bm_us.gba").read_bytes()
    results = []                      # (rs_off, delta or None)
    last = 0
    for off in range(0, len(rs) - PROBE, STEP):
        blk = rs[off:off + PROBE]
        if blk.count(blk[0]) == PROBE:            # uniform (padding): uninformative
            results.append((off, "pad"))
            continue
        delta = None
        # try the last delta first (fast path), then search the window
        if bm[off + last:off + last + PROBE] == blk:
            delta = last
        else:
            lo, hi = max(0, off - WINDOW), min(len(bm), off + WINDOW + PROBE)
            i = bm.find(blk, lo, hi)
            if i >= 0 and bm.find(blk, i + 1, hi) < 0:   # unique in window
                delta = i - off
        if delta is not None:
            last = delta
        results.append((off, delta))

    regions = []
    for off, d in results:
        if d == "pad":
            continue
        if regions and regions[-1][2] == d:
            regions[-1][1] = off + STEP
            regions[-1][3] += 1
        else:
            regions.append([off, off + STEP, d, 1])
    out = DECOMP / "symbols" / "shift_rs_bm.txt"
    with out.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# rs_start rs_end bm_delta probes   (tools/datamap.py; delta None = no unique match)\n")
        for a, b, d, n in regions:
            f.write(f"{BASE + a:08X} {BASE + b:08X} {d if d is None else f'{d:+#x}'} {n}\n")
    matched = sum(1 for _, d in results if isinstance(d, int))
    print(f"probes: {len(results)}, matched {matched}, padding {sum(1 for _, d in results if d == 'pad')}, "
          f"unmatched {sum(1 for _, d in results if d is None)}")
    print(f"regions: {len(regions)}")
    for a, b, d, n in regions[:60]:
        print(f"  0x{BASE + a:08X}-0x{BASE + b:08X}  delta {d if d is None else f'{d:+#x}'}  ({n} probes)")
    print(f"wrote {out.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
