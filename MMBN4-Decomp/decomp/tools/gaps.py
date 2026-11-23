#!/usr/bin/env python3
"""Classify the undecoded gaps of the code region after a survey.

    python3 tools/gaps.py rs

Re-runs the survey in memory, then looks at every undecoded gap inside the
code region: does it start with a Thumb `push {..., lr}` prologue that decodes
cleanly to a return? Those are probably functions nothing reached yet.
Prints counts and bytes per gap type, plus register usage at `mov pc, rX`.
"""
import struct
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import survey as sv  # noqa: E402


def main() -> None:
    v = sys.argv[1] if len(sys.argv) > 1 else "rs"
    rom = sv.Rom(sv.ROMS[v])
    s = sv.Survey(rom)
    s.code_end = 0x08120000
    ins = next(s.arm.disasm(rom.d[:4], sv.BASE))
    s.seed(sv.imm_target(ins.op_str), False, "entry")
    s.run()
    for a, _ in sv.ce_code_symbols():
        s.seed(a | 1, True, "ce")
    s.run()
    s.pointer_tables()
    s.run()
    s.short_prologue_pointer_tables()
    s.run()

    covered = set(s.code)
    for l in s.literals:
        covered |= {l, l + 2}
    gaps, a = [], sv.BASE
    while a < s.code_end:
        if a in covered:
            a += 2
            continue
        b = a
        while b < s.code_end and b not in covered:
            b += 2
        gaps.append((a, b))
        a = b

    kinds, nbytes = Counter(), Counter()
    prologue_starts = []
    for a, b in gaps:
        size = b - a
        h = struct.unpack_from("<H", rom.d, a - sv.BASE)[0]
        # skip alignment padding (0x0000) to the first real halfword
        p = a
        while p < b and struct.unpack_from("<H", rom.d, p - sv.BASE)[0] == 0:
            p += 2
        hp = struct.unpack_from("<H", rom.d, p - sv.BASE)[0] if p < b else 0
        if size <= 2 and h in (0, 0x46C0):
            k = "padding"
        elif p < b and (hp & 0xFF00) == 0xB500:
            k = "starts with push{lr} (likely unreached function)"
            prologue_starts.append(p)
        elif all(struct.unpack_from("<H", rom.d, x - sv.BASE)[0] == 0 for x in range(a, b, 2)):
            k = "zeros"
        else:
            k = "other (data or code without prologue)"
        kinds[k] += 1
        nbytes[k] += size

    total = sum(nbytes.values())
    print(f"undecoded gaps in code region: {len(gaps)}, {total} bytes")
    for k in kinds:
        print(f"  {k}: {kinds[k]} gaps, {nbytes[k]} bytes ({nbytes[k] / total * 100:.1f}%)")
    sizes = sorted((b - a, a) for a, b in gaps)[-8:]
    print("largest gaps:", ", ".join(f"0x{a:08X}+{sz}" for sz, a in reversed(sizes)))
    pcregs = Counter()
    for off in range(0, s.code_end - sv.BASE, 2):
        h = struct.unpack_from("<H", rom.d, off)[0]
        if (h & 0xFF87) == 0x4687 and (sv.BASE + off) in covered:
            pcregs[["r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
                    "sp", "lr", "pc"][(h >> 3) & 0xF]] += 1
    print("decoded `mov pc, rX` by register:", dict(pcregs))
    out = Path(__file__).resolve().parents[1] / "symbols" / f"prologue_candidates_{v}.txt"
    out.write_text("\n".join(f"{p:08X}" for p in prologue_starts) + "\n", newline="\n")
    print(f"wrote {len(prologue_starts)} prologue candidates to {out.name}")


if __name__ == "__main__":
    main()
