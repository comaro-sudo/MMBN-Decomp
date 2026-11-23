#!/usr/bin/env python3
"""Disassemble one segment of config/segments_<v>.toml to versioned GNU-as source.

    python3 tools/disasm.py rs crt0 [--no-verify]

Classification inside the segment:
  - code: recursive descent from known function starts (symbols/functions_<v>.txt)
    and `entries`, or a linear decode up to `data_from` when `linear = true`
  - literal pool words referenced by pc-relative loads (labelled)
  - everything else: data (.word/.hword/.byte, raw)

Verification loop (default): regenerate rom.s, `make`, compare the segment's
bytes with the baserom; any instruction that GNU as encodes differently (or
rejects) is re-emitted as raw .hword/.word with the instruction as a comment.
Repeats until the segment matches byte for byte.
"""
from __future__ import annotations

import argparse
import re
import struct
import subprocess
import sys
import tomllib
from pathlib import Path

from capstone import CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB, Cs

DECOMP = Path(__file__).resolve().parents[1]
ROOT = DECOMP.parent
BASE = 0x08000000
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
COND = {"eq", "ne", "cs", "hs", "cc", "lo", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le"}
TARGET_RE = re.compile(r"#(0x[0-9a-f]+|\d+)$")


def is_branch(m: str) -> bool:
    return m in ("b", "bl") or (m[0] == "b" and m[1:] in COND)


def fname(a: int) -> str:
    return f"sub_{a:08X}"


def lname(a: int) -> str:
    return f"_{a:08X}"


class Segment:
    def __init__(self, v: str, name: str):
        cfg = tomllib.loads((DECOMP / "config" / f"segments_{v}.toml").read_text())
        segs = {s["name"]: s for s in cfg["segment"]}
        if name not in segs:
            sys.exit(f"no segment {name}")
        s = segs[name]
        self.v, self.name = v, name
        self.start, self.end = s["start"], s["end"]
        self.thumb = s.get("mode", "thumb") == "thumb"
        self.linear = s.get("linear", False)
        self.data_from = s.get("data_from", self.end)
        self.entries = s.get("entries", [])
        self.bm_delta = s.get("bm_delta", 0)
        self.rom = ROMS[v].read_bytes()
        self.jumptables: dict[int, list[int]] = {}
        for jt in s.get("jumptables", []):
            jt_addr = jt["addr"]
            jt_count = jt["count"]
            targets = []
            for i in range(jt_count):
                off = jt_addr - BASE + i * 4
                w = struct.unpack_from("<I", self.rom, off)[0]
                targets.append(w)
            self.jumptables[jt_addr] = targets
        self.arm_ranges: list[tuple[int, int]] = [(r["start"], r["end"]) for r in s.get("arm_ranges", [])]
        self.md = {True: Cs(CS_ARCH_ARM, CS_MODE_THUMB), False: Cs(CS_ARCH_ARM, CS_MODE_ARM)}
        self.funcs: dict[int, bool] = {}       # start -> thumb (inside this segment)
        self.allfuncs: dict[int, bool] = {}    # every known function start (for externs)
        fpath = DECOMP / "symbols" / f"functions_{v}.txt"
        if fpath.exists():
            for line in fpath.read_text().splitlines():
                if line.startswith("#"):
                    continue
                a, mode, *_ = line.split()
                a = int(a, 16)
                self.allfuncs[a] = mode == "T"
                if self.start <= a < self.data_from:
                    self.funcs[a] = mode == "T"
        for e in self.entries:
            self.funcs.setdefault(e, self.thumb)
        self.datarefs: set[int] = set()        # literal targets in the code region (all segments)
        dpath = DECOMP / "symbols" / f"datarefs_{v}.txt"
        if dpath.exists():
            self.datarefs = {int(l, 16) for l in dpath.read_text().split() if not l.startswith("#")
                             and all(c in "0123456789ABCDEFabcdef" for c in l)}
        self.datalabels: set[int] = set()      # dat_ labels defined in this segment
        self.insns: dict[int, tuple] = {}      # addr -> (insn, thumb)
        self.literals: set[int] = set()
        self.labels: set[int] = set()
        self.externs: set[int] = set()
        self.raw: set[int] = set()             # addresses forced to raw
        self.raw_lits: set[int] = set()        # literal words forced to raw values
        self.names: dict[int, str] = {}
        names_path = DECOMP / "symbols" / "names.txt"
        if names_path.exists():
            for line in names_path.read_text().splitlines():
                if line.startswith("#") or not line.strip():
                    continue
                parts = line.split()
                if len(parts) >= 5:
                    addr_str = parts[0] if v == "rs" else parts[1]
                    sym_name = parts[4]
                    try:
                        self.names[int(addr_str, 16)] = sym_name
                    except ValueError:
                        pass

    def inside(self, a: int) -> bool:
        return self.start <= a < self.data_from

    def decode(self, a: int, thumb: bool):
        off = a - BASE
        insn = next(self.md[thumb].disasm(self.rom[off:off + 4], a), None)
        if insn is None or (thumb and insn.size == 4 and insn.mnemonic != "bl"):
            return None
        if thumb:
            hw = struct.unpack_from("<H", self.rom, off)[0]
            if (hw & 0xFFC0) == 0x0800:
                return None
            m, ops = insn.mnemonic, insn.op_str
            if m in ("bkpt", "udf"):
                return None
            if m == "cmp" and ("pc" in ops or "sp" in ops):
                return None
            if m == "stm" and "!" in ops and "{" in ops:
                base_r = ops.split("!")[0].strip()
                rlist = [r.strip() for r in ops.split("{")[1].rstrip("}").split(",")]
                if base_r in rlist and rlist[0] != base_r:
                    return None
        return insn

    def classify(self) -> None:
        if self.linear:
            a = self.start
            while a < self.data_from:
                insn = self.decode(a, self.thumb)
                if insn is None:
                    break
                self.insns[a] = (insn, self.thumb)
                self.note_refs(insn, self.thumb)
                a += insn.size
            return
        jt_targets = set()
        for jt_addr, targets in self.jumptables.items():
            self.labels.add(jt_addr)
            for t in targets:
                self.labels.add(t)
                jt_targets.add(t)

        for f, thumb in sorted(self.funcs.items()):
            work = [f]
            seen = set()
            while work:
                a = work.pop()
                prev = None
                while self.inside(a) and a not in seen:
                    insn = self.decode(a, thumb)
                    if insn is None or a in self.literals:
                        break
                    seen.add(a)
                    self.insns[a] = (insn, thumb)
                    self.note_refs(insn, thumb)
                    m, ops = insn.mnemonic, insn.op_str
                    if m == "b" or (m[0] == "b" and m[1:] in COND):
                        t = self.target(ops)
                        if t is not None and self.inside(t) and t not in seen:
                            work.append(t)
                        if m == "b":
                            break
                    elif m == "bx" and prev == ("mov", "lr, pc"):
                        pass                # call through a register: returns here
                    elif m == "bx" and prev and prev[0] == "adr" and (a + insn.size) in {s for s, _ in self.arm_ranges}:
                        pass                # Thumb-to-ARM inline switch (m4a SoundMainRAM): returns after arm_range
                    elif m == "bx" or (m == "pop" and "pc" in ops) or \
                            (m in ("mov", "add", "ldr") and ops.startswith("pc")):
                        break
                    prev = (m, ops)
                    a += insn.size
                    for arm_s, arm_e in self.arm_ranges:
                        if a == arm_s:
                            a = arm_e
                            break
            # Include trailing alignment nop (mov r8, r8 = 0x46C0) on _call_via_rX 2-byte bx trampolines
            if f in self.insns and self.insns[f][0].mnemonic == "bx" and (f + 2) not in self.insns and (f + 2) < self.data_from:
                if struct.unpack_from("<H", self.rom, f + 2 - BASE)[0] == 0x46C0:
                    nop_insn = self.decode(f + 2, thumb)
                    if nop_insn is not None:
                        self.insns[f + 2] = (nop_insn, thumb)

        # Trace jump table targets within this segment
        work = sorted(jt_targets)
        while work:
            a = work.pop()
            prev = None
            thumb = self.thumb
            while self.inside(a) and a not in self.insns:
                insn = self.decode(a, thumb)
                if insn is None or a in self.literals:
                    break
                self.insns[a] = (insn, thumb)
                self.note_refs(insn, thumb)
                m, ops = insn.mnemonic, insn.op_str
                if m == "b" or (m[0] == "b" and m[1:] in COND):
                    t = self.target(ops)
                    if t is not None and self.inside(t):
                        work.append(t)
                    if m == "b":
                        break
                elif m == "bx" and prev == ("mov", "lr, pc"):
                    pass
                elif m == "bx" or (m == "pop" and "pc" in ops) or \
                        (m in ("mov", "add", "ldr") and ops.startswith("pc")):
                    break
                prev = (m, ops)
                a += insn.size
        # Linearly decode explicit ARM ranges (e.g. m4a SoundMainRAM mixer loops)
        for arm_s, arm_e in self.arm_ranges:
            a = arm_s
            while a < arm_e:
                insn = self.decode(a, False)
                if insn is None:
                    break
                self.insns[a] = (insn, False)
                self.note_refs(insn, False)
                if insn.mnemonic == "bx" and (a + 4) == arm_e and self.inside(arm_e):
                    # Thumb continuation right after ARM block
                    work = [arm_e]
                    seen_cont = set()
                    while work:
                        ca = work.pop()
                        cprev = None
                        while self.inside(ca) and ca not in seen_cont and ca not in self.insns:
                            if any(rs <= ca < re for rs, re in self.arm_ranges):
                                break
                            cinsn = self.decode(ca, True)
                            if cinsn is None or ca in self.literals:
                                break
                            seen_cont.add(ca)
                            self.insns[ca] = (cinsn, True)
                            self.note_refs(cinsn, True)
                            cm, cops = cinsn.mnemonic, cinsn.op_str
                            if cm == "b" or (cm[0] == "b" and cm[1:] in COND):
                                ct = self.target(cops)
                                if ct is not None and self.inside(ct) and ct not in seen_cont:
                                    work.append(ct)
                                if cm == "b":
                                    break
                            elif cm == "bx" and cprev == ("mov", "lr, pc"):
                                pass
                            elif cm == "bx" or (cm == "pop" and "pc" in cops) or \
                                    (cm in ("mov", "add", "ldr") and cops.startswith("pc")):
                                break
                            cprev = (cm, cops)
                            ca += cinsn.size
                a += insn.size
        # Drop instructions that overlap a literal (descent ran into a pool).
        for a in list(self.insns):
            insn, _ = self.insns[a]
            if any(l <= a < l + 4 or a <= l < a + insn.size for l in self.literals):
                del self.insns[a]
        self.place_datalabels()

    def place_datalabels(self) -> None:
        """dat_ labels for data references that land on data in this segment
        (not on, or inside, an instruction)."""
        covered = set()
        for a, (insn, _) in self.insns.items():
            covered.update(range(a, a + insn.size))
        self.datalabels = {d for d in self.datarefs if self.start <= d < self.end and d not in covered}
        self.labels |= self.datalabels

    @staticmethod
    def target(ops: str):
        m = TARGET_RE.search(ops)
        return int(m.group(1), 0) if m else None

    def note_refs(self, insn, thumb: bool) -> None:
        m, ops = insn.mnemonic, insn.op_str
        if is_branch(m):
            t = self.target(ops)
            if t is None:
                return
            if self.start <= t < self.end:
                self.labels.add(t)
            else:
                self.externs.add(t)
        elif m == "ldr" and "[pc" in ops:
            imm = self.target(ops.rstrip("]")) or 0
            lit = ((insn.address + (4 if thumb else 8)) & ~3) + imm
            if self.start <= lit < self.end:
                self.literals.add(lit)
                self.labels.add(lit)

    def render(self, a: int, insn, thumb: bool) -> str:
        m, ops = insn.mnemonic, insn.op_str
        raw = self.raw_line(a, insn.size, f"{m} {ops}".rstrip())
        if a in self.raw:
            return raw
        if is_branch(m):
            t = self.target(ops)
            if t is None:
                return raw
            if self.start <= t < self.end:
                if t not in self.insns:
                    return raw              # branch into data: keep exact bytes
                return f"    {m} {self.label_for(t)}"
            # Outside the segment: only to known functions of the same mode
            # (the linker would otherwise fail, or add an interworking veneer).
            if m != "bl" or t not in self.allfuncs or self.allfuncs[t] != thumb:
                return raw
            sym_comment = f"   @ {self.names[t]}" if t in self.names else ""
            return f"    {m} {fname(t)}{sym_comment}"
        if m == "ldr" and "[pc" in ops:
            imm = self.target(ops.rstrip("]")) or 0
            lit = ((a + (4 if thumb else 8)) & ~3) + imm
            if lit in self.literals:
                reg = ops.split(",")[0]
                return f"    ldr {reg}, {lname(lit)}"
            return raw
        if ("pc" in ops and m in ("add", "adr", "sub")) or m == "adr":
            return raw
        if thumb and m == "add":
            # Thumb-1 hi-register/SP add (ADD Rdn, Rm): Capstone prints three
            # operands ("add r2, sp, r2"); GNU as wants the two-operand form.
            o = [x.strip() for x in ops.split(",")]
            if len(o) == 3 and not o[2].startswith("#"):
                if o[0] == o[2]:
                    return f"    add {o[0]}, {o[1]}"
                if o[0] == o[1]:
                    return f"    add {o[0]}, {o[2]}"
        return f"    {m} {ops}".rstrip()

    def literal_value(self, a: int, w: int) -> str:
        """A literal pool word: a known function pointer becomes its symbol
        (the Thumb bit comes from the symbol), anything else stays a number."""
        if a in self.literals and a not in self.raw_lits:
            target, thumb = w & ~1, bool(w & 1)
            if self.allfuncs.get(target) is thumb and BASE <= target < BASE + 0x120000:
                sym_comment = f"   @ {self.names[target]}" if target in self.names else ""
                return f"{fname(target)}{sym_comment}"
            if w in self.datarefs:
                sym_comment = f"   @ {self.names[w]}" if w in self.names else ""
                return f"dat_{w:08X}{sym_comment}"
        return f"0x{w:08X}"

    def label_for(self, a: int) -> str:
        return fname(a) if a in self.funcs else lname(a)

    def raw_line(self, a: int, size: int, comment: str) -> str:
        b = self.rom[a - BASE:a - BASE + size]
        if size == 4 and not self.insns.get(a, (None, True))[1]:
            return f"    .word 0x{struct.unpack('<I', b)[0]:08X}   @ {comment}"
        hws = struct.unpack(f"<{size // 2}H", b)
        return "    .hword " + ", ".join(f"0x{h:04X}" for h in hws) + f"   @ {comment}"

    def emit(self) -> tuple[str, dict[int, int]]:
        lines = [f"@ {self.name}: 0x{self.start:08X}-0x{self.end:08X} ({self.v.upper()})",
                 "@ GENERATED by tools/disasm.py, then verified byte-identical.", "",
                 f"    .set SEG_BM_DELTA, 0x{self.bm_delta:X}   @ Blue Moon = RS + this, inside this segment", ""]
        line_of: dict[int, int] = {}
        mode = None
        a = self.start
        while a < self.end:
            if a in self.insns:
                insn, thumb = self.insns[a]
                if mode != thumb:
                    lines.append("    .thumb" if thumb else "    .arm")
                    mode = thumb
                if a in self.funcs:
                    n = fname(a)
                    sym_comment = f"   @ {self.names[a]}" if a in self.names else ""
                    lines += ["", f"    .global {n}"]
                    if thumb:
                        lines.append("    .thumb_func")
                    lines.append(f"{n}:{sym_comment}")
                elif a in self.labels:
                    lines.append(f"{lname(a)}:")
                lines.append(self.render(a, insn, thumb))
                line_of[a] = len(lines)
                a += insn.size
                continue
            # data
            if a in self.datalabels:
                sym_comment = f"   @ {self.names[a]}" if a in self.names else ""
                lines += [f"    .global dat_{a:08X}", f"dat_{a:08X}:{sym_comment}"]
            if a in self.labels and (a in self.literals or a not in self.datalabels):
                lines.append(f"{lname(a)}:")
            if a in self.jumptables:
                jt_entries = self.jumptables[a]
                for target in jt_entries:
                    sym_comment = f"   @ {self.names[target]}" if target in self.names else ""
                    lines.append(f"    .word {lname(target)}{sym_comment}")
                    line_of[a] = len(lines)
                    a += 4
                continue
            # Unclassified runs (tables etc.) stay incbin from the player's ROM:
            # no bulk ROM data is written into the source. Labelled literal
            # words (constants/pointers used by code) are emitted as .word.
            run_end = a + 1
            while run_end < self.end and run_end not in self.insns and run_end not in self.labels:
                run_end += 1
            if a not in self.literals and run_end - a >= 8:
                lines.append(f'    incrom 0x{a - BASE:X}, 0x{run_end - a:X}')
                a = run_end
                continue
            if a % 4 == 0 and a + 4 <= self.end and not any(x in self.insns for x in range(a + 1, a + 4)):
                w = struct.unpack_from("<I", self.rom, a - BASE)[0]
                lines.append(f"    .word {self.literal_value(a, w)}")
                line_of[a] = len(lines)
                a += 4
            elif a % 2 == 0 and a + 2 <= self.end and a + 1 not in self.insns:
                h = struct.unpack_from("<H", self.rom, a - BASE)[0]
                lines.append(f"    .hword 0x{h:04X}")
                a += 2
            else:
                lines.append(f"    .byte 0x{self.rom[a - BASE]:02X}")
                a += 1
        lines.append("")
        return "\n".join(lines), line_of


def build(v: str) -> tuple[bool, str]:
    subprocess.run([sys.executable, str(DECOMP / "tools" / "split.py"), v], check=True,
                   stdout=subprocess.DEVNULL)
    r = subprocess.run(["make", "-C", str(DECOMP), f"VERSION={v}"], capture_output=True, text=True)
    return r.returncode == 0, r.stdout + r.stderr


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("version", choices=list(ROMS))
    ap.add_argument("segment")
    ap.add_argument("--no-verify", action="store_true")
    args = ap.parse_args()

    seg = Segment(args.version, args.segment)
    seg.classify()
    out = DECOMP / "asm" / ("" if args.version == "rs" else args.version) / f"{seg.name}.s"
    out.parent.mkdir(exist_ok=True)
    # GNU as reports paths relative to decomp/.  Keep the retry parser in
    # sync with the versioned output location (asm/code_*.s for RS,
    # asm/bm/code_*.s for Blue Moon, which split.py assembles from its renamed
    # copy build/bm/norm/asm/bm/code_*.s with the same line numbers).
    # DC-4: after `disasm.py rs X` and `disasm.py bm X`, run
    # `tools/merge_bm.py X` to fold the BM view back into the shared asm/X.s.
    out_rel = out.relative_to(DECOMP).as_posix()

    for attempt in range(1, 9):
        text, line_of = seg.emit()
        out.write_text(text, newline="\n")
        if args.no_verify:
            break
        ok, log = build(args.version)
        if not ok:
            errs = [(int(m.group(1)), m.group(2)) for m in
                    re.finditer(rf"(?:build/bm/norm/)?{re.escape(out_rel)}:(\d+): Error: ([^\n]*)", log)]
            # A rejected instruction emits no bytes, so later labels shift and
            # healthy pc-relative loads report "invalid offset". Fix primary
            # errors first; offset errors only when nothing else is left.
            primary = [e for e in errs if "invalid offset" not in e[1]]
            bad = {ln for ln, _ in (primary or errs)}
            hit = {a for a, ln in line_of.items() if ln in bad}
            if not hit:
                print(log[-3000:])
                sys.exit("build failed outside known instructions")
            seg.raw |= hit
            print(f"attempt {attempt}: {len(hit)} instruction(s) rejected by as -> raw")
            msgs = {}
            for m in re.finditer(rf"(?:build/bm/norm/)?{re.escape(out_rel)}:\d+: Error: ([^\n]*)", log):
                key = re.sub(r"`[^`]*'", "`...'", m.group(1))
                msgs.setdefault(key, m.group(1))
            for k, example in list(msgs.items())[:4]:
                print(f"    as: {example}")
            continue
        built = (DECOMP / "build" / args.version / f"bn4_{args.version}.gba").read_bytes()
        lo, hi = seg.start - BASE, seg.end - BASE
        diff = [i + BASE for i in range(lo, hi) if built[i] != seg.rom[i]]
        if not diff:
            n_code = len(seg.insns)
            print(f"{seg.name}: MATCH ({n_code} instructions, {len(seg.raw)} raw, "
                  f"{len(seg.literals)} literals, {len(seg.externs)} external refs)")
            break
        hit = {a for a, (insn, _) in seg.insns.items() if any(a <= d < a + insn.size for d in diff)}
        lits = {l for l in seg.literals if any(l <= d < l + 4 for d in diff)}
        if not hit and not lits:
            sys.exit(f"mismatch outside instructions at {', '.join(hex(d) for d in diff[:8])}")
        seg.raw |= hit
        seg.raw_lits |= lits
        print(f"attempt {attempt}: {len(hit)} instruction(s) / {len(lits)} literal(s) differ -> raw")
    else:
        sys.exit("did not converge")


if __name__ == "__main__":
    main()
