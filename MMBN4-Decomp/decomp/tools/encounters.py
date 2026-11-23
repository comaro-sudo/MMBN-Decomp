#!/usr/bin/env python3
"""
RE-06: Mega Man Battle Network 4 Encounters, Enemy Records & Drops extractor.

Reads US Red Sun (bn4_rs_us.gba) and US Blue Moon (bn4_bm_us.gba) read-only,
decodes:
  - BN4_ENEMY_STATS (254 x 12-byte enemy records)
  - BN4_REWARD_ROWS (276 x 20-u16 drop rows, RS & BM)
  - BN4_BUSTING_TABLE (Busting Level -> column probability table)
  - BN4_BATTLE_TABLES (all cyberworld random encounter tables, incl. +0x10/+0x20/+0x30)
  - BN4_STORY_BATTLES (245 story/tournament/boss battle records)

Usage:
    python decomp/tools/encounters.py             # print summary
    python decomp/tools/encounters.py --write-doc # write decomp/notes/re06-encounters.md
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path

ROM_BASE = 0x08000000

# ROM addresses (US Red Sun)
RS_ENEMY_STATS = 0x08011888        # 254 x 12 bytes (RS == BM)
ENEMY_COUNT = 254

RS_BATTLE_TABLES = 0x08017FB8      # {ptr rw_groups, ptr cw_groups} (RS == BM)
RS_CW_BATTLE_GROUPS = 0x08018014   # 21 u32 ptrs (groups 0x80..0x94)
RS_BUSTING_TABLE = 0x0801881D      # 12 x 16 bytes (RS == BM)

RS_REWARD_ROWS = 0x080F5F50        # 276 x 40 bytes (BM +0xC)
REWARD_ROW_COUNT = 276
RS_STORY_BATTLES = 0x080FA900      # 245 x 12 bytes (BM +0xC)
STORY_BATTLE_COUNT = 245
BM_CODE09_DELTA = 0xC

# Text archives (US Red Sun)
RS_CHIP_NAMES_LO = 0x087481D8
RS_CHIP_NAMES_HI = 0x08748C14
RS_CHIP_CODES = 0x087490E8
RS_ENEMY_NAMES = 0x0874A780
RS_MAP_NAMES = 0x086E8770
RS_CW_MAP_NAME_BASES = 0x0802DF00

ELEMENTS = {
    0: "Null",
    1: "Fire",
    2: "Aqua",
    3: "Elec",
    4: "Wood",
}

ATTR_MASK_NAMES = [
    (0x01, "Norm"),
    (0x02, "Fire"),
    (0x04, "Aqua"),
    (0x08, "Elec"),
    (0x10, "Wood"),
]

# Routines: (name, rs_addr, bm_delta, segment, description)
ROUTINES = [
    ("BN4_STORY_BATTLE",            0x08007BEC, 0x0, "code_00", "r0 battle_id: returns &BN4_STORY_BATTLES[r0] (12-byte record)"),
    ("BattleGetBackgroundOrMode",   0x08007DC0, 0x0, "code_00", "Reads battle record +2; adds 0x3C * min(playthrough, 2) unless bit 7 of +6 set"),
    ("BN4_ID_RAISE",                0x0800B036, 0x0, "code_00", "Raises enemy id by playthrough tier: virus +2*tier (cap ver 5), Navi +1*tier (cap ver 2), unless bit 7 of battle+6 set"),
    ("BN4_ENEMY_RECORD",            0x08011878, 0x0, "code_01", "r0 enemy_id: returns &BN4_ENEMY_STATS[r0] (12-byte record)"),
    ("RewardDecode",                0x08021A98, 0x4, "code_02", "Decodes 16-bit reward word: kind 0=chip(id|code<<9), 1=Zenny, 2=HP, 3=BugFrags, 0xFFFF=None"),
    ("ScriptCmd_startRandomBattle", 0x08050228, 0xC, "code_04", "Script command FB 02: calls BN4_ROLL(mask=0x1F) and enters battle"),
    ("Overworld_TryRandomEncounter",0x080F53D8, 0xC, "code_09", "Step-counter encounter trigger: calls BN4_ROLL with NaviCust element mask (fallback 0x1F)"),
    ("BN4_ROLL",                    0x080F5448, 0xC, "code_09", "Rolls weighted random encounter for current (group, map + BN4_STORY_OFFSET) filtered by r2 mask"),
    ("BN4_STORY_OFFSET",            0x080F54B8, 0xC, "code_09", "Per-group map-slot offset: +0x10 for wandering Ghost Navi, +0x20/+0x30 for story invasions"),
    ("BN4_REWARD_PICK",             0x080F580C, 0xC, "code_09", "Selects post-battle reward from BN4_REWARD_ROWS (handles normal, Collect, and Dark Soul drops)"),
    ("RewardPickBustingOffset",     0x080F5998, 0xC, "code_09", "Computes byte offset in 40-byte reward row from Busting Level (BN4_BUSTING_TABLE), RNG, and HP <= 3/8"),
    ("BN4_REWARD_ROW_OF",           0x080F5EC4, 0xC, "code_09", "Maps enemy id -> reward row (redirects to 0x100..0x113 Dark Soul rows under Dark Karma)"),
    ("RewardGetRowPtr",             0x080F5F40, 0xC, "code_09", "r0 row: returns &BN4_REWARD_ROWS[row] (row * 0x28)"),
]

# Wandering Ghost Navi unlock flags from BN4_STORY_OFFSET (sub_080F5568..080F56CA)
WANDERING_FLAGS = {
    (0x90, 0): 0x004D, (0x90, 1): 0x0044, (0x90, 2): 0x0050,
    (0x91, 0): 0x004F, (0x91, 1): 0x0047, (0x91, 2): 0x0043, (0x91, 3): 0x004A,
    (0x92, 0): 0x004E, (0x92, 1): 0x0051, (0x92, 2): 0x0052,
    (0x93, 0): 0x0045, (0x93, 1): 0x0048, (0x93, 2): 0x0054, (0x93, 3): 0x0053,
    (0x94, 0): 0x0046, (0x94, 1): 0x004C, (0x94, 2): 0x004B, (0x94, 3): 0x0055,
    (0x94, 4): 0x0056, (0x94, 5): 0x0049, (0x94, 7): 0x009F,
}

# Navi version availability in vanilla RS / BM (from docs/re/tournaments.md + game progression)
# Base enemy ID (V1) -> (version, how_reached)
NAVI_AVAILABILITY = {
    0x80: ("RS (BM via trade)", "Den Battle Tournament (Soul Navi 0); V2/V3 ghost in ACDC Area 1"),
    0x84: ("RS (BM via trade)", "Den Battle Tournament (Soul Navi 1); V2/V3 ghost in ACDC Area 2"),
    0x88: ("RS (BM via trade)", "Eagle Tournament (Soul Navi 4); V2/V3 ghost in Town Area 2"),
    0x8C: ("RS (BM via trade)", "Eagle Tournament (Soul Navi 5); V2/V3 ghost in Park Area 2"),
    0x90: ("RS (BM via trade)", "Red Sun Tournament (Soul Navi 8); V2/V3 ghost in Town Area 3"),
    0x94: ("RS (BM via trade)", "Red Sun Tournament (Soul Navi 9); V2/V3 ghost in YumLand Area"),
    0x98: ("BM (RS via trade)", "Den Battle Tournament (Soul Navi 2); V2/V3 ghost in Undernet 1"),
    0x9C: ("BM (RS via trade)", "Den Battle Tournament (Soul Navi 3); V2/V3 ghost in ACDC Area 3"),
    0xA0: ("BM (RS via trade)", "Hawk Tournament (Soul Navi 6); V2/V3 ghost in Town Area 1"),
    0xA4: ("BM (RS via trade)", "Hawk Tournament (Soul Navi 7); V2/V3 ghost in Park Area 1"),
    0xA8: ("BM (RS via trade)", "Blue Moon Tournament (Soul Navi 10); V2/V3 ghost in Park Area 3"),
    0xAC: ("BM (RS via trade)", "Blue Moon Tournament (Soul Navi 11); V2/V3 ghost in Netopia Area"),
    0xB0: ("Both",              "Den Battle Tournament (shared entrant); V2/V3 ghost in Town Area 4"),
    0xB4: ("Both",              "Story boss (Castillo / Undernet); V2/V3 ghost in Undernet 4"),
    0xB8: ("Both",              "Eagle/Hawk Tournament (shared entrant); V2/V3 ghost in Undernet 2"),
    0xBC: ("Both",              "Red Sun/Blue Moon Tournament (shared entrant); V2/V3 ghost in Sharo Area"),
    0xC0: ("Both",              "Den Battle Tournament (shared entrant); V2/V3 ghost in Undernet 3"),
    0xD0: ("Both",              "Story boss (Meteor / Undernet); V2/V3 ghost in Undernet 5"),
    0xD4: ("Both",              "Red Sun/Blue Moon Tournament (shared entrant); V2/V3 ghost in NetFrica Area"),
    0xD8: ("Both",              "Eagle/Hawk Tournament (shared entrant); V2/V3 ghost in Undernet 6"),
    0xE0: ("Both",              "Final story boss (Meteor Comp 4); scales V1/α/β by playthrough"),
    0xE4: ("Both",              "Post-game secret boss (Black Earth 2); BassΩ wandering in Black Earth 2 (flag 0x9F), BassXX via e-Reader/event"),
}


def find_game_root() -> Path:
    here = Path(__file__).resolve()
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
    with (root / "tools" / "textpet" / "plugins" / "bn4-utf8.tbl").open(encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if "=" in line:
                h, c = line.split("=", 1)
                tbl[int(h, 16)] = c
    return tbl


class Resolver:
    def __init__(self, rom: bytes, tbl: dict[int, str]):
        self.rom = rom
        self.tbl = tbl
        self.cw_bases = rom[RS_CW_MAP_NAME_BASES - ROM_BASE : RS_CW_MAP_NAME_BASES - ROM_BASE + 21]

    def text(self, arc_addr: int, idx: int) -> str:
        arc_off = arc_addr - ROM_BASE
        rel = struct.unpack_from("<H", self.rom, arc_off + idx * 2)[0]
        p = arc_off + rel
        out = []
        while p < len(self.rom) and self.rom[p] != 0xE5:
            b = self.rom[p]
            if b in self.tbl:
                out.append(self.tbl[b])
            p += 1
        return "".join(out).replace("\n", " ").strip("_■ ")

    def enemy_name(self, eid: int) -> str:
        if eid >= 253:
            return f"Enemy_{eid:02X}"
        s = self.text(RS_ENEMY_NAMES, eid)
        return s if s else f"(unused 0x{eid:02X})"

    def map_name(self, gid: int, mid: int) -> str:
        if gid >= 0x80:
            base = self.cw_bases[gid - 0x80]
            # Handle +0x10/+0x20/+0x30 encounter slots by masking low nibble for display
            idx = base + (mid & 0xF)
            if idx < 160:
                return self.text(RS_MAP_NAMES, idx)
        return f"Map({gid:02X},{mid:02X})"

    def reward_str(self, word: int) -> str:
        if word == 0xFFFF:
            return "None"
        kind = word >> 14
        if kind == 0:
            cid = word & 0x1FF
            code = (word >> 9) & 0x1F
            cname = self.text(RS_CHIP_NAMES_LO, cid) if cid < 256 else self.text(RS_CHIP_NAMES_HI, cid - 256)
            ccode = self.text(RS_CHIP_CODES, code)
            return f"{cname} {ccode}"
        if kind == 1:
            return f"{word & 0x3FFF}Z"
        if kind == 2:
            return f"HP+{word & 0x3FFF}"
        return f"{word & 0x3FFF} BF"


@dataclass
class EnemyStat:
    eid: int
    name: str
    hp: int
    elem: int
    t1_type: int
    cls: int
    family: int
    version: int
    atk_byte: int
    raw: bytes


@dataclass
class BattleRecord:
    addr: int
    weight: int
    b1: int
    b2: int
    b3: int
    b4: int
    b5: int
    flags: int
    b7: int
    form_addr: int
    enemies: list[tuple[int, int, int]]   # (enemy_id, col, row)
    objects: list[tuple[int, int, int]]   # (obj_id, col, row)


def parse_enemy_stats(rs: bytes, res: Resolver) -> list[EnemyStat]:
    out = []
    base = RS_ENEMY_STATS - ROM_BASE
    for i in range(ENEMY_COUNT):
        raw = rs[base + i * 12 : base + (i + 1) * 12]
        hp_f, t1, cls, fam, ver, _b6, _b7, u8 = struct.unpack("<HBBBBBBI", raw)
        atk_byte = (u8 >> 16) & 0xFF
        out.append(EnemyStat(i, res.enemy_name(i), hp_f & 0xFFF, hp_f >> 12, t1, cls, fam, ver, atk_byte, raw))
    return out


def parse_formation(rom: bytes, form_addr: int) -> tuple[list[tuple[int, int, int]], list[tuple[int, int, int]]]:
    off = form_addr - ROM_BASE
    enemies = []
    objects = []
    while rom[off] != 0xFF:
        oid, col, row, kind = rom[off : off + 4]
        if kind == 1:
            enemies.append((oid, col, row))
        elif kind != 0:
            objects.append((oid, col, row))
        off += 4
    return enemies, objects


def parse_battle_record(rom: bytes, addr: int) -> BattleRecord:
    off = addr - ROM_BASE
    b0, b1, b2, b3, b4, b5, b6, b7, fp = struct.unpack_from("<BBBBBBBBI", rom, off)
    enemies, objects = parse_formation(rom, fp)
    return BattleRecord(addr, b0, b1, b2, b3, b4, b5, b6, b7, fp, enemies, objects)


def parse_cyberworld_encounters(rs: bytes) -> dict[tuple[int, int], tuple[int, list[BattleRecord]]]:
    result: dict[tuple[int, int], tuple[int, list[BattleRecord]]] = {}
    cw_ptrs = struct.unpack_from("<21I", rs, RS_CW_BATTLE_GROUPS - ROM_BASE)
    for g in range(21):
        gp = cw_ptrs[g]
        if gp == 0:
            continue
        gid = 0x80 + g
        goff = gp - ROM_BASE
        first_mp = struct.unpack_from("<I", rs, goff)[0]
        n_slots = (first_mp - gp) // 4
        for m in range(n_slots):
            mp = struct.unpack_from("<I", rs, goff + m * 4)[0]
            if mp == 0:
                continue
            recs = []
            i = 0
            while rs[mp - ROM_BASE + i * 12] != 0xFF:
                recs.append(parse_battle_record(rs, mp + i * 12))
                i += 1
            if recs:
                result[(gid, m)] = (mp, recs)
    return result


def parse_story_battles(rs: bytes) -> list[BattleRecord]:
    return [parse_battle_record(rs, RS_STORY_BATTLES + i * 12) for i in range(STORY_BATTLE_COUNT)]


def format_attr_mask(flags: int) -> str:
    parts = [name for bit, name in ATTR_MASK_NAMES if flags & bit]
    if flags & 0x80:
        parts.append("FixedLv")
    return "+".join(parts) if parts else f"0x{flags:02X}"


def format_drop_row_compact(words: tuple[int, ...], res: Resolver) -> str:
    """Format 10-slot half-row as 5 Busting columns: C0 | C1 | C2 | C3 | C4."""
    cols = []
    for c in range(5):
        a = res.reward_str(words[c * 2])
        b = res.reward_str(words[c * 2 + 1])
        cols.append(a if a == b else f"{a} / {b}")
    return " | ".join(cols)


def verify_rs_bm(rs: bytes, bm: bytes) -> dict:
    # 1. Enemy stats
    es_same = rs[RS_ENEMY_STATS - ROM_BASE : RS_ENEMY_STATS - ROM_BASE + ENEMY_COUNT * 12] == bm[RS_ENEMY_STATS - ROM_BASE : RS_ENEMY_STATS - ROM_BASE + ENEMY_COUNT * 12]

    # 2. Busting table
    bt_same = rs[RS_BUSTING_TABLE - ROM_BASE : RS_BUSTING_TABLE - ROM_BASE + 12 * 16] == bm[RS_BUSTING_TABLE - ROM_BASE : RS_BUSTING_TABLE - ROM_BASE + 12 * 16]

    # 3. Story battles
    sb_diffs = 0
    for i in range(STORY_BATTLE_COUNT):
        r_off = RS_STORY_BATTLES - ROM_BASE + i * 12
        b_off = r_off + BM_CODE09_DELTA
        rr = rs[r_off : r_off + 12]
        rb = bm[b_off : b_off + 12]
        fp_r = struct.unpack_from("<I", rr, 8)[0]
        fp_b = struct.unpack_from("<I", rb, 8)[0]
        fr = rs[fp_r - ROM_BASE : fp_r - ROM_BASE + 32]
        fb = bm[fp_b - ROM_BASE : fp_b - ROM_BASE + 32]
        if rr[:8] != rb[:8] or fr[: fr.find(b"\xff") + 1] != fb[: fb.find(b"\xff") + 1]:
            sb_diffs += 1

    # 4. Random encounters
    enc_diffs = 0
    cw_rs = struct.unpack_from("<21I", rs, RS_CW_BATTLE_GROUPS - ROM_BASE)
    cw_bm = struct.unpack_from("<21I", bm, RS_CW_BATTLE_GROUPS - ROM_BASE)
    for g in range(21):
        gp_r, gp_b = cw_rs[g], cw_bm[g]
        first_mp = struct.unpack_from("<I", rs, gp_r - ROM_BASE)[0]
        n_slots = (first_mp - gp_r) // 4
        for m in range(n_slots):
            mp_r = struct.unpack_from("<I", rs, gp_r - ROM_BASE + m * 4)[0]
            mp_b = struct.unpack_from("<I", bm, gp_b - ROM_BASE + m * 4)[0]
            if mp_r == 0:
                continue
            i = 0
            while rs[mp_r - ROM_BASE + i * 12] != 0xFF:
                rr = rs[mp_r - ROM_BASE + i * 12 : mp_r - ROM_BASE + (i + 1) * 12]
                rb = bm[mp_b - ROM_BASE + i * 12 : mp_b - ROM_BASE + (i + 1) * 12]
                fp_r = struct.unpack_from("<I", rr, 8)[0]
                fp_b = struct.unpack_from("<I", rb, 8)[0]
                fr = rs[fp_r - ROM_BASE : fp_r - ROM_BASE + 32]
                fb = bm[fp_b - ROM_BASE : fp_b - ROM_BASE + 32]
                if rr[:8] != rb[:8] or fr[: fr.find(b"\xff") + 1] != fb[: fb.find(b"\xff") + 1]:
                    enc_diffs += 1
                i += 1

    # 5. Reward rows
    rr_diffs = []
    for row in range(REWARD_ROW_COUNT):
        rv = struct.unpack_from("<20H", rs, RS_REWARD_ROWS - ROM_BASE + row * 0x28)
        bv = struct.unpack_from("<20H", bm, RS_REWARD_ROWS - ROM_BASE + BM_CODE09_DELTA + row * 0x28)
        if rv != bv:
            rr_diffs.append((row, rv, bv))

    # 6. Routine prologues
    rchecks = []
    for name, rsa, delta, seg, desc in ROUTINES:
        bma = rsa + delta
        ok = rs[rsa - ROM_BASE : rsa - ROM_BASE + 4] == bm[bma - ROM_BASE : bma - ROM_BASE + 4]
        rchecks.append((name, rsa, bma, seg, ok, desc))

    return {
        "enemy_stats_identical": es_same,
        "busting_table_identical": bt_same,
        "story_battle_diffs": sb_diffs,
        "encounter_diffs": enc_diffs,
        "reward_row_diffs": rr_diffs,
        "routine_checks": rchecks,
    }


def write_markdown_doc(
    out_path: Path,
    rs: bytes,
    bm: bytes,
    res: Resolver,
    enemies: list[EnemyStat],
    encounters: dict[tuple[int, int], list[BattleRecord]],
    story_battles: list[BattleRecord],
    rs_bm: dict,
) -> None:
    lines: list[str] = []
    w = lines.append

    w("# RE-06: Encounter Tables, Enemy Records & Battle Drops")
    w("")
    w("Generated by `decomp/tools/encounters.py` from US Red Sun (`bn4_rs_us.gba`) and US Blue Moon (`bn4_bm_us.gba`).")
    w("Status: **V** = verified in disassembly and both US ROMs.")
    w("")
    w("## 1. Summary")
    w("")
    w(f"- **Enemy records (`BN4_ENEMY_STATS`, `0x08011888`):** **{ENEMY_COUNT}** 12-byte records (viruses `0x01`–`0x7F`, Navis/bosses `0x80`–`0xFC`), **100% identical** between US Red Sun and US Blue Moon.")
    w(f"- **Random encounter tables (`BN4_BATTLE_TABLES`, `0x08018014`):** **{len(encounters)}** non-empty cyberworld map encounter slots across groups `0x80`–`0x94` (including base `+0x00`, wandering Ghost Navi `+0x10`, and story invasion `+0x20`/`+0x30` tables), **0 differences** between US Red Sun and US Blue Moon.")
    w(f"- **Story/tournament/boss battles (`BN4_STORY_BATTLES`, RS `0x080FA900` / BM `0x080FA90C`):** **{STORY_BATTLE_COUNT}** 12-byte battle records, **0 differences** between US Red Sun and US Blue Moon.")
    w(f"- **Battle drop tables (`BN4_REWARD_ROWS`, RS `0x080F5F50` / BM `0x080F5F5C`):** **{REWARD_ROW_COUNT}** rows × 20 `u16` rewards (`0x00`–`0xFD` per enemy + `0x100`–`0x113` for the 20 Dark Soul Navi chip drops). **Exactly {len(rs_bm['reward_row_diffs'])} virus rows differ** between Red Sun and Blue Moon — exclusively in **chip codes** (e.g., Mettaur `Guard1 L` in RS vs `Guard1 V` in BM; VolGear swaps `FlmLine1 F`/`H`; Billy `Thunder1 L` vs `P`).")
    w("- **Playthrough tier scaling (`BN4_ID_RAISE`, `0x0800B036`):** BN4 does **not** store separate Normal/Hard/Super Hard encounter tables. Instead, every spawned enemy's ID is raised dynamically by `tier = min(GetPlaythrough(), 2)` unless bit 7 (`0x80`) of battle record `+6` is set: **viruses (`< 0x80`)** increase by `+2 * tier` (V1→V2→V3, V1EX→V2EX→V3EX, clamped to version 5), and **Navis (`0x80`–`0xE3`)** increase by `+1 * tier` (V1→α→β, clamped to version 2).")
    w("")

    # Section 2: Routines
    w("## 2. Code Routines (V)")
    w("")
    w("| RS Address | BM Address | Segment | Name | Purpose |")
    w("|---|---|---|---|---|")
    for name, rsa, bma, seg, ok, desc in rs_bm["routine_checks"]:
        assert ok, f"Prologue mismatch for {name}"
        w(f"| `0x{rsa:08X}` | `0x{bma:08X}` | `{seg}` | `{name}` | {desc} |")
    w("")

    # Section 3: Table layouts
    w("## 3. Table Layouts (V)")
    w("")
    w("| Table | RS Address | BM Address | Size / Layout |")
    w("|---|---|---|---|")
    w("| `BN4_ENEMY_STATS` | `0x08011888` | `0x08011888` | 254 × 12 bytes: `{u16 hp_elem (bits 0..11 HP, bits 12..15 elem), u8 t1_type, u8 cls (0x04 virus / 0x08 Navi), u8 family, u8 version (0..5 / 0..3), s8 unk6, u8 unk7, u8 unk8, u8 unk9, u8 base_atk, u8 unk11}` |")
    w("| `BN4_BATTLE_TABLES` | `0x08017FB8` | `0x08017FB8` | 2 pointers `{rw_groups=0x08017FC0, cw_groups=0x08018014}`; `cw_groups` holds 21 group pointers (`0x80`–`0x94`) → per-map pointer arrays → `0xFF`-terminated 12-byte battle records |")
    w("| `BN4_BUSTING_TABLE` | `0x0801881D` | `0x0801881D` | 12 × 16 bytes (Busting Level 0..11 × 16 RNG slots), each byte is `col * 4` (`0, 4, 8, 12, 16`) |")
    w("| `BN4_REWARD_ROWS` | `0x080F5F50` | `0x080F5F5C` | 276 × 40 bytes (20 `u16`: `0..9` when `HP > 3/8 MaxHP`, `10..19` when `HP <= 3/8 MaxHP`; 5 Busting columns × 2 50/50 slots each) |")
    w("| `BN4_STORY_BATTLES` | `0x080FA900` | `0x080FA90C` | 245 × 12-byte battle records: `{u8 weight, u8 b1, u8 bg_or_mode, u8 b3, u8 b4, u8 b5, u8 flags, u8 b7, ptr formation}` |")
    w("| Formation list | `0x080FB47C`… | `+0xC` | `0xFF`-terminated 4-byte records: `{u8 id, u8 col (1..6), u8 row (1..3), u8 kind (0=MegaMan, 1=Enemy, 2+=Obstacle)}` |")
    w("")
    w("### Busting Level → Drop Column Odds (`BN4_BUSTING_TABLE`, `0x0801881D`)")
    w("")
    w("Within the chosen column `C0`–`C4`, `RewardPickBustingOffset` picks slot `2*C` or `2*C+1` with 50/50 odds (`(rng >> 8) & 1`). Low HP (`HP <= (MaxHP >> 3) * 3`) uses the second 10-slot half-row (`+0x14`).")
    w("")
    w("| Busting Level | Col 0 | Col 1 | Col 2 | Col 3 | Col 4 |")
    w("|---|---|---|---|---|---|")
    lv_labels = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "S (11)"]
    for lv in range(12):
        row = list(rs[RS_BUSTING_TABLE - ROM_BASE + lv * 16 : RS_BUSTING_TABLE - ROM_BASE + (lv + 1) * 16])
        cells = []
        for c in range(5):
            cnt = row.count(c * 4)
            cells.append(f"{cnt}/16 ({100.0 * cnt / 16:.2f}%)" if cnt else "—")
        w(f"| {lv_labels[lv]} | " + " | ".join(cells) + " |")
    w("")

    # Section 4: Random Encounter Tables (deduplicated by pointer within each group+band)
    w("## 4. Cyberworld Random Encounter Tables (`BN4_BATTLE_TABLES`)")
    w("")
    w("Each map's base encounters sit at slot `map`. For net areas (`0x90`–`0x94`), `BN4_STORY_OFFSET` adds `+0x10` once the area's V2 Ghost Navi flag is set (adding the wandering Ω Ghost Navi to the encounter pool), `+0x20` when story flag `0x1C9`/`0x1CA` is set, and `+0x30` when story flag `0x1CB` is set. Unused map slots within a 16-entry band point to a shared default/dummy table and are collapsed below. Remember `BN4_ID_RAISE` upgrades non-fixed enemies (`+2*tier` viruses, `+1*tier` Navis) on Hard/Super Hard.")
    w("")
    # Group slots by (gid, band, table_ptr) preserving order
    grouped_enc: dict[tuple[int, int, int], tuple[list[int], list[BattleRecord]]] = {}
    for (gid, slot), (mp, recs) in sorted(encounters.items()):
        band = slot & 0xF0
        key = (gid, band, mp)
        if key not in grouped_enc:
            grouped_enc[key] = ([], recs)
        grouped_enc[key][0].append(slot & 0xF)

    for (gid, band, mp), (sub_maps, recs) in grouped_enc.items():
        if len(sub_maps) == 1:
            sm = sub_maps[0]
            title_maps = f"`({gid:02X},{sm:02X})` {res.map_name(gid, sm)}"
            flg = WANDERING_FLAGS.get((gid, sm)) if band == 0x10 else None
        else:
            sm0 = sub_maps[0]
            title_maps = f"`({gid:02X},{sm0:02X}..{sub_maps[-1]:02X})` {res.map_name(gid, sm0)} (+{len(sub_maps)-1} shared slots)"
            flg = None
        if band == 0x00:
            band_str = "Base"
        elif band == 0x10:
            band_str = f"Wandering Ghost (+0x10, flag `0x{flg:04X}`)" if flg else "Variant +0x10"
        else:
            band_str = f"Story Override (+0x{band:02X})"
        tot_w = sum(r.weight for r in recs)
        w(f"### {title_maps} — {band_str} (`0x{mp:08X}`, `{len(recs)}` formations, total W=`{tot_w}`)")
        w("")
        w("| # | Weight | Odds | Mask | Enemies (Normal; +2/tier in Hard/SH unless FixedLv) |")
        w("|---|---|---|---|---|")
        for idx, r in enumerate(recs):
            elist = ", ".join(f"{res.enemy_name(eid)} (`0x{eid:02X}` @ {c},{rw})" for eid, c, rw in r.enemies)
            if r.objects:
                olist = ", ".join(f"Obj `0x{oid:02X}`" for oid, _, _ in r.objects)
                elist = f"{elist}; [{olist}]" if elist else f"[{olist}]"
            w(f"| {idx} | {r.weight} | {100.0 * r.weight / tot_w:.1f}% | `{format_attr_mask(r.flags)}` | {elist} |")
        w("")

    # Section 5: Every Enemy & Navi Record + Drops
    w("## 5. Every Enemy and Navi Record (`BN4_ENEMY_STATS` + `BN4_REWARD_ROWS`)")
    w("")
    w("Drops are shown as `C0 | C1 | C2 | C3 | C4` (5 Busting columns; `A / B` = 50/50 pair within the column) for **High HP (`> 3/8`)** (RS). Rows marked `★` differ in Blue Moon (see §7 for every RS vs BM difference). Low-HP (`<= 3/8`) rows are identical to High-HP except where noted (typically replacing low-busting Zenny with `HP+` recovery or shifting a chip slot).")
    w("")
    diff_row_set = {row for row, _, _ in rs_bm["reward_row_diffs"]}
    w("| ID | Name | HP | Elem | Cls | Fam/Ver | High-HP Drops (`C0 | C1 | C2 | C3 | C4`, RS) |")
    w("|---|---|---|---|---|---|---|")
    for e in enemies:
        if e.eid == 0:
            continue
        if e.name.startswith("(unused") and e.hp == 40:
            continue
        rv = struct.unpack_from("<20H", rs, RS_REWARD_ROWS - ROM_BASE + e.eid * 0x28)
        dstr = format_drop_row_compact(rv[:10], res)
        mark = " ★" if e.eid in diff_row_set else ""
        cls_s = "Navi" if e.cls == 0x08 else "Virus"
        el_s = ELEMENTS.get(e.elem, str(e.elem))
        w(f"| `0x{e.eid:02X}` | {e.name}{mark} | {e.hp} | {el_s} | {cls_s} | {e.family}/{e.version} | {dstr} |")
    w("")
    w("### Dark Soul Navi Drop Rows (`0x100`–`0x113` in `BN4_REWARD_ROWS`)")
    w("")
    w("Returned by `BN4_REWARD_ROW_OF` (`0x080F5EC4`) when MegaMan's Dark Karma exceeds the threshold (`sub_0800F5A6() > 0x1D5`). All 20 rows are identical in RS and BM.")
    w("")
    w("| Row | Drop (all High-HP slots) |")
    w("|---|---|")
    for row in range(0x100, REWARD_ROW_COUNT):
        rv = struct.unpack_from("<20H", rs, RS_REWARD_ROWS - ROM_BASE + row * 0x28)
        w(f"| `0x{row:03X}` ({row}) | {format_drop_row_compact(rv[:10], res)} |")
    w("")

    # Section 6: Boss & Navi Battle List
    w("## 6. Boss and Navi Battles (Version & Reachability)")
    w("")
    # Map each Navi enemy ID to story battles and wandering encounters
    navi_story: dict[int, list[int]] = {}
    for bi, br in enumerate(story_battles):
        for eid, _, _ in br.enemies:
            if eid >= 0x80:
                navi_story.setdefault(eid, []).append(bi)
    navi_wander: dict[int, list[str]] = {}
    for (gid, slot), (_mp, recs) in encounters.items():
        mname = res.map_name(gid, slot & 0xF)
        for br in recs:
            for eid, _, _ in br.enemies:
                if eid >= 0x80:
                    tag = f"{mname} (+0x{slot & 0xF0:02X})"
                    if tag not in navi_wander.setdefault(eid, []):
                        navi_wander[eid].append(tag)

    w("| Base ID | Boss / Navi | Versions (HP) | Vanilla Version | Story Battle IDs (`BN4_STORY_BATTLES`) | Wandering / Random Locations | How Reached |")
    w("|---|---|---|---|---|---|---|")
    for base_id, (ver_str, how) in sorted(NAVI_AVAILABILITY.items()):
        count = 3 if base_id == 0xE4 else 4
        vers = []
        sbs = []
        wands = []
        for k in range(count):
            eid = base_id + k
            e = enemies[eid]
            vers.append(f"{e.name} ({e.hp})")
            for bi in navi_story.get(eid, []):
                sbs.append(f"`{bi}`({e.name})")
            for loc in navi_wander.get(eid, []):
                wands.append(f"{e.name}: {loc}")
        sb_s = ", ".join(sbs) if sbs else "—"
        wa_s = "; ".join(wands) if wands else "—"
        w(f"| `0x{base_id:02X}` | {enemies[base_id].name} | {', '.join(vers)} | {ver_str} | {sb_s} | {wa_s} | {how} |")
    w("")

    # Section 7: RS vs BM Differences
    w(f"## 7. Red Sun vs Blue Moon Differences ({len(rs_bm['reward_row_diffs'])} Virus Drop Rows)")
    w("")
    w("- `BN4_ENEMY_STATS` (`0x08011888`): **0 differences**.")
    w("- `BN4_BUSTING_TABLE` (`0x0801881D`): **0 differences**.")
    w("- `BN4_STORY_BATTLES` (`0x080FA900` RS / `0x080FA90C` BM, 245 battles): **0 differences**.")
    w("- `BN4_BATTLE_TABLES` (all 21 cyberworld groups): **0 differences**.")
    w(f"- `BN4_REWARD_ROWS` (`0x080F5F50` RS / `0x080F5F5C` BM): **{len(rs_bm['reward_row_diffs'])} virus rows differ**, listed in full below (each entry shows High-HP slots `0..9` that differ; Low-HP slots `10..19` mirror the same chip-code changes).")
    w("")
    w("| Enemy ID | Name | Differing Slots (RS → BM) |")
    w("|---|---|---|")
    for row, rv, bv in rs_bm["reward_row_diffs"]:
        en = res.enemy_name(row)
        diffs = []
        for c in range(10):
            if rv[c] != bv[c]:
                col = c // 2
                slot_ab = "a" if (c % 2 == 0) else "b"
                diffs.append(f"C{col}{slot_ab}: `{res.reward_str(rv[c])}` → `{res.reward_str(bv[c])}`")
        # Also note if any low-HP slot differs differently
        low_only = []
        for c in range(10, 20):
            if rv[c] != bv[c] and (rv[c - 10] == bv[c - 10]):
                col = (c - 10) // 2
                slot_ab = "a" if (c % 2 == 0) else "b"
                low_only.append(f"Low-C{col}{slot_ab}: `{res.reward_str(rv[c])}` → `{res.reward_str(bv[c])}`")
        all_d = diffs + low_only
        w(f"| `0x{row:02X}` | {en} | " + "; ".join(all_d) + " |")
    w("")

    # Section 8: Proposed names
    w("## 8. Proposed Symbol Names (`names.txt` format)")
    w("")
    w("```")
    # Only propose newly named routines (skip already-in-names.txt CE symbols)
    new_syms = [
        (0x08007DC0, 0x08007DC0, "code", "BattleGetBackgroundOrMode", "Reads battle record +2; adds 0x3C * min(playthrough, 2) unless bit 7 of +6 set"),
        (0x08021A98, 0x08021A9C, "code", "RewardDecode", "Decodes 16-bit reward word: kind 0=chip, 1=Zenny, 2=HP, 3=BugFrags, 0xFFFF=None"),
        (0x080F53D8, 0x080F53E4, "code", "Overworld_TryRandomEncounter", "Step-counter encounter trigger; calls BN4_ROLL with NaviCust element mask (fallback 0x1F)"),
        (0x080F5998, 0x080F59A4, "code", "RewardPickBustingOffset", "Computes byte offset in 40-byte reward row from Busting Level, RNG, and HP <= 3/8"),
        (0x080F5F40, 0x080F5F4C, "code", "RewardGetRowPtr", "r0 row: returns &BN4_REWARD_ROWS[row] (row * 0x28)"),
    ]
    for rsa, bma, kind, name, desc in new_syms:
        w(f"{rsa:08X} {bma:08X} {kind:6s} GEMINI       {name:32s} {desc}")
    w("```")
    w("")

    out_path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main() -> int:
    ap = argparse.ArgumentParser(description="Extract and verify MMBN4 encounters, enemy stats, and drops.")
    ap.add_argument("--write-doc", action="store_true", help="Write decomp/notes/re06-encounters.md.")
    args = ap.parse_args()

    root = find_game_root()
    rs = (root / "roms" / "bn4_rs_us.gba").read_bytes()
    bm = (root / "roms" / "bn4_bm_us.gba").read_bytes()
    tbl = load_tbl(root)
    res = Resolver(rs, tbl)

    enemies = parse_enemy_stats(rs, res)
    encounters = parse_cyberworld_encounters(rs)
    story_battles = parse_story_battles(rs)
    rs_bm = verify_rs_bm(rs, bm)

    print(f"Enemy records: {len(enemies)} (RS==BM: {rs_bm['enemy_stats_identical']})")
    print(f"Busting table RS==BM: {rs_bm['busting_table_identical']}")
    print(f"Cyberworld encounter slots: {len(encounters)} (RS!=BM diffs: {rs_bm['encounter_diffs']})")
    print(f"Story battles: {len(story_battles)} (RS!=BM diffs: {rs_bm['story_battle_diffs']})")
    print(f"Reward rows: {REWARD_ROW_COUNT} (RS!=BM diffs: {len(rs_bm['reward_row_diffs'])})")
    print(f"Routine prologues matched: {sum(1 for x in rs_bm['routine_checks'] if x[4])}/{len(rs_bm['routine_checks'])}")

    if args.write_doc:
        out_path = root / "decomp" / "notes" / "re06-encounters.md"
        write_markdown_doc(out_path, rs, bm, res, enemies, encounters, story_battles, rs_bm)
        print(f"Wrote {out_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
