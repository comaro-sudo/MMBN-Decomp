#!/usr/bin/env python3
"""Map BN4's text-script commands to their handler routines.

    python tools/script_cmds.py [rs|bm] [--names]

The script engine dispatches a command's first byte (0xE5-0xFF) through a main table (RS 0x0804E900),
and a group byte such as 0xEE through a second table indexed by the next byte (the group's dispatcher
loads it: ldr r0,=table; ldrb r1,[r4,#1]; lsls r1,#2; ldr r0,[r0,r1]; mov lr,pc; bx r0).
Command names and byte patterns come from TextPet's BN4 plugin (tools/textpet/plugins/mmbn4.ini, local).
Prints one line per handler; --names prints names.txt rows instead (addresses for both versions when the
other ROM is present, via symbols/map_rs_bm.txt).
"""
from __future__ import annotations

import argparse
import re
import struct
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DECOMP = ROOT / "decomp"
BASE = 0x08000000
ROMS = {"rs": ROOT / "roms/bn4_rs_us.gba", "bm": ROOT / "roms/bn4_bm_us.gba"}
INI = ROOT / "tools/textpet/plugins/mmbn4.ini"
FIRST_OP = 0xE5
# The main table is found from a known group dispatcher's pointer: RS 0x0804F00C handles 0xEE.
MAIN_TABLE = {"rs": 0x0804E900, "bm": None}


def commands() -> list[tuple[str, list[int], list[int]]]:
    out = []
    for blk in INI.read_text(encoding="utf-8").split("[Command]")[1:]:
        n = re.search(r"^name = (\S+)", blk, re.M)
        b = re.search(r"^base = ([0-9A-F ]+)", blk, re.M)
        m = re.search(r"^mask = ([0-9A-F ]+)", blk, re.M)
        if n and b and m:
            out.append((n.group(1), [int(x, 16) for x in b.group(1).split()], [int(x, 16) for x in m.group(1).split()]))
    return out


class Rom:
    def __init__(self, v: str):
        self.d = ROMS[v].read_bytes()

    def u16(self, a: int) -> int:
        return struct.unpack_from("<H", self.d, a - BASE)[0]

    def u32(self, a: int) -> int:
        return struct.unpack_from("<I", self.d, a - BASE)[0]


def find_main_table(rom: Rom, v: str) -> int:
    if MAIN_TABLE.get(v):
        return MAIN_TABLE[v]
    # Same table in the other version: find the run of 27 Thumb pointers into the engine whose shape matches.
    rs = Rom("rs")
    ref = [rs.u32(MAIN_TABLE["rs"] + 4 * k) for k in range(0x100 - FIRST_OP)]
    deltas = (0, 4, 8, 0xC, 0x14)
    for d in deltas:
        cand = MAIN_TABLE["rs"] + d
        got = [rom.u32(cand + 4 * k) for k in range(0x100 - FIRST_OP)]
        if all(g - r in deltas for g, r in zip(got, ref)):
            return cand
    raise SystemExit(f"{v}: main script table not found")


def group_table(rom: Rom, handler: int) -> int | None:
    """The sub-table a group dispatcher loads (pattern above), or None for a plain handler."""
    a = handler & ~1
    for k in range(0, 12, 2):
        hw = rom.u16(a + k)
        if (hw & 0xF800) == 0x4800 and (hw >> 8) & 7 == 0:          # ldr r0, [pc, #imm]
            lit = ((a + k + 4) & ~3) + (hw & 0xFF) * 4
            nxt = rom.u16(a + k + 2)
            if nxt == 0x7861:                                          # ldrb r1, [r4, #1]
                return rom.u32(lit)
    return None


def build(v: str) -> dict[int, list[str]]:
    rom = Rom(v)
    main = find_main_table(rom, v)
    by_op = defaultdict(list)
    for name, base, mask in commands():
        by_op[base[0]].append((name, base, mask))
    handlers: dict[int, list[str]] = defaultdict(list)
    for op in range(FIRST_OP, 0x100):
        h = rom.u32(main + 4 * (op - FIRST_OP))
        sub = group_table(rom, h)
        if sub is None:
            names = [n for n, b, m in by_op.get(op, []) if len(m) < 2 or m[1] != 0xFF] or [n for n, b, m in by_op.get(op, [])]
            handlers[h & ~1].append(f"{op:02X}:" + "/".join(sorted(set(names)) or ["?"]))
        else:
            handlers[h & ~1].append(f"{op:02X}:group")
            for name, base, mask in by_op.get(op, []):
                if len(base) > 1 and len(mask) > 1 and mask[1] != 0:
                    sh = rom.u32(sub + 4 * base[1])
                    handlers[sh & ~1].append(f"{op:02X} {base[1]:02X}:{name}")
    return handlers


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("version", nargs="?", default="rs", choices=["rs", "bm"])
    ap.add_argument("--names", action="store_true")
    a = ap.parse_args()
    hs = build(a.version)
    if not a.names:
        for addr in sorted(hs):
            print(f"{addr:08X}  {', '.join(hs[addr])}")
        return
    other = build("bm" if a.version == "rs" else "rs") if ROMS["bm"].exists() else {}
    # pair by the same command list
    key = {tuple(sorted(v)): k for k, v in other.items()}
    for addr in sorted(hs):
        labels = hs[addr]
        first = labels[0].split(":", 1)[1].split("/")[0]
        sym = ("ScriptGroup_" + labels[0][:2] if first == "group"
               else "ScriptCmd_" + (first if first != "?" else labels[0][:2]))
        oaddr = key.get(tuple(sorted(labels)))
        rs_a, bm_a = (addr, oaddr) if a.version == "rs" else (oaddr, addr)
        print(f"{rs_a or 0:08X} {bm_a or 0:08X} code   CLAUDE       {sym:32s} script command handler: {', '.join(labels)}")


if __name__ == "__main__":
    main()
