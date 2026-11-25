#!/usr/bin/env python3
"""Decode MMBN4 code_04..code_05 map-init story-progress dispatch tables into
symbolic cutscene_macros.inc invocations (cs_range / cs_jump).

    python3 tools/disasm_cutscene.py rs 0x08054540
    python3 tools/disasm_cutscene.py --scan-map-table rs
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
BASE = 0x08000000


def load_rom(version: str) -> bytes:
    name = "bn4_rs_us.gba" if version == "rs" else "bn4_bm_us.gba"
    return (DECOMP.parent / "roms" / name).read_bytes()


def decode_dispatch(rom: bytes, addr: int) -> tuple[list[str], int]:
    """Decode a contiguous sequence of 0x02 (range) and 0x01 (jump) opcodes.
    Returns (asm_lines, end_addr).
    """
    lines: list[str] = []
    cur = addr
    while True:
        off = cur - BASE
        if off + 5 > len(rom):
            break
        op = rom[off]
        if op == 0x02 and off + 7 <= len(rom):
            lo, hi = rom[off + 1], rom[off + 2]
            target = struct.unpack_from("<I", rom, off + 3)[0]
            if not (0x0804F000 <= target < 0x08095000):
                break
            lines.append(f"    cs_range 0x{lo:02X}, 0x{hi:02X}, dat_{target:08X}")
            cur += 7
        elif op == 0x01:
            target = struct.unpack_from("<I", rom, off + 1)[0]
            if not (0x0804F000 <= target < 0x08095000):
                break
            lines.append(f"    cs_jump dat_{target:08X}")
            cur += 5
            break  # unconditional jump terminates dispatch chain
        else:
            break
    return lines, cur


def scan_map_table(rom: bytes, version: str) -> None:
    # Master map-group script list in code_02 (RS 0x0802B1BC, BM +4)
    delta = 0 if version == "rs" else 4
    tbl = 0x0802B1BC + delta
    end = 0x0802BCCC + delta
    total_ops = 0
    total_bytes = 0
    tables = 0
    for a in range(tbl, end, 8):
        u, ptr = struct.unpack_from("<2I", rom, a - BASE)
        if not (0x0804F21C <= ptr < 0x08094758):
            continue
        # Scan up to +0x30 bytes after ptr for the first 0x02/0x01 dispatch chain
        for skip in range(0, 0x30):
            lines, end_a = decode_dispatch(rom, ptr + skip)
            if len(lines) >= 2:
                tables += 1
                total_ops += len(lines)
                total_bytes += end_a - (ptr + skip)
                break
    print(f"[{version}] scanned map-group list: {tables} dispatch tables, {total_ops} opcodes, {total_bytes} bytes decoded")


def main() -> None:
    if len(sys.argv) == 3 and sys.argv[1] == "--scan-map-table":
        v = sys.argv[2]
        scan_map_table(load_rom(v), v)
        return
    if len(sys.argv) != 3:
        sys.exit("usage: disasm_cutscene.py <rs|bm> <0xADDR> | --scan-map-table <rs|bm>")
    v, addr = sys.argv[1], int(sys.argv[2], 16)
    lines, end_a = decode_dispatch(load_rom(v), addr)
    print(f"@ dispatch table 0x{addr:08X}..0x{end_a:08X} ({end_a - addr} bytes, {len(lines)} ops)")
    for l in lines:
        print(l)


if __name__ == "__main__":
    main()
