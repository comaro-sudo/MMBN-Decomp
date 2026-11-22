#!/usr/bin/env python3
"""savetool.py: convert and inspect BN4 saves (format in docs/re/save-and-flags.md §1.2).

  python patch/tools/savetool.py FILE...              # summary of each save (.sav raw SRAM, or .sps SharkPort)
  python patch/tools/savetool.py --convert FILE...    # also write build/saves/<name>.sav (32 KB, mGBA/PC port)

The summary shows:
- the version (from the checksum constant: +0x16 Red Sun, +0x22 Blue Moon) and whether the checksum is valid;
- the playthrough, game time, Zenny, BugFrags and HP (current = game state +0x20, max = progress +0x14,
  max without NaviCust HP+ parts = progress +0x16);
- the Souls (flags 0x17-0x22) and the clear flag (0x500);
- the Library counts by class, read through the player's ROM chip table (flags 0x16C0 + id).

It prints numbers only. Converted saves go to build/ (gitignored)."""
import argparse
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRAM_USED, KEY_AT, SHIFT_AT, BLOCK_BASE = 0x73D2, 0x1554, 0x1550, 0x2130
PROGRESS, FLAGS, GAMESTATE = 0x84, 0x118, 0x0
CONST = {0x16: "Red Sun", 0x22: "Blue Moon"}
SOUL_NAMES = ("Roll", "Guts", "Wind", "Search", "Fire", "Thunder", "Proto", "Number", "Metal", "Junk", "Aqua", "Wood")
CHIP_TABLE, CHIP_SIZE, CHIP_COUNT = 0x197EC, 0x2C, 389


def read_sram(path):
    """Raw SRAM bytes from a .sav or a SharkPort .sps."""
    d = Path(path).read_bytes()
    if d[4:17] == b"SharkPortSave":
        p = 4 + struct.unpack_from("<I", d, 0)[0] + 4          # magic, platform
        meta = []
        for _ in range(3):                                      # title, date, notes
            n = struct.unpack_from("<I", d, p)[0]
            meta.append(d[p + 4:p + 4 + n].decode("latin-1")); p += 4 + n
        size = struct.unpack_from("<I", d, p)[0]; p += 4
        header, sram = d[p:p + 0x1C], d[p + 0x1C:p + size]
        return sram, {"title": meta[0], "date": meta[1], "notes": meta[2], "code": header[12:16].decode("latin-1")}
    return d, {}


def decode(sram):
    key = sram[KEY_AT]
    dec = bytearray(b ^ key for b in sram[:SRAM_USED])
    dec[KEY_AT:KEY_AT + 4] = sram[KEY_AT:KEY_AT + 4]
    shift = struct.unpack_from("<I", dec, SHIFT_AT)[0] & 0x1FE
    blk = BLOCK_BASE + shift
    prog = blk + PROGRESS
    stored = struct.unpack_from("<I", dec, prog + 0x34)[0]
    z = bytearray(dec); z[prog + 0x34:prog + 0x38] = b"\0\0\0\0"
    total = sum(z)
    version = next((v for c, v in CONST.items() if total + c == stored), None)
    return dec, blk, version, stored


def chip_classes():
    rom = ROOT / "roms" / "bn4_rs_us.gba"
    if not rom.exists():
        return None
    d = rom.read_bytes()
    out = []
    for i in range(CHIP_COUNT):
        r = d[CHIP_TABLE + i * CHIP_SIZE:][:CHIP_SIZE]
        number = struct.unpack_from("<H", r, 0x1C)[0]
        lib = r[8]
        if lib == 0 and number > 150:
            cls = "secret"
        else:
            cls = {0: "standard", 1: "mega", 2: "giga", 4: "pa"}.get(lib)
        out.append(cls if (r[9] & 0x40 or cls == "pa") else None)
    return out


def summary(path, classes):
    sram, meta = read_sram(path)
    dec, blk, version, stored = decode(sram)
    if dec[blk + PROGRESS + 0x54:blk + PROGRESS + 0x54 + 11] != b"ROCKMANEXE4":
        return f"{Path(path).name}: not a BN4 save (magic missing)"
    flags = dec[blk + FLAGS:blk + FLAGS + 0x3E0]
    fl = lambda n: bool(flags[n >> 3] & (0x80 >> (n & 7)))
    u32 = lambda a: struct.unpack_from("<I", dec, a)[0]
    u16 = lambda a: struct.unpack_from("<H", dec, a)[0]
    prog, gs = blk + PROGRESS, blk + GAMESTATE
    secs = u32(prog + 0x2C) // 60
    souls = [SOUL_NAMES[i] for i in range(12) if fl(0x17 + i)]
    lib = {}
    if classes:
        for i, c in enumerate(classes):
            if c and fl(0x16C0 + i):
                lib[c] = lib.get(c, 0) + 1
    libtxt = " ".join(f"{k}={lib.get(k, 0)}" for k in ("standard", "secret", "mega", "giga", "pa")) if classes else "(no ROM)"
    code = meta.get("code", "")
    return (f"{Path(path).name}: {version or 'CHECKSUM MISMATCH'}{' [' + code + ']' if code else ''}"
            f" playthrough={dec[prog + 0xD] + 1} time={secs // 3600}:{secs // 60 % 60:02d}"
            f" zenny={u32(gs + 0x74)} bugfrags={u32(gs + 0x78)} hp={u16(gs + 0x20)}/{u16(prog + 0x14)} (base {u16(prog + 0x16)})"
            f" cleared={'yes' if fl(0x500) else 'no'} souls={len(souls)}:{','.join(souls) or '-'}"
            f" library: {libtxt}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--convert", action="store_true")
    a = ap.parse_args()
    classes = chip_classes()
    out = ROOT / "build" / "saves"
    for f in a.files:
        print(summary(f, classes))
        if a.convert:
            sram, _ = read_sram(f)
            out.mkdir(parents=True, exist_ok=True)
            dst = out / (Path(f).stem + ".sav")
            dst.write_bytes(sram[:0x8000])
            print(f"  -> {dst.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
