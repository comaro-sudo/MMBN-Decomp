#!/usr/bin/env python3
"""DC-4: merge the generated Blue Moon view (asm/bm/<seg>.s) into the shared
source asm/<seg>.s, so one tree builds both ROMs.

    python3 tools/merge_bm.py [--dry-run] [seg ...]     (default: every segment with an asm/bm/ file)

How it works:
1. Every address token in the BM file (`sub_`, `dat_`, `_` + 8 hex digits) is
   renamed to its Red Sun equivalent: inside a code segment, RS = BM - the
   segment's delta (config/segments_*.toml). BM addresses with no RS twin (the
   few bytes BM inserts at the end of a segment) get a `_bm` suffix. Addresses
   outside the code region (the data region) are left as they are.
2. `incrom` offsets are rebased the same way, since the macro adds
   SEG_BM_DELTA back in a BM build.
3. The normalized BM file is diffed line by line against the RS file. Equal
   runs are emitted once; differences become `.if VERSION_BM` / `.else` /
   `.endif` blocks.
4. With --dry-run, only the per-segment difference counts are printed.
   Otherwise asm/<seg>.s is rewritten and asm/bm/<seg>.s is deleted.

Afterwards, run `make -B compare` for both versions. Regenerating a segment
with disasm.py writes per-version files again; run disasm.py for rs and bm,
then this script.
"""
from __future__ import annotations

import difflib
import re
import sys
import tomllib
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000
TOKEN = re.compile(r"\b(sub_|dat_|_)([0-9A-F]{8})\b")
INCROM = re.compile(r"^(\s*incrom\s+)0x([0-9A-Fa-f]+)(\s*,.*)$")


def segments(v: str) -> dict[str, dict]:
    cfg = tomllib.loads((DECOMP / "config" / f"segments_{v}.toml").read_text())
    return {s["name"]: s for s in cfg["segment"] if s["kind"] == "code"}


RS, BM = segments("rs"), segments("bm")
# (bm_start, bm_end, delta, rs_end) for every code segment
SPANS = sorted((BM[n]["start"], BM[n]["end"], BM[n]["start"] - RS[n]["start"], RS[n]["end"]) for n in RS)


def bm_to_rs(a: int) -> tuple[int, bool]:
    """RS address for BM address a, and whether it is BM-only (segment tail)."""
    for start, end, delta, rs_end in SPANS:
        if start <= a < end:
            r = a - delta
            return (a, True) if r >= rs_end else (r, False)
    return a, False


def normalize(line: str, delta: int) -> str:
    def tok(m: re.Match) -> str:
        r, tail = bm_to_rs(int(m.group(2), 16))
        return f"{m.group(1)}{r:08X}" + ("_bm" if tail else "")
    m = INCROM.match(line)
    if m:
        line = f"{m.group(1)}0x{int(m.group(2), 16) - delta:X}{m.group(3)}"
    elif line.lstrip().startswith(".set SEG_BM_DELTA,"):
        # incrom offsets are now RS-based; the macro adds this back in BM.
        return f"    .set SEG_BM_DELTA, 0x{delta:X}   @ Blue Moon = RS + this, inside this segment"
    return TOKEN.sub(tok, line)


def rs_addr_to_bm(r: int) -> int:
    """Inverse of bm_to_rs for a non-tail RS-named address (data region: unchanged)."""
    for name, s in RS.items():
        if s["start"] <= r < s["end"]:
            return r + BM[name]["start"] - s["start"]
    return r


def bm_value(name: str) -> int:
    """BM address of a shared-source symbol name (sub_/dat_ + 8 hex, optional _bm)."""
    a = int(name.split("_")[1], 16)
    return a if name.endswith("_bm") else rs_addr_to_bm(a)


def normalize_file(text: str, name: str) -> str:
    """A BM-addressed generated file (asm/bm/<seg>.s) renamed to shared-source names."""
    delta = BM[name]["start"] - RS[name]["start"]
    return "\n".join(normalize(l, delta) for l in text.splitlines()) + "\n"


IF_RE = re.compile(r"^\s*\.if\s+VERSION_(RS|BM)\b")


def active_lines(text: str, v: str) -> list[str]:
    """Lines a build of version v assembles (resolves the VERSION_* blocks this script writes)."""
    out, stack = [], []
    for line in text.splitlines():
        s = line.strip()
        m = IF_RE.match(line)
        if m:
            stack.append(m.group(1) == v.upper())
        elif s.startswith(".else") and stack:
            stack[-1] = not stack[-1]
        elif s.startswith(".endif") and stack:
            stack.pop()
        elif all(stack):
            out.append(line)
    return out


def merge(name: str, dry: bool) -> tuple[int, int]:
    rs_path, bm_path = DECOMP / "asm" / f"{name}.s", DECOMP / "asm" / "bm" / f"{name}.s"
    rs = rs_path.read_text().splitlines()
    if any(IF_RE.match(l) for l in rs):
        sys.exit(f"asm/{name}.s is already shared source: regenerate it first with "
                 f"`tools/disasm.py rs {name}`, then `tools/disasm.py bm {name}`, then rerun this")
    delta = BM[name]["start"] - RS[name]["start"]
    bm = [normalize(l, delta) for l in bm_path.read_text().splitlines()]
    # The header comment and the SEG_BM_DELTA line differ by design.
    for i, l in enumerate(rs[:6]):
        if l.startswith(f"@ {name}:"):
            rs[i] = (f"@ {name}: RS 0x{RS[name]['start']:08X}-0x{RS[name]['end']:08X}, "
                     f"BM 0x{BM[name]['start']:08X}-0x{BM[name]['end']:08X} (shared source, tools/merge_bm.py)")
    bm = [rs[i] if i < 6 and (l.startswith(f"@ {name}:") or "SEG_BM_DELTA" in l) else l
          for i, l in enumerate(bm)]
    sm = difflib.SequenceMatcher(None, rs, bm, autojunk=False)
    out, hunks, changed = [], 0, 0
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op == "equal":
            out += rs[i1:i2]
            continue
        hunks += 1
        changed += max(i2 - i1, j2 - j1)
        if op == "insert":
            out += ["    .if VERSION_BM   @ BM only"] + bm[j1:j2] + ["    .endif"]
        elif op == "delete":
            out += ["    .if VERSION_RS   @ RS only"] + rs[i1:i2] + ["    .endif"]
        else:
            out += ["    .if VERSION_BM"] + bm[j1:j2] + ["    .else"] + rs[i1:i2] + ["    .endif"]
    if not dry:
        rs_path.write_text("\n".join(out) + "\n", newline="\n")
        bm_path.unlink()
    return hunks, changed


def main() -> None:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    dry = "--dry-run" in sys.argv
    names = args or sorted(p.stem for p in (DECOMP / "asm" / "bm").glob("*.s"))
    for n in names:
        if n not in RS:
            sys.exit(f"{n}: not a code segment")
        h, c = merge(n, dry)
        print(f"{n}: {h} version blocks, {c} lines differ" + (" (dry run)" if dry else ""))


if __name__ == "__main__":
    main()
