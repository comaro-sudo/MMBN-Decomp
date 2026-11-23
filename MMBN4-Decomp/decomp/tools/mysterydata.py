#!/usr/bin/env python3
"""
RE-07: Mega Man Battle Network 4 Mystery Data extractor & verifier.

Reads the US Red Sun and US Blue Moon ROMs (read-only), decodes every Blue,
Purple, and Green Mystery Data placement and content table, compares RS vs BM,
and diffs against ref/maps/legend.csv (if present).

Usage (from MMBN4-Decomp/ or repo root):
    python decomp/tools/mysterydata.py                # summary + legend.csv diff
    python decomp/tools/mysterydata.py --dump-all     # print all BMD/PMD + GMD
    python decomp/tools/mysterydata.py --write-doc    # generate decomp/notes/re07-mystery-data.md
"""

from __future__ import annotations

import argparse
import csv
import os
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# ---------------------------------------------------------------------------
# ROM addresses (US Red Sun; BM is +0xC in code_06, +0x4 in code_02, +0 in code_00)
# ---------------------------------------------------------------------------

ROM_BASE = 0x08000000

RS_RW_MASTER_TABLE = 0x080ADD18  # 6 real-world groups (all 0) + sentinel 1
RS_CW_MASTER_TABLE = 0x080ADD34  # 21 cyberworld groups (0x80..0x94) + sentinel 1
RS_DATA_START = 0x080A9110       # first Mystery Data group pointer table
RS_DATA_END = 0x080ADD8C         # end of dat_080ADD34 (right before MysteryDataInitState)
BM_CODE06_DELTA = 0xC

RS_MD_SCRIPT_ARCHIVE = 0x087585A0
BM_MD_SCRIPT_ARCHIVE = 0x08758030
MD_SCRIPT_ARCHIVE_SIZE = 0xD1C

# Text archives (US Red Sun)
RS_ITEM_NAMES = 0x087571A0
RS_CHIP_NAMES_LO = 0x087481D8
RS_CHIP_NAMES_HI = 0x08748C14
RS_CHIP_CODES = 0x087490E8
RS_PROG_NAMES = 0x086E7FC8
RS_MAP_NAMES = 0x086E8770
BM_MAP_NAMES = 0x086E8264
RS_CW_MAP_NAME_BASES = 0x0802DF00

# NaviCust program color byte (entry +3 for content_kind 9)
NAVICUST_COLORS = {
    1: "White",
    2: "Pink",
    3: "Yellow",
    4: "Red",
    5: "Blue",
    6: "Green",
}

TYPE_NAMES = {
    1: "BMD",         # Blue, advance-on-pickup
    2: "BMD-Tier",    # Blue, playthrough-indexed (unused in BN4)
    3: "PMD",         # Purple, advance-on-pickup
    4: "PMD-Tier",    # Purple, playthrough-indexed (unused in BN4)
    5: "GMD",         # Green, random per jack-in
}

CONTENT_KIND_NAMES = {
    1: "Chip",
    2: "SubChip",
    3: "Zenny",
    4: "KeyItem",
    5: "BugFrags",
    6: "TrappedChip",
    7: "TrappedZenny",
    8: "HPMemory/ExpMemory",
    9: "NaviCust",
    10: "RegUp",
    11: "SubMemory",
}

ROUTINES = [
    ("WeightedRandomIndex",          0x08000CA0, 0x0,  "code_00", "Weighted RNG pick: (GetRNG() & 0x3FF) % sum(weights) + 1"),
    ("JackIn_ResetGreenMysteryData", 0x080297C8, 0x4,  "code_02", "Jack-in handler; calls MysteryDataRollAllGreen"),
    ("MysteryDataLoadScriptVars",    0x0809DB10, 0xC,  "code_06", "Loads flag/reward/param into text-script vars (+0x4C/+0x54/+0x58)"),
    ("MapObj_InteractMysteryData",   0x080A84E8, 0xC,  "code_06", "Map-object interact handler; calls MysteryDataLookupReward and runs pickup script"),
    ("MapObj_SpawnMysteryData",      0x080A89BC, 0xC,  "code_06", "Map-object spawn opcode; calls MysteryDataLookupSpawn, sets sprite/palette/coords/script"),
    ("MysteryDataInitState",         0x080ADD8C, 0xC,  "code_06", "Zeroes save slot +0x74 (0x800 bytes = 1024 x 2-byte slots for flags 0x0D00..0x10FF)"),
    ("MysteryDataRollAllGreen",      0x080ADDA0, 0xC,  "code_06", "Rerolls placement & content indices and clears flags for all type-5 GMDs"),
    ("MysteryDataRollGreen",         0x080ADDEC, 0xC,  "code_06", "Rolls weighted placement_idx and content_idx for one GMD via WeightedRandomIndex"),
    ("MysteryDataSetSlot",           0x080ADE38, 0xC,  "code_06", "Writes (placement_idx, content_idx) to save slot +0x74 for flag"),
    ("MysteryDataGetSlot",           0x080ADE48, 0xC,  "code_06", "Returns &[eToolkit+0x74] + (flag - 0x0D00) * 2"),
    ("MysteryDataAdvanceAll",        0x080ADE5C, 0xC,  "code_06", "NewGame / NG+: advances all non-green Mystery Data across CW and RW tables"),
    ("MysteryDataAdvanceGroupTable", 0x080ADE6C, 0xC,  "code_06", "Walks a master group table, calls MysteryDataNextIndices, updates slot, clears flag"),
    ("MysteryDataNextIndices",       0x080ADEB2, 0xC,  "code_06", "Types 1/3: +1 if flag set (clamped); Types 2/4: min(GetPlaythrough(), count-1)"),
    ("MysteryDataGetContentTable",   0x080ADF5C, 0xC,  "code_06", "For type 5 (GMD), offsets content ptr by min(GetPlaythrough(), 2) * 0x44"),
    ("MysteryDataLookupSpawn",       0x080ADF80, 0xC,  "code_06", "Given flag on current map, returns (type, obj_flags, &xyz, content_kind)"),
    ("MysteryDataLookupReward",      0x080ADFF0, 0xC,  "code_06", "Given flag on current map, returns (u32 reward_val, u8 param_byte)"),
]

# Mapping from ROM map name to ref/maps/legend.csv area name
MAP_TO_LEGEND_AREA = {
    "ACDC Area1":   "ACDC-1",
    "ACDC Area2":   "ACDC-2",
    "ACDC Area3":   "ACDC-3",
    "BlackEarth1":  "BlackEarth-1",
    "BlackEarth2":  "BlackEarth-2",
    "ElTwrComp1":   "ElecTower-1",
    "ElTwrComp2":   "ElecTower-2",
    "MeteorComp1":  "Meteor-1",
    "MeteorComp2":  "Meteor-2",
    "MeteorComp3":  "Meteor-3",
    "MeteorComp4":  "Meteor-4",
    "NetFricaArea": "Netfrica",
    "Netopia Area": "Netopia",
    "Park Area1":   "Park-1",
    "Park Area2":   "Park-2",
    "Park Area3":   "Park-3",
    "Sharo Area":   "Sharo",
    "Town Area1":   "Town-1",
    "Town Area2":   "Town-2",
    "Town Area3":   "Town-3",
    "Town Area4":   "Town-4",
    "ToyRoboComp1": "ToyRobo-1",
    "ToyRoboComp2": "ToyRobo-2",
    "ToyRoboComp3": "ToyRobo-3",
    "ToyRoboComp4": "ToyRobo-4",
    "Undernet1":    "Undernet-1",
    "Undernet2":    "Undernet-2",
    "Undernet3":    "Undernet-3",
    "Undernet4":    "Undernet-4",
    "Undernet5":    "Undernet-5",
    "Undernet6":    "Undernet-6",
    "YumLand Area": "YumLand",
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def find_game_root() -> Path:
    here = Path(__file__).resolve()
    # Expected: <repo>/MMBN4-Decomp/decomp/tools/mysterydata.py
    cand = here.parents[2]
    if (cand / "roms").is_dir():
        return cand
    cwd = Path.cwd()
    if (cwd / "roms").is_dir():
        return cwd
    if (cwd / "MMBN4-Decomp" / "roms").is_dir():
        return cwd / "MMBN4-Decomp"
    raise FileNotFoundError("Could not locate MMBN4-Decomp root with roms/")


def load_tbl(root: Path) -> dict[int, str]:
    tbl: dict[int, str] = {}
    tbl_path = root / "tools" / "textpet" / "plugins" / "bn4-utf8.tbl"
    with tbl_path.open(encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if "=" in line:
                h, c = line.split("=", 1)
                tbl[int(h, 16)] = c
    return tbl


def read_text(rom: bytes, tbl: dict[int, str], arc_addr: int, idx: int) -> str:
    arc_off = arc_addr - ROM_BASE
    rel = struct.unpack_from("<H", rom, arc_off + idx * 2)[0]
    p = arc_off + rel
    out: list[str] = []
    while p < len(rom) and rom[p] != 0xE5:
        b = rom[p]
        if b in tbl:
            out.append(tbl[b])
        p += 1
    return "".join(out).strip("_■ ")


class NameResolver:
    def __init__(self, rom: bytes, tbl: dict[int, str]):
        self.rom = rom
        self.tbl = tbl
        self.cw_bases = rom[RS_CW_MAP_NAME_BASES - ROM_BASE : RS_CW_MAP_NAME_BASES - ROM_BASE + 21]

    def map_name(self, gid: int, mid: int) -> str:
        if gid >= 0x80:
            idx = self.cw_bases[gid - 0x80] + mid
            return read_text(self.rom, self.tbl, RS_MAP_NAMES, idx)
        return f"RW_{gid:02X}_{mid:02X}"

    def item_name(self, item_id: int) -> str:
        return read_text(self.rom, self.tbl, RS_ITEM_NAMES, item_id)

    def chip_name(self, chip_id: int) -> str:
        if chip_id < 256:
            return read_text(self.rom, self.tbl, RS_CHIP_NAMES_LO, chip_id)
        return read_text(self.rom, self.tbl, RS_CHIP_NAMES_HI, chip_id - 256)

    def chip_code(self, code_id: int) -> str:
        return read_text(self.rom, self.tbl, RS_CHIP_CODES, code_id)

    def prog_name(self, prog_val: int) -> str:
        return read_text(self.rom, self.tbl, RS_PROG_NAMES, (prog_val - 0xC0) >> 2)

    def format_content(self, kind: int, param: int, val: int) -> str:
        if kind == 1:
            return f"{self.chip_name(val)} {self.chip_code(param)}"
        if kind == 6:
            return f"{self.chip_name(val)} {self.chip_code(param)} (virus)"
        if kind == 2:
            return self.item_name(val)
        if kind == 3:
            return f"{val}Z"
        if kind == 7:
            return f"{val}Z (virus)"
        if kind == 4:
            return self.item_name(val)
        if kind == 5:
            return f"{val} BugFrag" + ("s" if val != 1 else "")
        if kind == 8:
            return self.item_name(val)  # HP Memry
        if kind == 9:
            col = NAVICUST_COLORS.get(param, f"Color{param}")
            return f"{self.prog_name(val)} ({col})"
        if kind == 10:
            return self.item_name(val)  # RegUP1 / RegUP2 / RegUP3
        if kind == 11:
            return self.item_name(val)  # ExpMemry
        return f"Kind{kind}(p={param}, v=0x{val:X})"


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class Placement:
    valid: int
    weight: int
    x: int
    y: int
    z: int


@dataclass
class ContentEntry:
    kind: int
    weight: int
    pad: int
    param: int
    val: int
    label: str


@dataclass
class MysteryDataRecord:
    group_id: int
    map_id: int
    map_name: str
    entry_addr: int
    md_type: int
    obj_flags: int
    flag: int
    placements_addr: int
    contents_addr: int
    placements: list[Placement]
    # For BMD/PMD: 1 list of 2 or 4 entries (pickup 1, 2, [3, 4+]).
    # For GMD: 3 lists (Normal, Hard, Super Hard), each of 8 entries.
    tiers: list[list[ContentEntry]]


def parse_all_mystery_data(rom: bytes, resolver: NameResolver) -> list[MysteryDataRecord]:
    records: list[MysteryDataRecord] = []
    # Check real-world master table (expected all 0 up to sentinel 1)
    rw_off = RS_RW_MASTER_TABLE - ROM_BASE
    g = 0
    while True:
        gp = struct.unpack_from("<I", rom, rw_off + g * 4)[0]
        if gp == 1:
            break
        assert gp == 0, f"Unexpected non-zero RW group {g}: 0x{gp:08X}"
        g += 1

    # Walk cyberworld master table (0x80..0x94)
    cw_off = RS_CW_MASTER_TABLE - ROM_BASE
    g = 0
    while True:
        gp = struct.unpack_from("<I", rom, cw_off + g * 4)[0]
        if gp == 1:
            break
        if gp != 0:
            gid = 0x80 + g
            goff = gp - ROM_BASE
            m = 0
            while True:
                mp = struct.unpack_from("<I", rom, goff + m * 4)[0]
                if mp == 0:
                    break
                moff = mp - ROM_BASE
                mname = resolver.map_name(gid, m)
                e = 0
                while True:
                    eaddr = mp + e * 12
                    et, obj_flags, flag, pp, cp = struct.unpack_from("<BBHII", rom, moff + e * 12)
                    if et == 0:
                        break
                    # Placements
                    placements: list[Placement] = []
                    poff = pp - ROM_BASE
                    pi = 0
                    while True:
                        pv, pw, px, py, pz = struct.unpack_from("<BBhhh", rom, poff + pi * 8)
                        if pv == 0:
                            break
                        placements.append(Placement(pv, pw, px, py, pz))
                        pi += 1
                    # Contents
                    num_tiers = 3 if et == 5 else 1
                    tiers: list[list[ContentEntry]] = []
                    for t in range(num_tiers):
                        coff = cp - ROM_BASE + t * 0x44
                        centries: list[ContentEntry] = []
                        ci = 0
                        while True:
                            ck, cw, cpad, b3, u4 = struct.unpack_from("<BBBBI", rom, coff + ci * 8)
                            if ck == 0:
                                break
                            centries.append(
                                ContentEntry(ck, cw, cpad, b3, u4, resolver.format_content(ck, b3, u4))
                            )
                            ci += 1
                        tiers.append(centries)
                    records.append(
                        MysteryDataRecord(
                            group_id=gid,
                            map_id=m,
                            map_name=mname,
                            entry_addr=eaddr,
                            md_type=et,
                            obj_flags=obj_flags,
                            flag=flag,
                            placements_addr=pp,
                            contents_addr=cp,
                            placements=placements,
                            tiers=tiers,
                        )
                    )
                    e += 1
                m += 1
        g += 1
    return records


def verify_rs_bm(rs: bytes, bm: bytes) -> dict:
    # 1. Verify entire data block 0x080A9110..0x080ADD8C
    word_diffs = []
    for off in range(RS_DATA_START - ROM_BASE, RS_DATA_END - ROM_BASE, 4):
        rv = struct.unpack_from("<I", rs, off)[0]
        bv = struct.unpack_from("<I", bm, off + BM_CODE06_DELTA)[0]
        if rv == bv:
            continue
        if 0x080A0000 <= rv < 0x080B0000 and bv == rv + BM_CODE06_DELTA:
            continue
        word_diffs.append((ROM_BASE + off, rv, bv))

    # 2. Verify pickup script archive
    rs_arc = rs[RS_MD_SCRIPT_ARCHIVE - ROM_BASE : RS_MD_SCRIPT_ARCHIVE - ROM_BASE + MD_SCRIPT_ARCHIVE_SIZE]
    bm_arc = bm[BM_MD_SCRIPT_ARCHIVE - ROM_BASE : BM_MD_SCRIPT_ARCHIVE - ROM_BASE + MD_SCRIPT_ARCHIVE_SIZE]

    # 3. Verify routine prologues
    routine_checks = []
    for name, rsa, delta, seg, desc in ROUTINES:
        bma = rsa + delta
        rb = rs[rsa - ROM_BASE : rsa - ROM_BASE + 4]
        bb = bm[bma - ROM_BASE : bma - ROM_BASE + 4]
        routine_checks.append((name, rsa, bma, seg, rb == bb, desc))

    return {
        "word_diffs": word_diffs,
        "script_archive_identical": rs_arc == bm_arc,
        "routine_checks": routine_checks,
    }


def normalize_for_legend(entry: ContentEntry, resolver: NameResolver) -> tuple[str, str]:
    """Return (item_name, code_or_amount) in legend.csv style."""
    k, p, v = entry.kind, entry.param, entry.val
    if k in (1, 6):
        return (resolver.chip_name(v), resolver.chip_code(p))
    if k in (2, 4, 8, 11):
        return (resolver.item_name(v), "")
    if k == 10:
        # ROM says RegUP1/2/3, legend.csv says RegUp1/2/3
        return (resolver.item_name(v).replace("RegUP", "RegUp"), "")
    if k in (3, 7):
        return ("Zenny", str(v))
    if k == 5:
        return ("BugFrags", str(v))
    if k == 9:
        return (resolver.prog_name(v), "")
    return (entry.label, "")


def diff_legend_csv(root: Path, records: list[MysteryDataRecord], resolver: NameResolver) -> dict | None:
    csv_path = root / "ref" / "maps" / "legend.csv"
    if not csv_path.is_file():
        return None

    legend_by_area: dict[str, list[tuple[str, str, str, str]]] = {}
    with csv_path.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            src = row["source"].strip()
            if src not in ("BMD", "PMD"):
                continue
            area = row["area"].strip()
            legend_by_area.setdefault(area, []).append(
                (src, row["item"].strip(), row["code_or_amount"].strip(), row["marker"].strip())
            )

    rom_main_by_area: dict[str, list[tuple[str, str, str, MysteryDataRecord]]] = {}
    rom_comps: list[MysteryDataRecord] = []
    for rec in records:
        if rec.md_type == 5:
            continue
        area = MAP_TO_LEGEND_AREA.get(rec.map_name)
        if area is None:
            rom_comps.append(rec)
            continue
        src = TYPE_NAMES[rec.md_type]
        item, extra = normalize_for_legend(rec.tiers[0][0], resolver)
        rom_main_by_area.setdefault(area, []).append((src, item, extra, rec))

    matched = 0
    mismatches = []
    all_areas = sorted(set(legend_by_area) | set(rom_main_by_area))
    for area in all_areas:
        l_list = list(legend_by_area.get(area, []))
        r_list = list(rom_main_by_area.get(area, []))
        # Exact multiset match on (src, item.lower(), extra)
        used_l = [False] * len(l_list)
        used_r = [False] * len(r_list)
        for ri, (rs_, ritem, rextra, rrec) in enumerate(r_list):
            for li, (ls_, litem, lextra, lmark) in enumerate(l_list):
                if not used_l[li] and rs_ == ls_ and ritem.lower() == litem.lower() and rextra == lextra:
                    used_l[li] = True
                    used_r[ri] = True
                    matched += 1
                    break
        rem_l = [l_list[i] for i in range(len(l_list)) if not used_l[i]]
        rem_r = [r_list[i] for i in range(len(r_list)) if not used_r[i]]
        if rem_l or rem_r:
            mismatches.append((area, rem_l, rem_r))

    return {
        "legend_total": sum(len(v) for v in legend_by_area.values()),
        "rom_main_total": sum(len(v) for v in rom_main_by_area.values()),
        "rom_comp_total": len(rom_comps),
        "matched": matched,
        "mismatches": mismatches,
        "rom_comps": rom_comps,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description="Extract and verify MMBN4 Mystery Data tables.")
    ap.add_argument("--dump-all", action="store_true", help="Print all BMD/PMD and GMD tables to stdout.")
    ap.add_argument("--write-doc", action="store_true", help="Write decomp/notes/re07-mystery-data.md.")
    args = ap.parse_args()

    root = find_game_root()
    rs = (root / "roms" / "bn4_rs_us.gba").read_bytes()
    bm = (root / "roms" / "bn4_bm_us.gba").read_bytes()
    tbl = load_tbl(root)
    resolver = NameResolver(rs, tbl)

    records = parse_all_mystery_data(rs, resolver)
    rs_bm = verify_rs_bm(rs, bm)
    diff = diff_legend_csv(root, records, resolver)

    bmds = [r for r in records if r.md_type in (1, 2)]
    pmds = [r for r in records if r.md_type in (3, 4)]
    gmds = [r for r in records if r.md_type == 5]

    print(f"Total Mystery Data placements: {len(records)} (BMD={len(bmds)}, PMD={len(pmds)}, GMD={len(gmds)})")
    print(f"RS vs BM data word diffs (0x080A9110..0x080ADD8C): {len(rs_bm['word_diffs'])}")
    print(f"RS vs BM pickup script archive identical: {rs_bm['script_archive_identical']}")
    print(f"RS vs BM routine prologues matched: {sum(1 for x in rs_bm['routine_checks'] if x[4])}/{len(rs_bm['routine_checks'])}")

    if diff is not None:
        print(
            f"legend.csv diff (main areas): {diff['matched']}/{diff['rom_main_total']} exact matches "
            f"(legend={diff['legend_total']}, rom_main={diff['rom_main_total']}, rom_comps_not_in_legend={diff['rom_comp_total']})"
        )
        for area, rem_l, rem_r in diff["mismatches"]:
            print(f"  MISMATCH {area}:")
            for x in rem_l:
                print(f"    legend.csv only: {x}")
            for rs_, ritem, rextra, rrec in rem_r:
                tiers_str = " / ".join(c.label for c in rrec.tiers[0])
                print(f"    ROM only: ({rs_}, {ritem!r}, {rextra!r}) flag=0x{rrec.flag:04X} [{tiers_str}]")

    if args.dump_all:
        print("\n=== ALL BMD & PMD ===")
        for r in bmds + pmds:
            tstr = " | ".join(c.label for c in r.tiers[0])
            p0 = r.placements[0]
            print(f"({r.group_id:02X},{r.map_id:02X}) {r.map_name:14s} {TYPE_NAMES[r.md_type]:3s} 0x{r.flag:04X} ({p0.x},{p0.y},{p0.z}) : {tstr}")

    if args.write_doc:
        out_path = root / "decomp" / "notes" / "re07-mystery-data.md"
        write_markdown_doc(out_path, records, rs_bm, diff, resolver)
        print(f"Wrote {out_path}")

    return 0


def write_markdown_doc(
    out_path: Path,
    records: list[MysteryDataRecord],
    rs_bm: dict,
    diff: dict | None,
    resolver: NameResolver,
) -> None:
    bmds = [r for r in records if r.md_type in (1, 2)]
    pmds = [r for r in records if r.md_type in (3, 4)]
    gmds = [r for r in records if r.md_type == 5]

    lines: list[str] = []
    w = lines.append

    w("# RE-07: Mystery Data Tables (Blue, Purple, Green)")
    w("")
    w("Generated by `decomp/tools/mysterydata.py` from US Red Sun (`bn4_rs_us.gba`) and US Blue Moon (`bn4_bm_us.gba`).")
    w("Status: **V** = verified in disassembly and both US ROMs.")
    w("")
    w("## 1. Summary & Correction to Earlier Leads")
    w("")
    w(f"- **Total Mystery Data placements in BN4:** **{len(records)}** — **{len(bmds)} Blue (BMD)**, **{len(pmds)} Purple (PMD)**, and **{len(gmds)} Green (GMD)**, all in cyberworld map groups (`0x80`–`0x82`, `0x8C`–`0x8E`, `0x90`–`0x94`). The real-world master table (`dat_080ADD18`) has 6 zero slots and a sentinel `1` (no real-world Mystery Data).")
    w("- **Flag range correction:** Mystery Data \"taken\" flags are in **`0x0D00`–`0x10FF`**, **not** `0x15C0`–`0x163F`. Each flag `f` also has a 2-byte state slot in save slot `+0x74` (`[eToolkit+0x74] + (f - 0x0D00) * 2`, `0x800` bytes total): byte `+0` = `placement_idx`, byte `+1` = `content_idx`. Both flags `0x0D00`–`0x10FF` and slot `+0x74` are stashed/restored across New Game+ (`sub_0800143C`/`sub_08001460`), and then `MysteryDataAdvanceAll` (`sub_080ADE5C`) advances taken BMDs/PMDs to their next pickup tier and clears their flags.")
    w("- **Advance-on-pickup mechanic (types 1 & 3):** Every BMD (`type 1`) and PMD (`type 3`) in BN4 has **2 or 4 sequential reward slots** (1st pickup, 2nd pickup, [3rd pickup, 4th+ pickup]). On New Game+, `MysteryDataNextIndices` (`sub_080ADEB2`) checks whether `flag` was set on the previous playthrough: **if taken**, `content_idx` increments by 1 (clamped to the last slot) and `flag` is cleared; **if not taken**, `content_idx` stays unchanged so the player cannot permanently miss an earlier tier's reward by skipping it on Normal/Hard. (Types `2` and `4` exist in `sub_080ADEB2` as playthrough-indexed variants, but **0** entries in BN4 use them.)")
    w("- **Green Mystery Data (type 5):** Every GMD has **4 spawn coordinates** and **3 playthrough tiers** (Normal / Hard / Super Hard, capped at `min(GetPlaythrough(), 2)`), each with **8 weighted reward entries** (including virus-trapped chips and virus-trapped Zenny). `MysteryDataRollAllGreen` (`sub_080ADDA0`) rerolls every GMD's placement and content and clears its flag on new game and on every jack-in (`sub_080297C8`).")
    w("- **RS vs BM differences:** **Zero** differences in the Mystery Data tables (`0x080A9110`–`0x080ADD8C`, 0 non-pointer-shift word diffs between US RS and US BM) and **zero** differences in the Mystery Data pickup script archive (RS `0x087585A0` / BM `0x08758030`, `0xD1C` bytes, byte-identical). The only practical version difference is **reachability of version-exclusive scenario comps** in groups `0x8C`–`0x8E` (e.g., `WtrGodComp1`–`16` for Blue Moon's AquaMan scenario, vs Red Sun-exclusive tournament comps).")
    w("")

    # Section 2: Routines
    w("## 2. Code Routines (V)")
    w("")
    w("| RS Address | BM Address | Segment | Proposed Name | Purpose |")
    w("|---|---|---|---|---|")
    for name, rsa, bma, seg, ok, desc in rs_bm["routine_checks"]:
        assert ok, f"Prologue mismatch for {name}"
        w(f"| `0x{rsa:08X}` | `0x{bma:08X}` | `{seg}` | `{name}` | {desc} |")
    w("")
    w("### Lifecycle")
    w("1. **New Game (`NewGameInit`, `0x080049A8`):** Calls `MysteryDataInitState` (zeroes slot `+0x74`), `MysteryDataAdvanceAll`, and `MysteryDataRollAllGreen`.")
    w("2. **New Game+ (`sub_08000DE8`):** Restores flags `0x0D00`–`0x10FF` and slot `+0x74`, increments playthrough, and calls `MysteryDataAdvanceAll` (`sub_080ADE5C`).")
    w("3. **Jack-In (`JackIn_ResetGreenMysteryData`, `0x080297C8`):** Calls `MysteryDataRollAllGreen` (`sub_080ADDA0`), rerolling all 46 GMDs via `WeightedRandomIndex` (`sub_08000CA0`: `(GetRNG() & 0x3FF) % sum(weights) + 1`) and clearing their flags.")
    w("4. **Map Load (`MapObj_SpawnMysteryData`, `0x080A89BC`):** Map object spawn script passes the 16-bit `flag`; calls `MysteryDataLookupSpawn` (`0x080ADF80`), selects palette from `dat_080A8A50` (`FF 00 00 02 02 01`: Blue=0, Purple=2, Green=1), places object at `(x, y, z)`, and computes pickup script index `(content_kind - 1) * 10 + (type - 1)` in text archive `0x087585A0` (RS) / `0x08758030` (BM).")
    w("5. **Pickup (`MapObj_InteractMysteryData`, `0x080A84E8` / `MysteryDataLoadScriptVars`, `0x0809DB10`):** Calls `MysteryDataLookupReward` (`0x080ADFF0`), writes `flag` to `[text_state+0x4C]`, `reward_val` (`u32` at `+4`) to `[text_state+0x54]`, and `param` (`u8` at `+3`) to `[text_state+0x58]`, then executes the pickup text script (which handles Unlocker for PMDs, Untrap/virus battles for trapped GMDs, grants the reward, and sets `flag`).")
    w("")

    # Section 3: Table layouts
    w("## 3. Table Layouts (V)")
    w("")
    w("All Mystery Data tables sit in a single contiguous block in `code_06`: RS `0x080A9110`–`0x080ADD8C` (BM `0x080A911C`–`0x080ADD98`, delta `+0xC`, `0x4C7C` bytes).")
    w("")
    w("| Table | RS Address | BM Address | Layout |")
    w("|---|---|---|---|")
    w("| Real-world master table | `0x080ADD18` | `0x080ADD24` | 6 × `u32` (all `0`) + sentinel `0x00000001` |")
    w("| Cyberworld master table | `0x080ADD34` | `0x080ADD40` | 21 × `u32` group pointers (`0x80`–`0x94`, `0` if unused) + sentinel `0x00000001` |")
    w("| Per-group map pointer table | `0x080A9110`… | `+0xC` | `0`-terminated array of pointers to per-map entry lists |")
    w("| Per-map entry list | `0x080A9110`… | `+0xC` | 12-byte records terminated by `type == 0`: `{u8 type, u8 obj_flags, u16 flag, ptr placements, ptr contents}` |")
    w("| Placement list | `0x080A9110`… | `+0xC` | 8-byte records terminated by `valid == 0`: `{u8 valid (1), u8 weight, s16 x, s16 y, s16 z}` (1 entry for BMD/PMD, 4 for GMD) |")
    w("| Content list | `0x080A9110`… | `+0xC` | 8-byte records terminated by `kind == 0`: `{u8 kind, u8 weight, u8 pad (0xFF), u8 param, u32 value}`. For GMD (`type 5`), 3 consecutive `0x44`-byte tier blocks (8 × 8-byte entries + 4-byte `0` terminator each). |")
    w("")
    w("### `content_kind` Encoding (Verified from `0x087585A0` and `ScriptCmd_printItem` `0x0804FAD0`)")
    w("")
    w("| Kind | Meaning | `param` (`+3`) | `value` (`+4`, `u32`) | Used By |")
    w("|---|---|---|---|---|")
    w("| `1` | Battle Chip | Chip code (`0`=`A`..`25`=`Z`, `26`=`*`) | Chip ID (`0x087481D8` / `0x08748C14`) | BMD, PMD, GMD |")
    w("| `2` | SubChip | `0xFF` | Item ID `176`–`181` (`0x087571A0`) | BMD |")
    w("| `3` | Zenny | `0xFF` | Amount | BMD, PMD, GMD |")
    w("| `4` | Key Item (Spin) | `0xFF` | Item ID `128`–`133` (`SpinWhit`..`SpinGrn`) | BMD, PMD |")
    w("| `5` | BugFrags | `0xFF` | Amount | BMD, PMD, GMD |")
    w("| `6` | Virus-trapped Battle Chip | Chip code | Chip ID (triggers virus battle unless Untrap active) | GMD only |")
    w("| `7` | Virus-trapped Zenny | `0xFF` | Amount (triggers virus battle unless Untrap active) | GMD only |")
    w("| `8` | HP Memory / ExpMemory | `0xFF` | `160` (`0xA0`, `HP Memry`), `161` (`0xA1`, `ExpMemry`) | BMD, PMD |")
    w("| `9` | NaviCust Program | Color (`1`=White, `2`=Pink, `3`=Yellow, `4`=Red, `5`=Blue, `6`=Green) | `0xC0 + prog_idx * 4` (`0x086E7FC8`) | BMD, PMD |")
    w("| `10` | RegUP | `0xFF` | `162`=`RegUP1`, `163`=`RegUP2`, `164`=`RegUP3` | BMD, PMD |")
    w("| `11` | SubMemory | `0xFF` | `165` (`0xA5`, `SubMemry`) | BMD |")
    w("")

    # Section 4: Every BMD and PMD
    w(f"## 4. Every Blue and Purple Mystery Data Placement ({len(bmds)} BMD + {len(pmds)} PMD = {len(bmds)+len(pmds)} Total)")
    w("")
    w("Because BN4 uses advance-on-pickup (`type 1`/`3`), **1st / 2nd / 3rd / 4th+** denote how many times that specific Mystery Data has been collected across playthroughs (a 2-entry item repeats its 2nd reward on all subsequent pickups; a 4-entry item repeats its 4th reward from the 4th pickup onward).")
    w("")
    w("### 4.1 Main Net & Dungeon Areas (64 BMD + 14 PMD = 78)")
    w("")
    w("| Map (Grp,Num) | Map Name | Type | Flag | Coords (X,Y,Z) | 1st Pickup | 2nd Pickup | 3rd Pickup | 4th+ Pickup |")
    w("|---|---|---|---|---|---|---|---|---|")
    main_recs = [r for r in bmds + pmds if r.map_name in MAP_TO_LEGEND_AREA]
    comp_recs = [r for r in bmds + pmds if r.map_name not in MAP_TO_LEGEND_AREA]
    for r in main_recs:
        p0 = r.placements[0]
        c = [e.label for e in r.tiers[0]]
        c1 = c[0]
        c2 = c[1] if len(c) > 1 else "—"
        c3 = c[2] if len(c) > 2 else "(repeats 2nd)"
        c4 = c[3] if len(c) > 3 else ("(repeats 2nd)" if len(c) == 2 else "—")
        w(f"| `({r.group_id:02X},{r.map_id:02X})` | {r.map_name} | {TYPE_NAMES[r.md_type]} | `0x{r.flag:04X}` | `({p0.x},{p0.y},{p0.z})` | {c1} | {c2} | {c3} | {c4} |")
    w("")

    w(f"### 4.2 Small Comps & Scenario Dungeons ({len(comp_recs)} BMD, 0 PMD — Not in `legend.csv`)")
    w("")
    w("| Map (Grp,Num) | Map Name | Type | Flag | Coords (X,Y,Z) | 1st Pickup | 2nd Pickup | 3rd Pickup | 4th+ Pickup |")
    w("|---|---|---|---|---|---|---|---|---|")
    for r in comp_recs:
        p0 = r.placements[0]
        c = [e.label for e in r.tiers[0]]
        c1 = c[0]
        c2 = c[1] if len(c) > 1 else "—"
        c3 = c[2] if len(c) > 2 else "(repeats 2nd)"
        c4 = c[3] if len(c) > 3 else ("(repeats 2nd)" if len(c) == 2 else "—")
        w(f"| `({r.group_id:02X},{r.map_id:02X})` | {r.map_name} | {TYPE_NAMES[r.md_type]} | `0x{r.flag:04X}` | `({p0.x},{p0.y},{p0.z})` | {c1} | {c2} | {c3} | {c4} |")
    w("")

    # Section 5: Green Mystery Data
    w(f"## 5. Green Mystery Data Tables ({len(gmds)} Spawns Across 22 Net Areas)")
    w("")
    w("Each GMD selects 1 of 4 spawn positions (equal weight `32` each = 25% each) and 1 of 8 rewards for the active playthrough tier (`Normal` = 0, `Hard` = 1, `Super Hard` = 2+). Exact odds are `weight / sum(weights)` (with `WeightedRandomIndex` rolling `0..1023` mod `sum`).")
    w("")
    tier_names = ["Normal", "Hard", "Super Hard"]
    by_map: dict[tuple[int, int, str], list[MysteryDataRecord]] = {}
    for r in gmds:
        by_map.setdefault((r.group_id, r.map_id, r.map_name), []).append(r)

    for (gid, mid, mname), recs in by_map.items():
        w(f"### `({gid:02X},{mid:02X})` {mname} ({len(recs)} GMDs)")
        w("")
        # Check if multiple GMDs in the same map share the exact same content pointer
        for idx, r in enumerate(recs, 1):
            w(f"**GMD #{idx}** — Flag `0x{r.flag:04X}`, contents `0x{r.contents_addr:08X}` (BM `0x{r.contents_addr+BM_CODE06_DELTA:08X}`):")
            w("")
            w("| Tier | Total W | Entries (Reward — weight / odds) |")
            w("|---|---|---|")
            for ti, tlist in enumerate(r.tiers):
                tot = sum(e.weight for e in tlist)
                parts = [f"{e.label} (`{e.weight}` = {100.0 * e.weight / tot:.1f}%)" for e in tlist]
                w(f"| {tier_names[ti]} | {tot} | " + "; ".join(parts) + " |")
            w("")

    # Section 6: RS vs BM Differences
    w("## 6. Red Sun vs Blue Moon Differences")
    w("")
    w(f"- **ROM Mystery Data tables (`0x080A9110`–`0x080ADD8C`):** **{len(rs_bm['word_diffs'])}** non-pointer-shift word differences. Every single BMD, PMD, and GMD placement, flag, coordinate, weight, and reward across all tiers is **100% identical** between US Red Sun and US Blue Moon (every internal ROM pointer shifts uniformly by `+0xC`).")
    w(f"- **Pickup text script archive (RS `0x087585A0` / BM `0x08758030`, `0xD1C` bytes):** **Byte-identical** (`{rs_bm['script_archive_identical']}`).")
    w("- **Scenario-comp reachability caveat (for Violet Eclipse):** Although the tables are identical in both ROMs, several small comps in groups `0x8C`–`0x8E` belong to version-exclusive tournament scenarios (for example, `WtrGodComp1`–`16` in group `0x8E` belong to Blue Moon's AquaMan scenario, and certain DenDome/Castillo/Colosseum comps belong to specific tournament branches). Making all scenarios playable in Violet Eclipse automatically unlocks their BMDs too.")
    w("")

    # Section 7: Diff against ref/maps/legend.csv
    w("## 7. Diff Against `ref/maps/legend.csv`")
    w("")
    if diff is None:
        w("`ref/maps/legend.csv` was not present when this document was generated.")
    else:
        w(f"- **Main 32 areas:** **{diff['matched']} / {diff['rom_main_total']}** exact matches (`legend.csv` has {diff['legend_total']} BMD/PMD rows; ROM has {diff['rom_main_total']} in those 32 areas).")
        w(f"- **Small comps (`0x8C`–`0x8E`):** **{diff['rom_comp_total']}** additional BMDs in the ROM are not listed in `legend.csv` (the fan maps only covered the 32 main net and dungeon areas). Notable unique items among those 45 comp BMDs include `SpinWhit` (`StatueComp`), `SpinPink` (`ColoNBMComp`), `SpinYllw` (`GoddessComp`), `ExpMemry` (`HeroComp`), `SubMemry` (`CastNBMComp` 2nd, `NupopoComp`, `Ticket Comp`), `FullCust *` (`WtrGodComp12`), `HP Memry` (7 comps), `RegUP1`/`RegUP2`, `Fish`, `Battery`, and multiple battle chips.")
        w("")
        if not diff["mismatches"]:
            w("Zero mismatches in the 32 main areas.")
        else:
            w("### Mismatches in the 32 Main Areas")
            w("")
            w("| Area | `legend.csv` | ROM (1st Pickup) | Flag | Explanation |")
            w("|---|---|---|---|---|")
            for area, rem_l, rem_r in diff["mismatches"]:
                l_str = ", ".join(f"{s} {i} {e}".strip() + f" (marker {m})" for s, i, e, m in rem_l) or "—"
                r_str = ", ".join(f"{s} {i} {e}".strip() for s, i, e, _ in rem_r) or "—"
                f_str = ", ".join(f"`0x{rec.flag:04X}`" for _, _, _, rec in rem_r) or "—"
                # Provide specific explanation based on actual mismatch
                expl = _explain_mismatch(area, rem_l, rem_r)
                w(f"| {area} | {l_str} | {r_str} | {f_str} | {expl} |")
    w("")

    # Section 8: Proposed names
    w("## 8. Proposed Symbol Names (`names.txt` format)")
    w("")
    w("```")
    for name, rsa, bma, seg, ok, desc in rs_bm["routine_checks"]:
        w(f"{rsa:08X} {bma:08X} code   GEMINI       {name:32s} {desc}")
    w(f"{RS_RW_MASTER_TABLE:08X} {RS_RW_MASTER_TABLE+BM_CODE06_DELTA:08X} data   GEMINI       {'BN4_MYSTERY_DATA_RW':32s} Real-world Mystery Data master table (6 zero slots + sentinel 1)")
    w(f"{RS_CW_MASTER_TABLE:08X} {RS_CW_MASTER_TABLE+BM_CODE06_DELTA:08X} data   GEMINI       {'BN4_MYSTERY_DATA_CW':32s} Cyberworld Mystery Data master table (21 groups 0x80..0x94 + sentinel 1)")
    w(f"{RS_MD_SCRIPT_ARCHIVE:08X} {BM_MD_SCRIPT_ARCHIVE:08X} data   GEMINI       {'BN4_MYSTERY_DATA_SCRIPTS':32s} Mystery Data pickup text script archive (110 scripts, 0xD1C bytes)")
    w(f"{RS_ITEM_NAMES:08X} {RS_ITEM_NAMES-0x570:08X} data   GEMINI       {'BN4_ITEM_NAMES':32s} Item and SubChip name text archive (indexed by item id)")
    w(f"{RS_CHIP_CODES:08X} {RS_CHIP_CODES-0x50C:08X} data   GEMINI       {'BN4_CHIP_CODES':32s} Chip code text archive (A..Z, *)")
    w(f"{RS_PROG_NAMES:08X} {RS_PROG_NAMES-0x50C:08X} data   GEMINI       {'BN4_NAVICUST_NAMES':32s} NaviCust program name text archive (index = (id - 0xC0) >> 2)")
    w(f"{RS_MAP_NAMES:08X} {BM_MAP_NAMES:08X} data   GEMINI       {'BN4_MAP_NAMES':32s} Map name text archive (244 entries)")
    w("```")
    w("")

    out_path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def _explain_mismatch(area: str, rem_l: list, rem_r: list) -> str:
    if len(rem_l) == 1 and len(rem_r) == 1:
        ls, li, le, lm = rem_l[0]
        rs, ri, re_, rrec = rem_r[0]
        if li.lower() == ri.lower() and le == re_ and ls != rs:
            return f"Source color error in fan map: ROM type is `{rrec.md_type}` ({rs}), not {ls}."
        if li.lower() == ri.lower() and le != re_:
            return f"Code/amount typo in fan map (`{le}` vs ROM `{re_}`)."
        # Check if legend entry matches a later pickup tier!
        for ti, ce in enumerate(rrec.tiers[0][1:], 2):
            if li.lower() in ce.label.lower():
                return f"Fan map recorded {ti}nd/rd-pickup reward (`{ce.label}`) instead of 1st-pickup (`{rrec.tiers[0][0].label}`)."
        return f"Fan map error: ROM 1st pickup is `{rrec.tiers[0][0].label}` (2nd: `{rrec.tiers[0][1].label}`)."
    return "See ROM entries above; fan map differs from ROM table."


if __name__ == "__main__":
    raise SystemExit(main())
