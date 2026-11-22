#!/usr/bin/env python3
"""DC-1 survey: find functions by recursive descent and report code coverage.

    ~/.venvs/mmbn4-decomp/bin/python tools/survey.py [rs|bm] [--out notes/]

Seeds: the ROM entry (ARM), Cyberworld Endless code symbols (RS only),
odd ROM pointers found in literal pools, and tables of >=4 consecutive
Thumb code pointers. Follows bl/b/bcc; stops at bx, pop {pc}, mov/add pc.
Jump tables (mov pc / add pc) are not resolved yet: that's DC-2.

Output: symbols/functions_<v>.txt (addr mode size source) and a summary.
ARMv4T only: any 32-bit Thumb encoding other than BL is treated as invalid.
"""
from __future__ import annotations

import argparse
import csv
import re
import struct
import sys
from collections import Counter
from pathlib import Path

from capstone import CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB, Cs

ROOT = Path(__file__).resolve().parents[2]
DECOMP = ROOT / "decomp"
BASE = 0x08000000
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
CE_CSV = ROOT / "ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/symbols/bn4-red-sun-us.csv"

COND_B = {"beq", "bne", "bcs", "bhs", "bcc", "blo", "bmi", "bpl", "bvs", "bvc",
          "bhi", "bls", "bge", "blt", "bgt", "ble"}


class Rom:
    def __init__(self, path: Path):
        self.d = path.read_bytes()
        self.end = BASE + len(self.d)

    def in_rom(self, a: int) -> bool:
        return BASE <= a < self.end

    def u32(self, a: int) -> int:
        return struct.unpack_from("<I", self.d, a - BASE)[0]


def imm_target(op_str: str) -> int | None:
    m = re.search(r"#(0x[0-9a-f]+|\d+)$", op_str)
    return int(m.group(1), 0) if m else None


class Survey:
    def __init__(self, rom: Rom):
        self.rom = rom
        self.thumb = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
        self.arm = Cs(CS_ARCH_ARM, CS_MODE_ARM)
        self.funcs: dict[int, dict] = {}       # start -> info
        self.queue: list[tuple[int, bool, str]] = []
        self.code: set[int] = set()            # halfword addresses decoded as code
        self.literals: set[int] = set()
        self.unresolved = Counter()            # kind -> count
        self.ram_calls = Counter()

    code_end: int | None = None
    rejected = 0

    def seed(self, addr: int, thumb: bool, source: str) -> None:
        if self.code_end is not None and self.rom.in_rom(addr) and (addr & ~1) >= self.code_end:
            self.rejected += 1
            return
        if not self.rom.in_rom(addr):
            if 0x02000000 <= addr < 0x04000000:
                self.ram_calls["ewram" if addr < 0x03000000 else "iwram"] += 1
            return
        if thumb and addr & 1:
            addr &= ~1
        if addr not in self.funcs:
            self.funcs[addr] = {"thumb": thumb, "source": source, "end": addr, "ok": True}
            self.queue.append((addr, thumb, source))

    def run(self) -> None:
        while self.queue:
            start, thumb, _ = self.queue.pop()
            self.walk(start, thumb)

    def walk(self, start: int, thumb: bool) -> None:
        info = self.funcs[start]
        md = self.thumb if thumb else self.arm
        work, seen = [start], set()
        while work:
            a = work.pop()
            prev = None
            while self.rom.in_rom(a) and a not in seen:
                off = a - BASE
                insn = next(md.disasm(self.rom.d[off:off + 4], a), None)
                if insn is None or (thumb and insn.size == 4 and insn.mnemonic != "bl"):
                    info["ok"] = False
                    self.unresolved["invalid"] += 1
                    break
                seen.add(a)
                for h in range(a, a + insn.size, 2):
                    self.code.add(h)
                info["end"] = max(info["end"], a + insn.size)
                m, ops = insn.mnemonic, insn.op_str
                nxt = a + insn.size
                if m == "bl":
                    t = imm_target(ops)
                    if t is not None:
                        self.seed(t | (1 if thumb else 0), thumb, "bl")
                elif m == "b":
                    t = imm_target(ops)
                    if t is not None:
                        work.append(t)
                    break
                elif m in COND_B or (not thumb and m[:1] == "b" and m[1:] in {c[1:] for c in COND_B}):
                    t = imm_target(ops)
                    if t is not None:
                        work.append(t)
                elif m == "bx":
                    if prev == ("mov", "lr, pc"):
                        # `mov lr, pc; bx rX` is a call through a register:
                        # execution returns to the next instruction.
                        self.unresolved["bx_call"] += 1
                        prev = (m, ops)
                        a = nxt
                        continue
                    if not ops.startswith("lr"):
                        self.unresolved["bx_reg"] += 1
                    break
                elif m == "pop" and "pc" in ops:
                    break
                elif m in ("mov", "add", "ldr") and ops.startswith("pc"):
                    self.unresolved["jump_table_or_pc_write"] += 1
                    break
                elif m == "ldr" and "[pc" in ops:
                    imm = imm_target(ops.rstrip("]")) or 0
                    lit = ((a + (4 if thumb else 8)) & ~3) + imm
                    if self.rom.in_rom(lit):
                        self.literals.add(lit)
                        v = self.rom.u32(lit)
                        if self.rom.in_rom(v) and v & 1:
                            self.seed(v, True, "literal_ptr")
                        elif 0x02000000 <= v < 0x04000000 and v & 1:
                            self.ram_calls["literal_ram_code_ptr"] += 1
                prev = (m, ops)
                a = nxt
            # literal pools right after the function belong to it (rough)

    def prologue_pass(self) -> int:
        """Seed undecoded gaps that start (after 0x0000 padding) with a Thumb
        `push {..., lr}`: functions nothing references directly (reached only
        through data we don't parse yet). Repeat until nothing new decodes."""
        added_total = 0
        end = self.code_end or self.rom.end
        for _ in range(8):
            covered = set(self.code)
            for l in self.literals:
                covered |= {l, l + 2}
            added = 0
            a = BASE
            while a < end:
                if a in covered:
                    a += 2
                    continue
                p = a
                while p < end and p not in covered and struct.unpack_from("<H", self.rom.d, p - BASE)[0] == 0:
                    p += 2
                if p < end and p not in covered and \
                        (struct.unpack_from("<H", self.rom.d, p - BASE)[0] & 0xFF00) == 0xB500 and \
                        p not in self.funcs:
                    self.seed(p | 1, True, "prologue")
                    added += 1
                while a < end and a not in covered:
                    a += 2
            if not added:
                break
            self.run()
            added_total += added
        return added_total

    def pointer_tables(self, min_run: int = 4) -> int:
        """Seed from runs of >= min_run words that are odd ROM pointers."""
        d, n, found = self.rom.d, len(self.rom.d) // 4, 0
        run: list[int] = []
        for i in range(n + 1):
            v = struct.unpack_from("<I", d, i * 4)[0] if i < n else 0
            if self.rom.in_rom(v) and v & 1 and (v - 1) % 2 == 0:
                run.append(v)
                continue
            if len(run) >= min_run:
                found += 1
                for p in run:
                    self.seed(p, True, "ptr_table")
            run = []
        return found


def ce_code_symbols() -> list[tuple[int, str]]:
    if not CE_CSV.exists():
        return []
    out = []
    with CE_CSV.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            if row["kind"] == "code":
                try:
                    a = int(row["address"], 16)
                except ValueError:
                    continue
                if BASE <= a < BASE + 0x800000:
                    out.append((a, row["name"]))
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("version", nargs="?", default="rs", choices=list(ROMS))
    ap.add_argument("--code-end", type=lambda x: int(x, 0), default=0x08120000,
                    help="seeds at/after this address are rejected as data (DC-1: 0x08120000)")
    args = ap.parse_args()

    rom = Rom(ROMS[args.version])
    s = Survey(rom)
    s.code_end = args.code_end

    # ROM entry: ARM branch at 0x08000000.
    ins = next(s.arm.disasm(rom.d[:4], BASE))
    s.seed(imm_target(ins.op_str), False, "entry")
    s.run()
    after_entry = len(s.funcs)

    ce = ce_code_symbols() if args.version == "rs" else []
    for a, name in ce:
        s.seed(a | 1, True, "ce_symbol")
    s.run()
    after_ce = len(s.funcs)

    tables = s.pointer_tables()
    s.run()
    prologues = s.prologue_pass()
    print(f"prologue pass: {prologues} gap-start functions seeded")

    ok = sum(1 for f in s.funcs.values() if f["ok"])
    code_bytes = len(s.code) * 2
    hi = max(s.code) if s.code else BASE
    by_source = Counter(f["source"] for f in s.funcs.values())
    m4a = DECOMP / "notes"
    m4a.mkdir(exist_ok=True)

    out = DECOMP / "symbols" / f"functions_{args.version}.txt"
    out.parent.mkdir(exist_ok=True)
    with out.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# addr mode size source ok   (generated by tools/survey.py; DC-1, not final)\n")
        for a in sorted(s.funcs):
            fi = s.funcs[a]
            f.write(f"{a:08X} {'T' if fi['thumb'] else 'A'} {fi['end'] - a:5d} {fi['source']} {int(fi['ok'])}\n")

    ce_hit = sum(1 for a, _ in ce if a in s.funcs)
    print(f"version={args.version}")
    print(f"functions={len(s.funcs)} (clean={ok}); from entry={after_entry}, +CE={after_ce - after_entry}, "
          f"+ptr tables={len(s.funcs) - after_ce} ({tables} tables)")
    print(f"by source: {dict(by_source)}")
    print(f"code bytes decoded={code_bytes} ({code_bytes / len(rom.d) * 100:.1f}% of ROM), "
          f"highest code address=0x{hi:08X}")
    region = args.code_end - BASE
    in_region = sum(1 for h in s.code if h < args.code_end) * 2
    lits = sum(4 for l in s.literals if l < args.code_end)
    print(f"code region 0x{BASE:08X}-0x{args.code_end:08X} ({region} bytes): decoded code {in_region} "
          f"({in_region / region * 100:.1f}%), literal pools {lits} ({lits / region * 100:.1f}%), "
          f"rest (data, unreached code, padding) {region - in_region - lits} "
          f"({(region - in_region - lits) / region * 100:.1f}%)")
    print(f"seeds rejected beyond code end: {s.rejected}")
    print(f"literal pool words={len(s.literals)}")
    print(f"unresolved: {dict(s.unresolved)}; calls/pointers into RAM code: {dict(s.ram_calls)}")
    if ce:
        print(f"CE code symbols: {len(ce)}, of which are function starts we reached: {ce_hit}")
    print(f"wrote {out.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
