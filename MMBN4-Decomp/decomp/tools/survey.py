#!/usr/bin/env python3
"""DC-1 survey: find functions by recursive descent and report code coverage.

    ~/.venvs/mmbn4-decomp/bin/python tools/survey.py [rs|bm] [--out notes/]

Seeds: the ROM entry (ARM), Cyberworld Endless code symbols (RS only),
odd ROM pointers found in literal pools, tables of >=4 consecutive Thumb
code pointers, and short two/three-entry tables whose every target starts
with a Thumb `push {..., lr}` prologue. Follows bl/b/bcc; stops at bx, pop
{pc}, mov/add pc.
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
from collections import Counter, defaultdict
from pathlib import Path

from capstone import CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB, Cs

ROOT = Path(__file__).resolve().parents[2]
DECOMP = ROOT / "decomp"
BASE = 0x08000000
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
CE_CSV = ROOT / "ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/symbols/bn4-red-sun-us.csv"

COND_B = {"beq", "bne", "bcs", "bhs", "bcc", "blo", "bmi", "bpl", "bvs", "bvc",
          "bhi", "bls", "bge", "blt", "bgt", "ble"}


def is_bad_thumb_insn(insn, hw: int) -> bool:
    """Return True for Thumb encodings that only occur when data is decoded as code."""
    if (hw & 0xFFC0) == 0x0800:  # lsrs Rd, Rm, #0x20 (upper halfword of 0x08XXXXXX pointer or 0x08 data)
        return True
    m, ops = insn.mnemonic, insn.op_str
    if m in ("bkpt", "udf"):
        return True
    if m == "svc":
        t = imm_target(ops)
        if t is None or t > 0x2A:
            return True
    if m == "cmp" and ("pc" in ops or "sp" in ops):
        return True
    if m == "stm" and "!" in ops and "{" in ops:
        base_r = ops.split("!")[0].strip()
        rlist = [r.strip() for r in ops.split("{")[1].rstrip("}").split(",")]
        if base_r in rlist and rlist[0] != base_r:
            return True
    if m in ("add", "mov", "ldr") and ops.startswith("pc"):
        if not (m == "mov" and ops != "pc, pc"):
            return True
    return False


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
        self.lit_values: set[int] = set()      # values loaded from literal pools
        self.unresolved = Counter()            # kind -> count
        self.unresolved_sites: dict[str, set[int]] = defaultdict(set)
        self.unresolved_origins = defaultdict(lambda: defaultdict(set))
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

    def note_unresolved(self, kind: str, address: int, origin: int) -> None:
        """Count an unresolved control-flow case and retain its site/origin."""
        self.unresolved[kind] += 1
        self.unresolved_sites[kind].add(address)
        self.unresolved_origins[kind][address].add(origin)

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
                hw = struct.unpack_from("<H", self.rom.d, off)[0]
                if insn is None or (thumb and insn.size == 4 and insn.mnemonic != "bl") or (thumb and is_bad_thumb_insn(insn, hw)):
                    info["ok"] = False
                    self.note_unresolved("invalid", a, start)
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
                        self.note_unresolved("bx_call", a, start)
                        prev = (m, ops)
                        a = nxt
                        continue
                    if not ops.startswith("lr"):
                        self.note_unresolved("bx_reg", a, start)
                    break
                elif m == "pop" and "pc" in ops:
                    break
                elif m == "mov" and ops == "pc, lr":
                    # Thumb return form. It stops this path but is not an
                    # unresolved computed jump or a jump-table dispatch.
                    break
                elif m in ("mov", "add", "ldr") and ops.startswith("pc"):
                    self.note_unresolved("jump_table_or_pc_write", a, start)
                    break
                elif m == "ldr" and "[pc" in ops:
                    imm = imm_target(ops.rstrip("]")) or 0
                    lit = ((a + (4 if thumb else 8)) & ~3) + imm
                    if self.rom.in_rom(lit):
                        self.literals.add(lit)
                        v = self.rom.u32(lit)
                        self.lit_values.add(v)
                        code_limit = self.code_end or self.rom.end
                        if self.rom.in_rom(v) and v & 1 and BASE + 0x25C <= (v & ~1) < code_limit and (struct.unpack_from("<H", self.rom.d, (v & ~1) - BASE)[0] & 0xFF00) == 0xB500 and self.verify_clean_thumb_func(v & ~1):
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
                        p not in self.funcs and self.verify_clean_thumb_func(p):
                    self.seed(p | 1, True, "prologue")
                    added += 1
                while a < end and a not in covered:
                    a += 2
            if not added:
                break
            self.run()
            added_total += added
        return added_total

    def verify_clean_thumb_func(self, addr: int, max_insns: int = 1500) -> bool:
        """Verify by control-flow walk that addr points to a valid Thumb function:
        all reachable instructions are valid ARMv4T Thumb (no data signatures or
        undefined encodings), and execution reaches a standard function return."""
        d = self.rom.d
        code_limit = self.code_end or self.rom.end
        if not (BASE + 0x25C <= addr < code_limit):
            return False
        if addr in self.lit_values or struct.unpack_from("<H", d, addr - BASE)[0] == 0:
            return False
        work, seen = [addr], set()
        has_ret = False
        while work and len(seen) < max_insns:
            a = work.pop()
            prev = None
            while len(seen) < max_insns:
                if a in seen:
                    has_ret = True
                    break
                if a != addr and a in self.funcs:
                    if not (prev and prev[0] == "pop" and (struct.unpack_from("<H", d, a - BASE)[0] & 0xFF87) == 0x4700):
                        return False
                if a != addr and a in self.code:
                    if not any(struct.unpack_from("<H", d, s_addr - BASE)[0] >> 8 == 0xB5 or
                               (struct.unpack_from("<H", d, s_addr - BASE)[0] & 0xF800) == 0xF000
                               for s_addr in seen):
                        return False
                    has_ret = True
                    break
                if a in self.literals:
                    return False
                o = a - BASE
                if not (0 <= o <= len(d) - 4):
                    return False
                hw = struct.unpack_from("<H", d, o)[0]
                insn = next(self.thumb.disasm(d[o:o+4], a), None)
                if insn is None or (insn.size == 4 and insn.mnemonic != "bl") or is_bad_thumb_insn(insn, hw):
                    return False
                seen.add(a)
                m, ops = insn.mnemonic, insn.op_str
                if m == "b":
                    t = imm_target(ops)
                    if t is not None and BASE + 0x25C <= t < code_limit:
                        if t in self.funcs or t in self.code or t in seen:
                            has_ret = True
                        else:
                            work.append(t)
                    else:
                        return False
                    break
                if m in COND_B:
                    t = imm_target(ops)
                    if t is not None and BASE + 0x25C <= t < code_limit:
                        if t in self.funcs or t in self.code or t in seen:
                            has_ret = True
                        else:
                            work.append(t)
                    else:
                        return False
                elif m == "pop" and "pc" in ops:
                    has_ret = True
                    break
                elif m == "mov" and ops.startswith("pc,") and ops != "pc, pc":
                    has_ret = True
                    break
                elif m == "bx":
                    if prev == ("mov", "lr, pc"):
                        prev = (m, ops)
                        a += insn.size
                        continue
                    has_ret = True
                    break
                elif m in ("mov", "add", "ldr") and ops.startswith("pc"):
                    return False
                prev = (m, ops)
                a += insn.size
        return has_ret

    def verify_leaf_thumb_func(self, addr: int, max_insns: int = 48) -> bool:
        """Verify by control-flow walk that addr is a clean leaf Thumb function:
        no BL calls, no unpushed pop {..., pc}, all paths return via mov pc, lr
        (0x46F7) or bx lr (0x4770), and never walks linearly into existing code."""
        d = self.rom.d
        code_limit = self.code_end or self.rom.end
        if not (BASE + 0x25C <= addr < code_limit):
            return False
        if addr in self.lit_values or addr in self.literals or addr in self.funcs or addr in self.code:
            return False
        first_hw = struct.unpack_from("<H", d, addr - BASE)[0]
        if first_hw == 0 or first_hw in (0x46F7, 0x4770, 0x46C0):
            return False
        work, seen = [addr], set()
        has_ret = False
        while work and len(seen) < max_insns:
            a = work.pop()
            while len(seen) < max_insns:
                if a in seen:
                    break
                if a != addr and (a in self.funcs or a in self.code):
                    return False
                if a in self.literals:
                    return False
                o = a - BASE
                if not (0 <= o <= len(d) - 4):
                    return False
                hw = struct.unpack_from("<H", d, o)[0]
                insn = next(self.thumb.disasm(d[o:o+4], a), None)
                if insn is None or insn.size == 4 or is_bad_thumb_insn(insn, hw):
                    return False
                seen.add(a)
                m, ops = insn.mnemonic, insn.op_str
                if m == "b":
                    t = imm_target(ops)
                    if t is not None and addr <= t < addr + max_insns * 2:
                        if t not in seen:
                            work.append(t)
                    else:
                        return False
                    break
                if m in COND_B:
                    t = imm_target(ops)
                    if t is not None and addr <= t < addr + max_insns * 2:
                        if t not in seen:
                            work.append(t)
                    else:
                        return False
                elif m == "pop" and "pc" in ops:
                    return False
                elif hw in (0x46F7, 0x4770):
                    has_ret = True
                    break
                elif m == "bx" or (m in ("mov", "add", "ldr") and ops.startswith("pc")):
                    return False
                a += insn.size
        return has_ret and len(seen) >= 2

    def leaf_gap_pass(self) -> int:
        """Seed unreferenced leaf functions (returning via `mov pc, lr` or `bx lr`)
        and `_call_via_rX` trampolines (`bx rX; nop`) that sit at the start of an
        undecoded gap (immediately following a previous function/literal pool and
        optional 0x0000 alignment padding)."""
        added_total = 0
        end = self.code_end or self.rom.end
        d = self.rom.d
        for _ in range(16):
            covered = set(self.code)
            for l in self.literals:
                covered |= {l, l + 2}
            added = 0
            a = BASE + 0x25C
            while a < end:
                if a in covered:
                    a += 2
                    continue
                gs = a
                ge = a + 2
                while ge < end and ge not in covered:
                    ge += 2
                p = gs
                while p < ge and struct.unpack_from("<H", d, p - BASE)[0] in (0, 0x46C0):
                    p += 2
                if p < ge:
                    hw0 = struct.unpack_from("<H", d, p - BASE)[0]
                    hw1 = struct.unpack_from("<H", d, p - BASE + 2)[0] if p + 2 < ge else 0
                    if (hw0 & 0xFF87) == 0x4700 and hw1 == 0x46C0 and p not in self.funcs:
                        self.seed(p | 1, True, "call_via_rx")
                        self.code.add(p + 2)
                        added += 1
                    elif hw0 == 0x46F7 and p not in self.funcs and p not in self.literals and p not in self.lit_values and \
                            struct.unpack_from("<I", d, 0)[0] and (d.find(struct.pack("<I", p | 1)) != -1):
                        self.seed(p | 1, True, "noop_stub")
                        added += 1
                    elif self.verify_leaf_thumb_func(p):
                        self.seed(p | 1, True, "leaf_gap")
                        added += 1
                    elif any((struct.unpack_from("<H", d, x - BASE)[0] & 0xF800) == 0xF000 and
                             (struct.unpack_from("<H", d, x - BASE + 2)[0] & 0xF800) == 0xF800
                             for x in range(p, min(ge - 2, p + 64), 2)) and \
                            any(struct.unpack_from("<H", d, x - BASE)[0] >> 8 == 0xB5 for x in range(p, min(ge, p + 32), 2)) and \
                            self.verify_clean_thumb_func(p):
                        self.seed(p | 1, True, "late_push_gap")
                        added += 1
                a = ge
            if not added:
                break
            self.run()
            added_total += added
        return added_total

    def clean_gap_prologue_pass(self) -> int:
        """Scan undecoded gaps for Thumb push {..., lr} or preceded push {r...}
        that decode cleanly to a valid function return. This recovers functions
        situated inside gaps after data tables or unreferenced dispatch blocks."""
        added_total = 0
        end = self.code_end or self.rom.end
        d = self.rom.d
        for _ in range(4):
            covered = set(self.code)
            for l in self.literals:
                covered |= {l, l + 2}
            added = 0
            a = BASE + 0x25C
            while a < end:
                if a in covered or a in self.funcs:
                    a += 2
                    continue
                val = struct.unpack_from("<H", d, a - BASE)[0]
                if (val & 0xFF00) == 0xB500:
                    if self.verify_clean_thumb_func(a):
                        self.seed(a | 1, True, "clean_prologue")
                        added += 1
                elif (val & 0xFF00) == 0xB400:
                    prev_h = struct.unpack_from("<H", d, a - BASE - 2)[0] if a > BASE else 0
                    if prev_h == 0 or prev_h == 0x46F7 or prev_h == 0x4770 or (prev_h & 0xFF00) == 0xBD00 or (prev_h & 0xF800) == 0xE000:
                        if self.verify_clean_thumb_func(a):
                            self.seed(a | 1, True, "preceded_b4_push")
                            added += 1
                a += 2
            if not added:
                break
            self.run()
            added_total += added
        return added_total

    def pointer_tables(self, min_run: int = 4) -> int:
        """Seed from runs of >= min_run words that are clean odd ROM code pointers."""
        d, n, found = self.rom.d, len(self.rom.d) // 4, 0
        code_limit = self.code_end or self.rom.end
        run: list[int] = []
        for i in range(n + 1):
            v = struct.unpack_from("<I", d, i * 4)[0] if i < n else 0
            if (BASE + 0x25C <= (v & ~1) < code_limit) and (v & 1) and (v & ~1) not in self.literals and self.verify_clean_thumb_func(v & ~1):
                run.append(v)
                continue
            if len(run) >= min_run:
                found += 1
                for p in run:
                    self.seed(p, True, "ptr_table")
            run = []
        return found

    def short_prologue_pointer_tables(self) -> tuple[int, int]:
        """Seed conservative two/three-entry Thumb callback tables.

        The broad pointer-table pass deliberately requires four adjacent ROM
        pointers to avoid treating arbitrary data as code.  Real callback and
        state tables commonly contain only two or three entries, though.  A
        short run is admitted only when *all* its targets begin with a Thumb
        `push {..., lr}` instruction and the targets are not already decoded
        code or literal-pool words.  This gives the same structural evidence
        as a normal function entry without making single pointer-shaped data
        a seed.

        Returns `(tables, targets)` for the survey report.
        """
        d, n = self.rom.d, len(self.rom.d) // 4
        tables = targets = 0
        run: list[int] = []
        for i in range(n + 1):
            v = struct.unpack_from("<I", d, i * 4)[0] if i < n else 0
            if self.rom.in_rom(v) and v & 1:
                run.append(v)
                continue
            if 2 <= len(run) <= 3:
                starts = [p & ~1 for p in run]
                code_limit = self.code_end or self.rom.end
                has_anchor = any(
                    BASE + 0x25C <= start < code_limit
                    and start not in self.literals
                    and (start in self.funcs or ((struct.unpack_from("<H", d, start - BASE)[0] & 0xFF00) == 0xB500 and self.verify_clean_thumb_func(start)))
                    for start in starts
                )
                valid = has_anchor and all(
                    BASE + 0x25C <= start < code_limit
                    and start not in self.literals
                    and (
                        start in self.funcs
                        or ((struct.unpack_from("<H", d, start - BASE)[0] & 0xFF00) == 0xB500 and self.verify_clean_thumb_func(start))
                        or self.verify_leaf_thumb_func(start)
                        or struct.unpack_from("<H", d, start - BASE)[0] == 0x46F7
                    )
                    for start in starts
                ) and any(start not in self.funcs for start in starts)
                if valid:
                    tables += 1
                    for p in run:
                        before = len(self.funcs)
                        self.seed(p, True, "short_ptr_table")
                        targets += len(self.funcs) - before
            run = []
        return tables, targets

    def sparse_pointer_tables(self) -> tuple[int, int]:
        """Seed callback tables that contain interleaved null pointers (0x00000000).

        Sparse tables occur in opcode dispatchers where unassigned opcodes have null
        table entries. Admitted only when all non-zero entries verify as clean Thumb
        routines.
        """
        d, n = self.rom.d, len(self.rom.d) // 4
        code_limit = self.code_end or self.rom.end
        tables = targets = 0
        run: list[int] = []
        for i in range(n + 1):
            w = struct.unpack_from("<I", d, i * 4)[0] if i < n else 0xDEADBEEF
            is_odd_rom = (BASE + 0x25C <= w < code_limit) and (w & 1)
            is_null = (w == 0)
            if is_odd_rom or is_null:
                run.append(w)
                continue
            odd_ptrs = [w for w in run if w != 0]
            null_ptrs = [w for w in run if w == 0]
            if 4 <= len(run) <= 64 and len(odd_ptrs) >= 2 and len(null_ptrs) >= 1:
                starts = [w & ~1 for w in odd_ptrs]
                if all(start not in self.literals and self.verify_clean_thumb_func(start) for start in starts) and any(start not in self.funcs for start in starts):
                    tables += 1
                    for p in odd_ptrs:
                        before = len(self.funcs)
                        self.seed(p, True, "sparse_ptr_table")
                        targets += len(self.funcs) - before
            run = []
        return tables, targets


def ce_code_symbols(version: str = "rs") -> list[tuple[int, str]]:
    out = []
    seen = set()
    names_path = DECOMP / "symbols" / "names.txt"
    if names_path.exists():
        for line in names_path.read_text(encoding="utf-8").splitlines():
            if line.startswith("#") or not line.strip():
                continue
            parts = line.split()
            if len(parts) >= 5 and parts[2] == "code":
                addr_str = parts[0] if version == "rs" else parts[1]
                try:
                    a = int(addr_str, 16)
                except ValueError:
                    continue
                if BASE <= a < BASE + 0x800000 and a not in seen:
                    seen.add(a)
                    out.append((a, parts[4]))
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("version", nargs="?", default="rs", choices=list(ROMS))
    ap.add_argument("--code-end", type=lambda x: int(x, 0), default=None,
                    help="seeds at/after this address are rejected as data (0x08113CB8 RS / 0x08113CCC BM)")
    args = ap.parse_args()
    if args.code_end is None:
        args.code_end = 0x08113CCC if args.version == "bm" else 0x08113CB8

    rom = Rom(ROMS[args.version])
    s = Survey(rom)
    s.code_end = args.code_end

    # ROM entry: ARM branch at 0x08000000.
    ins = next(s.arm.disasm(rom.d[:4], BASE))
    s.seed(imm_target(ins.op_str), False, "entry")
    s.run()
    after_entry = len(s.funcs)

    tables = s.pointer_tables()
    s.run()
    short_tables, short_targets = s.short_prologue_pointer_tables()
    s.run()
    sparse_tables, sparse_targets = s.sparse_pointer_tables()
    s.run()
    prologues = s.prologue_pass()
    print(f"prologue pass: {prologues} gap-start functions seeded")
    clean_prologues = s.clean_gap_prologue_pass()
    print(f"clean gap prologue pass: {clean_prologues} functions seeded")
    leaf_gaps = s.leaf_gap_pass()
    print(f"leaf gap pass: {leaf_gaps} functions seeded")
    short_tables2, short_targets2 = s.short_prologue_pointer_tables()
    if short_targets2:
        s.run()
        short_tables += short_tables2
        short_targets += short_targets2
        leaf_gaps += s.leaf_gap_pass()

    # Seed named code symbols only when they are not mid-function instruction hooks
    # inside an already-decoded function.
    before_ce = len(s.funcs)
    ce = ce_code_symbols(args.version)
    for a, name in ce:
        start = a & ~1
        if start not in s.code and start not in s.literals and s.verify_clean_thumb_func(start):
            s.seed(start | 1, True, "ce_symbol")
    s.run()
    after_ce = len(s.funcs)
    if after_ce > before_ce:
        prologues += s.prologue_pass()
        clean_prologues += s.clean_gap_prologue_pass()
        leaf_gaps += s.leaf_gap_pass()

    # Drop any seed that hit an invalid instruction during descent.
    for a in [a for a, fi in s.funcs.items() if not fi["ok"]]:
        del s.funcs[a]

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

    # Data references: literal values pointing into the code region that are
    # not function starts (tables inside the code region, struct fields...).
    code_end = args.code_end
    datarefs = sorted(v for v in s.lit_values
                      if BASE <= v < code_end and not ((v & ~1) in s.funcs and v & 1))
    dpath = DECOMP / "symbols" / f"datarefs_{args.version}.txt"
    with dpath.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# literal values pointing into the code region (not function starts); tools/survey.py\n")
        for v in datarefs:
            f.write(f"{v:08X}\n")
    print(f"data references into the code region: {len(datarefs)} -> {dpath.relative_to(ROOT)}")

    ce_hit = sum(1 for a, _ in ce if (a & ~1) in s.funcs or (a & ~1) in s.code)
    print(f"version={args.version}")
    print(f"functions={len(s.funcs)} (clean={ok}); from entry={after_entry}, +CE={after_ce - before_ce}, "
          f"+tables/prologues={before_ce - after_entry} ({tables} long tables, "
          f"{short_tables} short tables/{short_targets} new targets, "
          f"{sparse_tables} sparse tables/{sparse_targets} new targets)")
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
