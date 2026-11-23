#!/usr/bin/env python3
"""DC-4 support: catalogue every Red Sun / Blue Moon difference in the code
region (0x08000000-0x08120000).

Reads the two ROMs directly (`roms/bn4_rs_us.gba`, `roms/bn4_bm_us.gba`) along
with `config/segments_*.toml` and `symbols/{names,functions_rs,map_rs_bm}.txt`
in read-only mode. Does not modify or build anything.

Usage:
    python3 tools/vdiff.py              # print summary and shift/site tables
    python3 tools/vdiff.py --markdown   # emit the full markdown report
"""
from __future__ import annotations

import re
import struct
import subprocess
import sys
import tomllib
from collections import Counter, defaultdict
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
GAME_ROOT = DECOMP.parent
REPO_ROOT = GAME_ROOT.parent
BASE = 0x08000000
CODE_END = 0x08120000

# Piecewise-constant RS -> BM delta intervals covering 0x08000000..0x08120000.
# Transitions between consecutive intervals are exact insertion/deletion sites
# where the RS->BM shift changes.
DELTA_SPANS: list[tuple[int, int, int, str]] = [
    (0x08000000, 0x08016A68, +0x0,  "header..code_01 (up to BN4_FACE_GET)"),
    (0x08016A68, 0x08016A6A, +0x2,  "code_01: inside BN4_FACE_GET (after BM +2 insn, before RS +2 pad)"),
    (0x08016A6A, 0x08020D64, +0x0,  "code_01: after BN4_FACE_GET up to table at 0x08020CE0"),
    (0x08020D66, 0x0803503C, +0x4,  "code_01 tail + code_02 (up to sub_08035020)"),
    (0x0803503C, 0x0804F208, +0x8,  "code_02 tail + code_03 (up to ScriptCmd_checkGameVersion)"),
    (0x0804F208, 0x0804F21C, +0xA,  "code_03: inside ScriptCmd_checkGameVersion (before BM +2 pad)"),
    (0x0804F21C, 0x0805C19C, +0xC,  "code_04 part 1 (code + cutscene scripts up to 0x0805C19C)"),
    (0x0805C1AA, 0x0805C266, -0x2,  "code_04 cutscene script block 1a (-0xE deletion at 0x0805C19C)"),
    (0x0805C26A, 0x0805C80C, -0x4,  "code_04 cutscene script block 1b (-0x2 alignment pad at 0x0805C266)"),
    (0x0805C80C, 0x0805C8B2, +0xA,  "code_04 cutscene script block 1c (+0xE insertion at 0x0805C80C)"),
    (0x0805C8B6, 0x0805CE02, +0x8,  "code_04 cutscene script block 1d (-0x2 alignment pad at 0x0805C8B2)"),
    (0x0805CE02, 0x0805CE50, +0x16, "code_04 cutscene script block 1e (+0xE insertion at 0x0805CE02)"),
    (0x0805CE52, 0x080609EE, +0x18, "code_04 part 2 (+0x2 pad at 0x0805CE50; functions 0x0805D4AC..0x08060264)"),
    (0x080609FC, 0x08060AFE, +0xA,  "code_04 cutscene script block 2a (-0xE deletion at 0x080609EE)"),
    (0x08060B02, 0x08060B32, +0x8,  "code_04 cutscene script block 2b (-0x2 alignment pad at 0x08060AFE)"),
    (0x08060B40, 0x08060BA2, -0x6,  "code_04 cutscene script block 2c (-0xE deletion at 0x08060B32)"),
    (0x08060BA6, 0x08060D00, -0x8,  "code_04 cutscene script block 2d (-0x2 alignment pad at 0x08060BA2)"),
    (0x08060D00, 0x08060D98, +0x6,  "code_04 cutscene script block 2e (+0xE insertion at 0x08060D00)"),
    (0x08060D9A, 0x08061078, +0x8,  "code_04 cutscene script block 2f (+0x2 alignment pad at 0x08060D98)"),
    (0x08061078, 0x080610CA, +0x16, "code_04 cutscene script block 2g (+0xE insertion at 0x08061078)"),
    (0x080610CE, 0x080656DC, +0x14, "code_04 part 3 (-0x2 pad at 0x080610CA; functions 0x080612C8..0x0806544C)"),
    (0x080656EC, 0x080657C4, +0x6,  "code_04 cutscene script block 3a (-0xE deletion at 0x080656DC)"),
    (0x080657C6, 0x08066366, +0x8,  "code_04 cutscene script block 3b (+0x2 alignment pad at 0x080657C4)"),
    (0x08066378, 0x080663E0, -0x6,  "code_04 cutscene script block 3c (-0xE deletion at 0x08066366)"),
    (0x080663E2, 0x08066520, -0x4,  "code_04 cutscene script block 3d (+0x2 alignment pad at 0x080663E0)"),
    (0x08066524, 0x08109EB2, +0xC,  "code_04 part 4 (+0x10 ins at 0x08066520) + code_05..code_09"),
    (0x08109EB2, 0x08109ED8, +0x12, "code_09: inside sub_08109E48 (after BM +6 insns, before BM +2 pad)"),
    (0x08109ED8, 0x08120000, +0x14, "code_09 tail (from literal pool 0x08109ED8) + code_10"),
]


def true_rs_to_bm_delta(rs_addr: int) -> int | None:
    a = rs_addr & ~1
    for s, e, d, _ in DELTA_SPANS:
        if s <= a < e:
            return d
    return None


def decode_bl(rom: bytes, addr: int) -> int:
    h1, h2 = struct.unpack_from("<HH", rom, addr - BASE)
    off = ((h1 & 0x7FF) << 12) | ((h2 & 0x7FF) << 1)
    if off & 0x400000:
        off -= 0x800000
    return (addr + 4 + off) & 0xFFFFFFFF


def is_thumb_bl(rom: bytes, addr: int) -> bool:
    if addr - BASE < 0 or addr - BASE + 4 > len(rom):
        return False
    h1, h2 = struct.unpack_from("<HH", rom, addr - BASE)
    return (h1 & 0xF800) == 0xF000 and (h2 & 0xF800) == 0xF800


def load_names() -> dict[int, str]:
    names: dict[int, str] = {}
    for line in (DECOMP / "symbols" / "names.txt").read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        parts = line.split(None, 5)
        if len(parts) >= 5:
            names[int(parts[0], 16)] = parts[4]
    return names


def load_funcs() -> list[tuple[int, int, str]]:
    funcs: list[tuple[int, int, str]] = []
    for line in (DECOMP / "symbols" / "functions_rs.txt").read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        a, mode, sz, _, _ = line.split()
        funcs.append((int(a, 16), int(sz), mode))
    funcs.sort()
    return funcs


def enclosing_func(rs_addr: int, funcs: list[tuple[int, int, str]], names: dict[int, str]) -> str:
    lo, hi = 0, len(funcs) - 1
    best = None
    while lo <= hi:
        mid = (lo + hi) // 2
        if funcs[mid][0] <= rs_addr:
            best = funcs[mid]
            lo = mid + 1
        else:
            hi = mid - 1
    if best and rs_addr < best[0] + best[1]:
        fa = best[0]
        sym = names.get(fa)
        return f"`{sym}` (`sub_{fa:08X}`)" if sym and sym != f"sub_{fa:08X}" else f"`sub_{fa:08X}`"
    sym = names.get(rs_addr)
    return f"`{sym}`" if sym else "(data / pool)"


def find_pre_merge_rev() -> str:
    res = subprocess.run(
        ["git", "log", "-n", "1", "--format=%H", "--", "MMBN4-Decomp/decomp/asm/bm/code_00.s"],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )
    h = res.stdout.strip()
    if not h:
        return "HEAD"
    chk = subprocess.run(
        ["git", "cat-file", "-e", f"{h}:MMBN4-Decomp/decomp/asm/bm/code_00.s"],
        cwd=REPO_ROOT,
        capture_output=True,
    )
    return h if chk.returncode == 0 else f"{h}~1"


def parse_pre_merge_asm(rev: str, ver: str, seg_cfg: dict) -> dict[int, tuple[int, str, str]]:
    name = seg_cfg["name"]
    rel = f"{rev}:MMBN4-Decomp/decomp/asm/{'bm/' if ver == 'bm' else ''}{name}.s"
    res = subprocess.run(["git", "show", rel], cwd=REPO_ROOT, capture_output=True, text=True)
    text = res.stdout
    cur = seg_cfg["start"]
    is_thumb = seg_cfg.get("mode") != "arm"
    items: dict[int, tuple[int, str, str]] = {}

    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("@"):
            continue
        if line.startswith((".thumb_func", ".thumb")):
            is_thumb = True
            continue
        if line.startswith(".arm"):
            is_thumb = False
            continue
        if line.startswith((".section", ".syntax", ".cpu", ".global", ".equiv", ".include", ".align", ".set")):
            continue
        if line.endswith(":"):
            continue
        code_part = line.split("@")[0].strip()
        if code_part.endswith(":"):
            continue
        if code_part.startswith("incrom "):
            m = re.match(r"incrom\s+0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)", code_part)
            sz = int(m.group(2), 16)
            items[cur] = (sz, "incrom", code_part)
            cur += sz
        elif code_part.startswith(".word "):
            items[cur] = (4, ".word", code_part)
            cur += 4
        elif code_part.startswith(".hword "):
            items[cur] = (2, ".hword", code_part)
            cur += 2
        elif code_part.startswith(".byte "):
            n = len(code_part[6:].split(","))
            items[cur] = (n, ".byte", code_part)
            cur += n
        else:
            sz = 4 if (not is_thumb or code_part.startswith(("bl ", "bl\t"))) else 2
            items[cur] = (sz, "insn", code_part)
            cur += sz
    return items


def analyze():
    rs_rom = (GAME_ROOT / "roms" / "bn4_rs_us.gba").read_bytes()
    bm_rom = (GAME_ROOT / "roms" / "bn4_bm_us.gba").read_bytes()
    rev = find_pre_merge_rev()
    cfg_rs = tomllib.loads(
        subprocess.run(["git", "show", f"{rev}:MMBN4-Decomp/decomp/config/segments_rs.toml"], cwd=REPO_ROOT, capture_output=True, text=True).stdout
    )
    cfg_bm = tomllib.loads(
        subprocess.run(["git", "show", f"{rev}:MMBN4-Decomp/decomp/config/segments_bm.toml"], cwd=REPO_ROOT, capture_output=True, text=True).stdout
    )
    names = load_names()
    funcs = load_funcs()

    rs_items: dict[int, tuple[int, str, str]] = {}
    bm_items: dict[int, tuple[int, str, str]] = {}
    for s_rs, s_bm in zip(cfg_rs["segment"], cfg_bm["segment"]):
        if s_rs["kind"] != "code":
            continue
        rs_items.update(parse_pre_merge_asm(rev, "rs", s_rs))
        bm_items.update(parse_pre_merge_asm(rev, "bm", s_bm))

    def get_seg_name(rs_addr: int) -> str:
        for s in cfg_rs["segment"]:
            if s["start"] <= rs_addr < s["end"]:
                return s["name"]
        return "unknown"

    counts_by_seg: dict[str, Counter] = defaultdict(Counter)
    sites_imm: list[tuple] = []
    sites_branch_local: list[tuple] = []
    sites_word_const: list[tuple] = []
    sites_code_ptr_diff: list[tuple] = []
    sites_data_ptr: list[tuple] = []
    sites_incrom_const: list[tuple] = []
    sites_code_ptr_shifted: list[tuple] = []

    for s, e, d, _ in DELTA_SPANS:
        i = s
        while i < e:
            b = i + d
            seg = get_seg_name(i)
            r_it = rs_items.get(i)
            b_it = bm_items.get(b)

            # 1. Decoded 4-byte item (.word or 4-byte instruction)
            if r_it and b_it and r_it[0] == 4 and b_it[0] == 4 and r_it[1] != "incrom" and b_it[1] != "incrom":
                rw = struct.unpack_from("<I", rs_rom, i - BASE)[0]
                bw = struct.unpack_from("<I", bm_rom, b - BASE)[0]
                if rw != bw:
                    if (r_it[1] == "insn" and r_it[2].startswith("bl")) or (is_thumb_bl(rs_rom, i) and is_thumb_bl(bm_rom, b)):
                        t_rs = decode_bl(rs_rom, i)
                        t_bm = decode_bl(bm_rom, b)
                        td = true_rs_to_bm_delta(t_rs)
                        if td is not None and t_rs + td == t_bm:
                            counts_by_seg[seg]["bl_shifted"] += 1
                        else:
                            counts_by_seg[seg]["code_ptr_diff"] += 1
                            sites_code_ptr_diff.append((seg, i, b, f"bl {t_rs:#010x}", f"bl {t_bm:#010x}", enclosing_func(i, funcs, names)))
                    elif is_thumb_bl(rs_rom, i - 2) and is_thumb_bl(bm_rom, b - 2) and (rw >> 16) == (bw >> 16):
                        t_rs = decode_bl(rs_rom, i - 2)
                        t_bm = decode_bl(bm_rom, b - 2)
                        td = true_rs_to_bm_delta(t_rs)
                        if td is not None and t_rs + td == t_bm:
                            counts_by_seg[seg]["bl_shifted"] += 1
                    elif 0x08000000 <= (rw & 0x7FFFFFFF) < CODE_END and 0x08000000 <= (bw & 0x7FFFFFFF) < CODE_END and (rw & 0x80000000) == (bw & 0x80000000):
                        td = true_rs_to_bm_delta(rw & 0x7FFFFFFF)
                        if td is not None and (rw & 0x7FFFFFFF) + td == (bw & 0x7FFFFFFF):
                            counts_by_seg[seg]["code_ptr_shifted"] += 1
                            sites_code_ptr_shifted.append((seg, i, b, rw, bw, "pool", enclosing_func(i, funcs, names)))
                        else:
                            counts_by_seg[seg]["code_ptr_diff"] += 1
                            sites_code_ptr_diff.append((seg, i, b, f"{rw:#010x} ({r_it[2]})", f"{bw:#010x} ({b_it[2]})", enclosing_func(i, funcs, names)))
                    elif CODE_END <= (rw & 0x7FFFFFFF) < 0x08800000 and CODE_END <= (bw & 0x7FFFFFFF) < 0x08800000 and (rw & 0x80000000) == (bw & 0x80000000):
                        counts_by_seg[seg]["data_ptr_pool"] += 1
                        sites_data_ptr.append((seg, i, b, rw, bw, "pool", enclosing_func(i, funcs, names)))
                    else:
                        counts_by_seg[seg]["const_word"] += 1
                        sites_word_const.append((seg, i, b, rw, bw, r_it[2], b_it[2], enclosing_func(i, funcs, names)))
                i += 4
                continue

            # 2. Decoded 2-byte item (insn or .hword)
            if r_it and b_it and r_it[0] == 2 and b_it[0] == 2 and r_it[1] != "incrom" and b_it[1] != "incrom":
                rh = struct.unpack_from("<H", rs_rom, i - BASE)[0]
                bh = struct.unpack_from("<H", bm_rom, b - BASE)[0]
                if rh != bh:
                    fn = enclosing_func(i, funcs, names)
                    if any(r_it[2].startswith(p) for p in ("b ", "bne ", "beq ", "ldr ")) and any(b_it[2].startswith(p) for p in ("b ", "bne ", "beq ", "ldr ")) and i not in (0x08035042,):
                        counts_by_seg[seg]["local_disp_shifted"] += 1
                        sites_branch_local.append((seg, i, b, rh, bh, r_it[2], b_it[2], fn))
                    elif i == 0x0803502C:
                        counts_by_seg[seg]["local_disp_shifted"] += 1
                        sites_branch_local.append((seg, i, b, rh, bh, f"{r_it[2]} (ldr r6, [pc, #0x1a8])", f"{b_it[2]} (ldr r6, [pc, #0x1ac])", fn))
                    else:
                        counts_by_seg[seg]["imm_insn"] += 1
                        sites_imm.append((seg, i, b, rh, bh, r_it[2], b_it[2], fn))
                i += 2
                continue

            # 3. Inside incrom / header: check if 2-byte aligned unreached Thumb BL
            if (i % 2 == 0) and (i + 2 <= e) and rs_rom[i - BASE : i - BASE + 2] != bm_rom[b - BASE : b - BASE + 2]:
                if i - 2 >= s and is_thumb_bl(rs_rom, i - 2) and is_thumb_bl(bm_rom, b - 2):
                    t_rs = decode_bl(rs_rom, i - 2)
                    t_bm = decode_bl(bm_rom, b - 2)
                    td = true_rs_to_bm_delta(t_rs)
                    if td is not None and t_rs + td == t_bm:
                        counts_by_seg[seg]["bl_shifted_incrom"] += 1
                        i += 2
                        continue

            if rs_rom[i - BASE] == bm_rom[b - BASE]:
                i += 1
                continue

            matched_ptr = False
            for off in (0, -1, -2, -3):
                p = i + off
                if s <= p and p + 4 <= e:
                    rw = struct.unpack_from("<I", rs_rom, p - BASE)[0]
                    bw = struct.unpack_from("<I", bm_rom, p + d - BASE)[0]
                    rw_c = rw & 0x7FFFFFFF
                    bw_c = bw & 0x7FFFFFFF
                    if (rw_c >> 24) == 0x08 and (bw_c >> 24) == 0x08 and (rw & 0x80000000) == (bw & 0x80000000) and rw != bw:
                        if rw_c < CODE_END and bw_c < CODE_END:
                            td = true_rs_to_bm_delta(rw_c)
                            if td is not None and rw_c + td == bw_c:
                                counts_by_seg[seg]["code_ptr_shifted_incrom"] += 1
                                sites_code_ptr_shifted.append((seg, p, p + d, rw, bw, "incrom", enclosing_func(p, funcs, names)))
                                i = p + 4
                                matched_ptr = True
                                break
                            elif abs(bw_c - rw_c) <= 0x40:
                                counts_by_seg[seg]["code_ptr_diff"] += 1
                                sites_code_ptr_diff.append((seg, p, p + d, f"{rw:#010x} (incrom)", f"{bw:#010x} (incrom)", enclosing_func(p, funcs, names)))
                                i = p + 4
                                matched_ptr = True
                                break
                        elif rw_c >= CODE_END and bw_c >= CODE_END:
                            counts_by_seg[seg]["data_ptr_incrom"] += 1
                            sites_data_ptr.append((seg, p, p + d, rw, bw, "incrom", enclosing_func(p, funcs, names)))
                            i = p + 4
                            matched_ptr = True
                            break
            if not matched_ptr:
                counts_by_seg[seg]["const_byte_incrom"] += 1
                sites_incrom_const.append((seg, i, b, rs_rom[i - BASE], bm_rom[b - BASE], enclosing_func(i, funcs, names)))
                i += 1

    # Cluster incrom constant byte diffs (gap <= 32 bytes within same segment)
    incrom_clusters = []
    for seg, r_a, b_a, rb, bb, fn in sites_incrom_const:
        if incrom_clusters and incrom_clusters[-1][0] == seg and r_a - incrom_clusters[-1][2] <= 32:
            incrom_clusters[-1][2] = r_a + 1
            incrom_clusters[-1][4] = b_a + 1
            incrom_clusters[-1][5] += 1
        else:
            incrom_clusters.append([seg, r_a, r_a + 1, b_a, b_a + 1, 1, fn])

    # Cluster incrom data-pointer sites into contiguous/nearby table spans (gap <= 64 bytes)
    data_ptr_incrom_clusters = []
    for seg, r_a, b_a, rw, bw, kind, fn in sites_data_ptr:
        if kind != "incrom":
            continue
        d_ptr = (bw & 0x7FFFFFFF) - (rw & 0x7FFFFFFF)
        if data_ptr_incrom_clusters and data_ptr_incrom_clusters[-1][0] == seg and r_a - data_ptr_incrom_clusters[-1][2] <= 64:
            c = data_ptr_incrom_clusters[-1]
            c[2] = r_a + 4
            c[4] = b_a + 4
            c[5] += 1
            c[8] = min(c[8], rw & 0x7FFFFFFF)
            c[9] = max(c[9], rw & 0x7FFFFFFF)
            c[10].add(d_ptr)
        else:
            data_ptr_incrom_clusters.append([seg, r_a, r_a + 4, b_a, b_a + 4, 1, rw, bw, rw & 0x7FFFFFFF, rw & 0x7FFFFFFF, {d_ptr}])

    return {
        "rs_rom": rs_rom,
        "bm_rom": bm_rom,
        "cfg_rs": cfg_rs,
        "cfg_bm": cfg_bm,
        "counts_by_seg": counts_by_seg,
        "sites_imm": sites_imm,
        "sites_branch_local": sites_branch_local,
        "sites_word_const": sites_word_const,
        "sites_code_ptr_diff": sites_code_ptr_diff,
        "sites_data_ptr": sites_data_ptr,
        "sites_incrom_const": sites_incrom_const,
        "sites_code_ptr_shifted": sites_code_ptr_shifted,
        "incrom_clusters": incrom_clusters,
        "data_ptr_incrom_clusters": data_ptr_incrom_clusters,
    }


def generate_markdown(data: dict) -> str:
    rs_rom = data["rs_rom"]
    bm_rom = data["bm_rom"]
    cfg_rs = data["cfg_rs"]
    counts = data["counts_by_seg"]
    out: list[str] = []

    out.append("# RS / BM Version Differences in the Code Region (`0x08000000`–`0x08120000`)")
    out.append("")
    out.append("Generated by `decomp/tools/vdiff.py` for Milestone **DC-4** (`tools/merge_bm.py` shared-source merge and segment splitting).")
    out.append("")
    out.append("## 1. Executive Summary & Per-Segment Counts")
    out.append("")
    out.append("Across the 1,179,648-byte (`0x120000`) Red Sun code region (`0x08000000`–`0x08120000`, corresponding to Blue Moon `0x08000000`–`0x08120014`), every difference between Red Sun (`bn4_rs_us.gba`) and Blue Moon (`bn4_bm_us.gba`) falls into either:")
    out.append("1. **Shift changes (insertions / deletions):** 5 functions with version-specific instructions (totaling +0x14 bytes in BM) and 3 cutscene/map script data tables in `code_04` with version-specific 14-byte `0x3E`/`0x3C` event-check commands (net +0xC, -0x4, and -0x8 bytes in BM, summing to 0 across `code_04`).")
    out.append("2. **Equal-length differing sites:**")
    out.append("   - **Shifted same-target code references:** 17,487 decoded Thumb `bl` calls (+ 15 unreached `bl` calls in dead-code `incrom` blocks), 3,832 decoded `.word` code pointers, 30,625 `incrom` code pointers (many at unaligned bytecode offsets in `code_04`–`code_10`), and 12 intra-function PC-relative branch/`ldr` displacements.")
    out.append("   - **True code/constant differences:** 36 decoded 16-bit instruction/halfword immediates, 10 decoded 32-bit `.word` constants, 19 non-trivial code-region pointers (2 version-specific battle object pointer slots in `code_00` + 17 intra-table pointers in `code_04`–`code_06` that cross local cutscene script insertions/deletions), and 538 constant bytes across 105 `incrom` table clusters.")
    out.append("   - **Data-region pointers (`0x08120000`–`0x087FFFFF`, including bit-31-flagged `0x88xxxxxx` compressed asset pointers):** 644 decoded literal-pool `.word` pointers and 5,158 `incrom` table pointers (5,802 total).")
    out.append("")
    out.append("| Segment | RS Range | BM Range | Current `bm_delta` | Shifted `bl` (decoded / `incrom`) | Shifted Code Ptrs (pool / `incrom`) | Local PC Disp | Immediates & `.hword` | `.word` Consts | Non-Trivial Code Ptrs | Data Ptrs (pool) | Data Ptrs (`incrom`) | `incrom` Const Bytes (Clusters) |")
    out.append("|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|")

    cluster_cnt_by_seg = Counter(c[0] for c in data["incrom_clusters"])
    totals = Counter()
    for s_rs, s_bm in zip(cfg_rs["segment"], data["cfg_bm"]["segment"]):
        name = s_rs["name"]
        c = counts[name]
        totals.update(c)
        d = s_bm["start"] - s_rs["start"]
        out.append(
            f"| `{name}` | `0x{s_rs['start']:08X}`–`0x{s_rs['end']:08X}` | `0x{s_bm['start']:08X}`–`0x{s_bm['end']:08X}` | `{d:+#x}` | "
            f"{c['bl_shifted']} / {c['bl_shifted_incrom']} | {c['code_ptr_shifted']} / {c['code_ptr_shifted_incrom']} | "
            f"{c['local_disp_shifted']} | {c['imm_insn']} | {c['const_word']} | {c['code_ptr_diff']} | "
            f"{c['data_ptr_pool']} | {c['data_ptr_incrom']} | {c['const_byte_incrom']} ({cluster_cnt_by_seg[name]}) |"
        )
    out.append(
        f"| **TOTAL** | `0x08000000`–`0x08120000` | `0x08000000`–`0x08120014` | `+0x0`..`+0x14` | "
        f"**{totals['bl_shifted']} / {totals['bl_shifted_incrom']}** | **{totals['code_ptr_shifted']} / {totals['code_ptr_shifted_incrom']}** | "
        f"**{totals['local_disp_shifted']}** | **{totals['imm_insn']}** | **{totals['const_word']}** | **{totals['code_ptr_diff']}** | "
        f"**{totals['data_ptr_pool']}** | **{totals['data_ptr_incrom']}** | **{totals['const_byte_incrom']} ({len(data['incrom_clusters'])})** |"
    )
    out.append("")

    # Section 2: Shift changes
    out.append("## 2. Shift Changes (Every RS→BM Delta Transition in `0x08000000`–`0x08120000`)")
    out.append("")
    out.append("There are **11 code & segment-level delta transitions** (in 4 functions and 4 data tables) that affect function start addresses, plus **19 internal table delta transitions** inside the 3 cutscene script `incrom` blocks in `code_04`. In addition, 4 regions in `code_01`, `code_05`, and `code_10` have constant-delta RS/BM bytes where `survey.py` split an `incrom` or function boundary differently between versions.")
    out.append("")
    out.append("### 2.1 Code & Segment-Level Shift Changes")
    out.append("")
    out.append("| # | Segment | RS Address | BM Address | Old Delta | New Delta | Byte Change | Kind & Enclosing Symbol | What Changed |")
    out.append("|---:|---|---|---|---|---:|---:|---:|---|---|")
    code_shifts = [
        (1, "code_01", "0x08016A68", "0x08016A68", "+0x0", "+0x2", "+2 B (BM)", "Code in `BN4_FACE_GET` (`sub_08016A20`)", "BM inserts `subs r0, #6` (`06 38`) before `pop {r4, pc}` (`bd 10`)."),
        (2, "code_01", "0x08016A6A", "0x08016A6C", "+0x2", "+0x0", "-2 B (BM)", "Padding at end of `BN4_FACE_GET` (`sub_08016A20`)", "RS has 2 bytes of `.hword 0x0000` alignment padding before literal pool `_08016A6C`; BM needs no padding, immediately restoring delta `+0x0`."),
        (3, "code_01", "0x08020D64", "0x08020D64", "+0x0", "+0x4", "+4 B (BM)", "Data table in `incrom 0x20CE0` (after `sub_08020C44`)", "Pointer at `0x08020D60` adjusts (`0x08020D68` → `0x08020D6C`) and entry at `0x08020D64` expands from 2 bytes (`50 42`) in RS to 6 bytes (`50 12 72 22 72 42`) in BM."),
        (4, "code_02", "0x0803503C", "0x08035040", "+0x4", "+0x8", "+4 B (BM)", "Code in `sub_08035020` (BM `sub_08035024`)", "BM inserts `cmp r1, #0x79; blt _0803504C` (`79 29 03 db`, 4 bytes) and changes `cmp r1, #0x7f; bge` to `cmp r1, #0x7e; ble`."),
        (5, "code_03", "0x0804F208", "0x0804F210", "+0x8", "+0xA", "+2 B (BM)", "Code in `ScriptCmd_checkGameVersion` (`sub_0804F204`)", "BM inserts `movs r0, #3` (`03 20`, 2 bytes) immediately after `movs r0, #2` (`02 20`)."),
        (6, "code_03", "0x0804F21C", "0x0804F226", "+0xA", "+0xC", "+2 B (BM)", "Padding after `ScriptCmd_checkGameVersion`", "BM appends `.hword 0x0000` alignment padding at `0x0804F226`–`0x0804F228` before `sub_0804F21C` (BM `sub_0804F228`)."),
        (7, "code_04", "0x0805C19C..0x0805CE52", "0x0805C1A8..0x0805CE6A", "+0xC", "+0x18", "+12 B net (BM)", "Cutscene script data (`incrom 0x5AFC6, 0x1F96` vs `0x1FA2`)", "RS deletes one 14B `0x3E` script command (-14B, -2B pad) and BM inserts two 14B `0x3E` commands (+28B, -2B, +2B pad) = net `+0xC` (`+0xC` → `+0x18`). Every function in `0x0805D4AC`–`0x08060264` is shifted by **`+0x18`**."),
        (8, "code_04", "0x080609EE..0x080610CE", "0x08060A06..0x080610E2", "+0x18", "+0x14", "-4 B net (BM)", "Cutscene script data (`incrom 0x60264, 0x1064` vs `0x1060`)", "RS has two 14B `0x3E` script commands and BM has two 14B `0x3E` commands at different points, plus 4-byte pointer table alignment padding changes (-2B, -2B, +2B, -2B) = net **`-0x4`** (`+0x18` → `+0x14`). Every function in `0x080612C8`–`0x0806544C` is shifted by **`+0x14`**."),
        (9, "code_04", "0x080656DC..0x08066524", "0x080656F0..0x08066530", "+0x14", "+0xC", "-8 B net (BM)", "Cutscene script data (`incrom 0x6544C, 0x2210` vs `0x2208`)", "RS has two 14B `0x3E` script commands (-14B + 2B pad twice = -24B) and BM has one 16B `0x04`/`0x16` command (+16B) = net **`-0x8`** (`+0x14` → `+0xC`). Functions from `0x08067B3C` onward return to **`+0xC`**."),
        (10, "code_09", "0x08109EB2", "0x08109EBE", "+0xC", "+0x12", "+6 B (BM)", "Code in `sub_08109E48` (BM `sub_08109E54`)", "BM inserts 3 instructions (`tst r1, r1; bne _08109EC4; adds r1, #1`, bytes `09 42 00 d1 01 31`, 6 bytes) before `push {r5}`."),
        (11, "code_09", "0x08109ED8", "0x08109EEA", "+0x12", "+0x14", "+2 B (BM)", "Padding before literal pool `_08109ED8` in `sub_08109E48`", "BM inserts `.hword 0x0000` alignment padding at `0x08109EEA` so literal pool `_08109EEC` is 4-byte aligned. From RS `0x08109ED8` through `0x08120000`, the delta is **`+0x14`**."),
    ]
    for row in code_shifts:
        out.append(f"| {row[0]} | `{row[1]}` | `{row[2]}` | `{row[3]}` | `{row[4]}` | `{row[5]}` | `{row[6]}` | {row[7]} | {row[8]} |")
    out.append("")

    out.append("### 2.2 Internal Shift Changes Inside the Three `code_04` Cutscene Script Tables")
    out.append("")
    out.append("Inside the three `code_04` cutscene script `incrom` regions (`0x0805AFC6`–`0x0805CF5C`, `0x08060264`–`0x080612C8`, and `0x0806544C`–`0x0806765C`), version-specific script commands (`3e 08 xx 16 18 2a c0 10 xx 16 xx 3c 08 xx`, 14 bytes) are inserted in one version and omitted in the other, followed by 2-byte `00 00` alignment padding changes at the end of each scene's script before its 32-bit word-aligned pointer table:")
    out.append("")
    out.append("| Table Block | RS Address Range | BM Address Range | Old Delta | New Delta | Change | Contents |")
    out.append("|---|---|---|---:|---:|---:|---|")
    table_shifts = [
        ("Table 1 (`0x5AFC6`)", "0x0805C19C..0x0805C1AA", "0x0805C1A8..0x0805C1A8", "+0xC", "-0x2", "-14 B (BM)", "RS-only 14-byte script check: `3e 08 10 16 18 2a c0 10 1e 16 01 3c 08 11`"),
        ("Table 1 (`0x5AFC6`)", "0x0805C266..0x0805C26A", "0x0805C264..0x0805C266", "-0x2", "-0x4", "-2 B (BM)", "RS has 2 extra `00 00` alignment bytes before pointer table at RS `0x0805C26C` / BM `0x0805C268`"),
        ("Table 1 (`0x5AFC6`)", "0x0805C80C..0x0805C80C", "0x0805C808..0x0805C816", "-0x4", "+0xA", "+14 B (BM)", "BM-only 14-byte script check: `3e 08 10 16 18 2a c0 10 1e 16 01 3c 08 11`"),
        ("Table 1 (`0x5AFC6`)", "0x0805C8B2..0x0805C8B6", "0x0805C8BC..0x0805C8BE", "+0xA", "+0x8", "-2 B (BM)", "RS has 2 extra `00 00` alignment bytes before pointer table at RS `0x0805C8B8` / BM `0x0805C8C0`"),
        ("Table 1 (`0x5AFC6`)", "0x0805CE02..0x0805CE02", "0x0805CE0A..0x0805CE18", "+0x8", "+0x16", "+14 B (BM)", "BM-only 14-byte script check: `3e 08 10 16 18 2a c0 10 1e 16 01 3c 08 11`"),
        ("Table 1 (`0x5AFC6`)", "0x0805CE50..0x0805CE52", "0x0805CE66..0x0805CE6A", "+0x16", "+0x18", "+2 B (BM)", "BM has 2 extra `00 00` alignment bytes before pointer table at RS `0x0805CE54` / BM `0x0805CE6C`"),
        ("Table 2 (`0x60264`)", "0x080609EE..0x080609FC", "0x08060A06..0x08060A06", "+0x18", "+0xA", "-14 B (BM)", "RS-only 14-byte script check: `3e 08 fe 16 18 2a c0 10 3c 16 07 3c 08 ff`"),
        ("Table 2 (`0x60264`)", "0x08060AFE..0x08060B02", "0x08060B08..0x08060B0A", "+0xA", "+0x8", "-2 B (BM)", "RS has 2 extra `00 00` alignment bytes before pointer table at RS `0x08060B04` / BM `0x08060B0C`"),
        ("Table 2 (`0x60264`)", "0x08060B32..0x08060B40", "0x08060B3A..0x08060B3A", "+0x8", "-0x6", "-14 B (BM)", "RS-only 14-byte script check: `3e 08 03 16 18 2a c0 10 3c 16 07 3c 08 04`"),
        ("Table 2 (`0x60264`)", "0x08060BA2..0x08060BA6", "0x08060B9C..0x08060B9E", "-0x6", "-0x8", "-2 B (BM)", "RS has 2 extra `00 00` alignment bytes before pointer table at RS `0x08060BA8` / BM `0x08060BA0`"),
        ("Table 2 (`0x60264`)", "0x08060D00..0x08060D00", "0x08060CF8..0x08060D06", "-0x8", "+0x6", "+14 B (BM)", "BM-only 14-byte script check: `3e 08 fe 16 18 2a c0 10 3c 16 07 3c 08 ff`"),
        ("Table 2 (`0x60264`)", "0x08060D98..0x08060D9A", "0x08060D9E..0x08060DA2", "+0x6", "+0x8", "+2 B (BM)", "BM has 2 extra `00 00` alignment bytes before pointer table at RS `0x08060D9C` / BM `0x08060DA4`"),
        ("Table 2 (`0x60264`)", "0x08061078..0x08061078", "0x08061080..0x0806108E", "+0x8", "+0x16", "+14 B (BM)", "BM-only 14-byte script check: `3e 08 10 16 18 2a c0 10 1e 16 07 3c 08 11`"),
        ("Table 2 (`0x60264`)", "0x080610CA..0x080610CE", "0x080610E0..0x080610E2", "+0x16", "+0x14", "-2 B (BM)", "RS has 2 extra `00 00` alignment bytes before pointer table at RS `0x080610D0` / BM `0x080610E4`"),
        ("Table 3 (`0x6544C`)", "0x080656DC..0x080656EC", "0x080656F0..0x080656F2", "+0x14", "+0x6", "-14 B (BM)", "RS-only 14-byte script check: `3e 08 20 16 18 2a c0 10 1e 16 04 3c 08 21`"),
        ("Table 3 (`0x6544C`)", "0x080657C4..0x080657C6", "0x080657CA..0x080657CE", "+0x6", "+0x8", "+2 B (BM)", "BM has 2 extra `00 00` alignment bytes before pointer table at RS `0x080657C8` / BM `0x080657D0`"),
        ("Table 3 (`0x6544C`)", "0x08066366..0x08066378", "0x0806636E..0x08066372", "+0x8", "-0x6", "-14 B (BM)", "RS-only 14-byte script check: `3e 08 10 16 18 2a c0 10 1e 16 04 3c 08 11`"),
        ("Table 3 (`0x6544C`)", "0x080663E0..0x080663E2", "0x080663DA..0x080663DE", "-0x6", "-0x4", "+2 B (BM)", "BM has 2 extra `00 00` alignment bytes before pointer table at RS `0x080663E4` / BM `0x080663E0`"),
        ("Table 3 (`0x6544C`)", "0x08066520..0x08066524", "0x0806651C..0x08066530", "-0x4", "+0xC", "+16 B (BM)", "RS `16 07` replaced in BM by 18-byte block `04 1d 00 2a 65 06 08 16 18 2a c0 10 1e 16 07 3c 08 11`"),
    ]
    for r in table_shifts:
        out.append(f"| {r[0]} | `{r[1]}` | `{r[2]}` | `{r[3]}` | `{r[4]}` | `{r[5]}` | {r[6]} |")
    out.append("")

    out.append("### 2.3 Disassembler Boundary Artifacts (No Actual Byte Shift)")
    out.append("")
    out.append("When `tools/merge_bm.py` diffs RS and BM assembly, four spots produce version blocks purely because `functions_rs.txt` and `functions_bm.txt` have slightly different label/function boundaries over identical or constant-delta bytes:")
    out.append("- **`code_01` (`0x08011888`–`0x08012470`, delta `+0x0`):** RS splits `BN4_ENEMY_STATS` into two `incrom` blocks (`incrom 0x11888, 0xEC` + `dat_08011974: incrom 0x11974, 0xAFC`) because `0x0800631C` references `dat_08011974` in RS (and `dat_080204C0` in BM), whereas BM emits a single `incrom 0x11888, 0xBE8`.")
    out.append("- **`code_05` (`0x08079908`–`0x0807B120`, delta `+0xC`):** BM splits `dat_08079914` (`0x1818` bytes) at `dat_080799F2` (`0xDE` + `0x173A`) because `0x08006324` references `dat_080799F2` in BM (and `dat_080C0F9C` in RS).")
    out.append("- **`code_05` (`0x08089F10`–`0x08089F9C`, delta `+0xC`):** BM has a false-positive 6-byte function entry `sub_08089F8C` inside a pointer/data block that RS keeps as `incrom 0x89F10, 0x8C` (the halfword at RS `0x08089F80` is `0xA947`, while the shifted pointer at BM `0x08089F8C` happens to be `0xB547` = `push {r0, r1, r2, r6, lr}`).")
    out.append("- **`code_10` (`0x08113C74`–`0x08113CB6`, delta `+0x14`):** `functions_rs.txt` has 3 mid-function entry labels (`sub_08113C88`, `sub_08113C94`, `sub_08113CB4`) that `functions_bm.txt` lacks, and BM `sub_08113CB4` stops 2 bytes earlier (`bx lr` is emitted as `.word 0x00004770` before `data_from = 0x08113CCC`).")
    out.append("")

    # Section 3: Recommended Segment Split
    out.append("## 3. Recommended Segment Split")
    out.append("")
    out.append("Splitting `segments_rs.toml` and `segments_bm.toml` at the points below gives every code segment a **single, invariant `bm_delta` across all functions**, eliminating all shift-drift `.if VERSION_BM` blocks:")
    out.append("")
    out.append("1. **`code_04` (critical — removes ~260 lines of shift-drift version blocks across 41 functions):**")
    out.append("   Currently `code_04` (`0x0804F21C`–`0x080711E4`) uses a single `bm_delta = 0xC`, which is wrong for the 7 functions in `0x0805D4AC`–`0x08060264` (`bm_delta = 0x18`) and the 34 functions in `0x080612C8`–`0x0806544C` (`bm_delta = 0x14`). Splitting `code_04` into 4 sub-segments aligns every function and data label:")
    out.append("")
    out.append("   | Proposed Segment | RS Start | RS End | BM Start | BM End | `bm_delta` | Contents |")
    out.append("   |---|---|---|---|---|---:|---|")
    out.append("   | `code_04a` | `0x0804F21C` | `0x0805CF5C` | `0x0804F228` | `0x0805CF74` | `+0xC` | 209 functions (`sub_0804F21C`–`sub_0805AF8C`) + cutscene table 1 (`incrom 0x5AFC6`, net `+0xC`) |")
    out.append("   | `code_04b` | `0x0805CF5C` | `0x080612C8` | `0x0805CF74` | `0x080612DC` | `+0x18` | `dat_0805CF5C`–`dat_0805D48C`, 7 functions (`sub_0805D4AC`–`sub_08060248`), + cutscene table 2 (`incrom 0x60264`, net `-0x4`) |")
    out.append("   | `code_04c` | `0x080612C8` | `0x0806765C` | `0x080612DC` | `0x08067668` | `+0x14` | 34 functions (`sub_080612C8`–`sub_0806541A`) + cutscene table 3 (`incrom 0x6544C`, net `-0x8`) |")
    out.append("   | `code_04d` | `0x0806765C` | `0x080711E4` | `0x08067668` | `0x080711F0` | `+0xC` | `dat_0806765C` + 66 functions (`sub_08067B3C`–`sub_08070B84`) |")
    out.append("")
    out.append("   *(Note: If segment starts must sit on function starts rather than `dat_` labels, `code_04b` can start at `0x0805D4AC` / BM `0x0805D4C4` and `code_04d` can start at `0x08067B3C` / BM `0x08067B48`, keeping the preceding `dat_` tables at the tail of `code_04a` and `code_04c`.)*")
    out.append("")
    out.append("2. **`code_09` / `code_10` boundary (removes shift drift across the last 17 functions of `code_09`):**")
    out.append("   Currently `code_09` ends at RS `0x0810A1B4` (BM `0x0810A1C8`), but the shift from `+0xC` to `+0x14` happens inside `sub_08109E48` (`0x08109E48`–`0x08109ED8`), so the 17 functions in RS `0x08109EF0`–`0x0810A1B4` already have `bm_delta = 0x14`.")
    out.append("   - Move the `code_09` / `code_10` boundary back from RS `0x0810A1B4` (BM `0x0810A1C8`) to **RS `0x08109EF0` (BM `0x08109F04`)**, the first function after `sub_08109E48`'s literal pool.")
    out.append("")
    out.append("3. **`code_02` / `code_03` and `code_03` / `code_04` boundaries (already optimal at function level):**")
    out.append("   - In `code_02`, the `+0x4` → `+0x8` insertion is inside the last function `sub_08035020` (`0x08035020`–`0x08035088`), right before `code_03` starts at `0x08035088` (BM `0x08035090`).")
    out.append("   - In `code_03`, the `+0x8` → `+0xC` insertion is inside the last function `ScriptCmd_checkGameVersion` (`0x0804F204`–`0x0804F21C`), right before `code_04` starts at `0x0804F21C` (BM `0x0804F228`).")
    out.append("   - In `code_01`, `BN4_FACE_GET` (`0x08016A20`–`0x08016A6C`) inserts +2 bytes at `0x08016A68` and removes 2 bytes of padding at `0x08016A6A`, so its net delta change is `0`.")
    out.append("")

    # Section 4: Differing Sites by Kind
    out.append("## 4. Differing Sites by Kind")
    out.append("")
    out.append("### 4.1 Whole Functions or Code Blocks That Differ (Length-Changing Functions)")
    out.append("")
    out.append("Only **4 functions** in the entire code region have different instructions/sizes between Red Sun and Blue Moon (plus 1 false-positive function entry `sub_08089F8C` in BM `code_05` and 3 extra mid-function labels in RS `code_10` `0x08113C88`–`0x08113CB4`):")
    out.append("")
    out.append("1. **`BN4_FACE_GET` (`sub_08016A20`, `code_01`, RS `0x08016A20`–`0x08016A6C` [74B + 2B pad] vs BM `0x08016A20`–`0x08016A6C` [76B]):**")
    out.append("   - At RS `0x08016A64`, RS computes `adds r0, r4, #0; adds r0, #4` and pops (`pop {r4, pc}; .hword 0x0000`).")
    out.append("   - BM adds `subs r0, #6` (`06 38`) at `0x08016A68` before `pop {r4, pc}`, consuming the 2-byte alignment pad so the literal pool at `0x08016A6C` does not shift. The 3 internal branches at `0x08016A38`, `0x08016A50`, and `0x08016A62` increase their target displacement by +2 bytes (`_08016A68` → `_08016A6A`).")
    out.append("2. **`sub_08035020` (BM `sub_08035024`, `code_02`, RS `0x08035020`–`0x08035088` [104B] vs BM `0x08035024`–`0x08035090` [108B]):**")
    out.append("   - Checks a range of IDs (`r1 = r0 & 0xFF`): RS checks `cmp r1, #0x85; beq _08035044; cmp r1, #0x7f; bge _08035084`.")
    out.append("   - BM checks `cmp r1, #0x79; blt _0803504C; cmp r1, #0x85; beq _0803504C; cmp r1, #0x7e; ble _0803508C` (+4 bytes). The PC-relative load at `0x0803502C` (`.hword 0x4E6A` → `0x4E6B`) adjusts its displacement by +4 bytes.")
    out.append("3. **`ScriptCmd_checkGameVersion` (`sub_0804F204`, BM `sub_0804F20C`, `code_03`, RS `0x0804F204`–`0x0804F21C` [24B] vs BM `0x0804F20C`–`0x0804F228` [26B + 2B pad]):**")
    out.append("   - RS loads script argument byte `movs r0, #2; ldrb r1, [r4, r0]`.")
    out.append("   - BM leaves `movs r0, #2` in place and immediately overwrites `r0` with `movs r0, #3` (+2 bytes), followed by 2 bytes of `.hword 0x0000` padding at `0x0804F226` (+4 bytes total).")
    out.append("4. **`sub_08109E48` (BM `sub_08109E54`, `code_09`, RS `0x08109E48`–`0x08109ED8` [144B] vs BM `0x08109E54`–`0x08109EEC` [150B + 2B pad]):**")
    out.append("   - At RS `0x08109EB2` (BM `0x08109EBE`), BM inserts 3 instructions (`tst r1, r1; bne _08109EC4; adds r1, #1`, 6 bytes) plus 2 bytes of `.hword 0x0000` padding before literal pool `_08109EEC` (+8 bytes total). Eight internal branch and `ldr` instructions in `sub_08109E48` adjust their PC-relative displacements accordingly.")
    out.append("")

    out.append("### 4.2 Decoded Instruction Immediates & 16-Bit Constants (36 Sites + 12 Local Displacements)")
    out.append("")
    out.append("Every 16-bit decoded instruction or `.hword` difference (at constant local delta), with its enclosing function from `symbols/names.txt`:")
    out.append("")
    out.append("| # | Segment | RS Address | BM Address | Enclosing Function | RS Instruction / Value | BM Instruction / Value | Notes |")
    out.append("|---:|---|---|---|---|---|---|---|")
    for idx, (seg, r_a, b_a, rh, bh, r_op, b_op, fn) in enumerate(data["sites_imm"], 1):
        note = ""
        if "0x18" in r_op and "0x21" in b_op:
            note = "Version ID / chip or navi constant (`0x18` RS vs `0x21` BM)"
        elif any(k in r_op for k in ("#0x2c", "#0x64", "#0x38", "#0x43")):
            note = "Save-file validation XOR key (`0x2C`/`0x64`/`0x38`/`0x43` RS vs `0x71`/`0x47`/`0x8E`/`0x31` BM)"
        elif r_a == 0x08014B98:
            note = "Mugshot face step offset (`subs r2, #4` RS vs `adds r2, #2` BM)"
        elif "#0x5b" in r_op:
            note = "Version constant check (`0x5B` RS vs `0x16` BM)"
        elif "#0x16" in r_op and "#0x22" in b_op:
            note = "Save checksum seed offset (`0x16` RS vs `0x22` BM)"
        elif "#0x17" in r_op and "#0x1d" in b_op:
            note = "Double Soul starting flag (`0x17` RS vs `0x1D` BM)"
        elif r_a == 0x08104146:
            note = "Tournament plate special table header (`BN4_PLATE_SPECIAL`)"
        elif "#8" in r_op and "#4" in b_op:
            note = "Version bitflag (`8` RS vs `4` BM)"
        elif ("#0" in r_op and "#1" in b_op):
            note = "Version index (`0` = Red Sun, `1` = Blue Moon)"
        elif r_a in (0x08035040, 0x08035042):
            note = "Range boundary check in `sub_08035020`"
        out.append(f"| {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | {fn} | `{r_op}` (`0x{rh:04X}`) | `{b_op}` (`0x{bh:04X}`) | {note} |")
    out.append("")

    out.append("#### Intra-Function PC-Relative Displacements Caused by Local Insertions (12 Sites)")
    out.append("")
    out.append("| # | Segment | RS Address | BM Address | Enclosing Function | RS Instruction | BM Instruction | Cause |")
    out.append("|---:|---|---|---|---|---|---|---|")
    for idx, (seg, r_a, b_a, rh, bh, r_op, b_op, fn) in enumerate(data["sites_branch_local"], 1):
        cause = "+2B insn in `BN4_FACE_GET`" if seg == "code_01" else ("+4B insns in `sub_08035020`" if seg == "code_02" else ("Swapped literal pool order `_080F5EBC`/`_080F5ECC`" if r_a == 0x080F5EA8 else "+6B insns/+2B pad in `sub_08109E48`"))
        out.append(f"| {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | {fn} | `{r_op}` (`0x{rh:04X}`) | `{b_op}` (`0x{bh:04X}`) | {cause} |")
    out.append("")

    out.append("### 4.3 Decoded 32-Bit `.word` Constants (10 Sites)")
    out.append("")
    out.append("*(Note: In `code_09` at RS `0x080F5EBC`–`0x080F5EC4` inside `sub_080F5DDC`, the two literal pool words `.word 0x00002330` and `.word 0x00000335` are swapped in order between RS and BM, while `ldr r0` at `0x080F5EA8` adjusts its offset by +4 so both versions load `0x00002330`.)*")
    out.append("")
    out.append("| # | Segment | RS Address | BM Address | Enclosing Symbol / Context | RS `.word` | BM `.word` | Meaning |")
    out.append("|---:|---|---|---|---|---|---|---|")
    for idx, (seg, r_a, b_a, rw, bw, r_op, b_op, fn) in enumerate(data["sites_word_const"], 1):
        meaning = "Version index word (`0` RS vs `1` BM)" if rw == 0 and bw == 1 else (
            "Soul / Navi ID table (`dat_080401FC`)" if 0x080401FC <= r_a <= 0x08040204 else (
                "Tournament plate special IDs (`BN4_PLATE_SPECIAL`)" if r_a == 0x08104148 else "Soul / tournament table (`dat_0810DAB8`)"
            )
        )
        out.append(f"| {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | {fn} | `{r_op}` | `{b_op}` | {meaning} |")
    out.append("")

    out.append("### 4.4 Code-Region Pointers (`0x08000000`–`0x0811FFFF`)")
    out.append("")
    out.append("#### A. Same-Target Shifted Code References (51,959 Total)")
    out.append("- **17,487 decoded Thumb `bl` calls** and **15 unreached Thumb `bl` calls inside dead-code `incrom` blocks** (`code_06` `0x080A2C86`, `code_07` `0x080B5252`/`0x080B7652`/`0x080CF4C2`, and `code_08` `0x080D5352`/`0x080D5362`/`0x080D5374`/`0x080D7606`/`0x080DD85A`/`0x080E2978`/`0x080EFF30` plus 4 others) whose 22-bit PC-relative offset changes by `-4`, `-6`, `-8`, `-12`, or `-20` bytes solely because the caller shifted while the callee is in `code_00`/`code_01` (`delta = 0`), or vice versa.")
    out.append("- **3,832 decoded `.word` code pointers** (literal pools and jump/function tables).")
    out.append("- **30,625 `incrom` code pointers** (including unaligned 32-bit script/cutscene pointers in `code_04`, `code_05`, `code_06`, `code_09`, and `code_10`) where `BM_ptr == RS_ptr + true_delta(RS_ptr)`.")
    out.append("")
    out.append("#### B. Non-Trivial Code-Region Pointers (19 Sites)")
    out.append("These 19 code-region pointers do **not** follow the local segment delta: 2 are version-specific data table pointers in `code_00`, 1 is a 4-byte sequence in `code_01`'s `dat_08017F5C` table that happens to end in `0x08`, and 16 are script jump/table pointers in `code_04`–`code_06` whose targets lie inside the 3 cutscene script tables in `code_04` where local 14-byte script checks were inserted or deleted:")
    out.append("")
    out.append("| # | Segment | RS Address | BM Address | RS Target | BM Target | Target Delta | Explanation |")
    out.append("|---:|---|---|---|---|---|---:|---|")
    for idx, (seg, r_a, b_a, r_t, b_t, fn) in enumerate(data["sites_code_ptr_diff"], 1):
        m_r = re.search(r"0x([0-9a-fA-F]+)", r_t)
        m_b = re.search(r"0x([0-9a-fA-F]+)", b_t)
        td_s = f"{int(m_b.group(1), 16) - int(m_r.group(1), 16):+#x}" if (m_r and m_b) else "N/A"
        expl = (
            "Save/init table pointer (`dat_08011974` in RS vs `dat_080204C0` in BM)" if r_a == 0x0800631C else
            "Save/init table pointer (`dat_080C0F9C` in RS vs `dat_080799F2` in BM)" if r_a == 0x08006324 else
            "4-byte constant sequence in `dat_08017F5C` (`24 00 00 08` vs `16 00 00 08`, not a real code pointer)" if r_a == 0x08017F97 else
            "Cutscene script branch/table pointer into `code_04` script block (shifted by local +-0xE script check)"
        )
        out.append(f"| {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | `{r_t}` | `{b_t}` | `{td_s}` | {expl} |")
    out.append("")

    # Section 4.5: Data-region pointers
    out.append("### 4.5 Pointers into the Data Region (`0x08120000`–`0x087FFFFF`, 5,802 Sites)")
    out.append("")
    out.append("There are **644 decoded literal-pool `.word` pointers** and **5,158 `incrom` table pointers** (including bit-31-flagged `0x88xxxxxx` compressed asset pointers in `code_02`, `code_03`, `code_05`, and `code_06`) that point into the data region (`0x08120000`–`0x087FFFFF`) and differ between RS and BM.")
    out.append("")
    out.append("#### A. All 644 Decoded Literal-Pool Data-Region Pointers")
    out.append("")
    out.append("| # | Segment | RS Pool Addr | BM Pool Addr | Enclosing Function | RS Target | BM Target | Target Delta |")
    out.append("|---:|---|---|---|---|---|---|---:|")
    pool_data_ptrs = [x for x in data["sites_data_ptr"] if x[5] == "pool"]
    for idx, (seg, r_a, b_a, rw, bw, _, fn) in enumerate(pool_data_ptrs, 1):
        d_val = (bw & 0x7FFFFFFF) - (rw & 0x7FFFFFFF)
        out.append(f"| {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | {fn} | `0x{rw:08X}` | `0x{bw:08X}` | `{d_val:+#x}` |")
    out.append("")

    out.append(f"#### B. All 5,158 `incrom` Data-Region Pointers (Grouped into {len(data['data_ptr_incrom_clusters'])} Contiguous Table Spans)")
    out.append("")
    out.append("| # | Segment | RS Table Range | BM Table Range | Ptr Count | First RS→BM Pointer | Target Range (RS) | Target Delta(s) |")
    out.append("|---:|---|---|---|---:|---|---|---|")
    for idx, c in enumerate(data["data_ptr_incrom_clusters"], 1):
        seg, r1, r2, b1, b2, cnt, rw0, bw0, t_min, t_max, deltas = c
        d_list = sorted(deltas)
        if len(d_list) <= 4:
            d_str = ", ".join(f"`{d:+#x}`" for d in d_list)
        else:
            d_str = f"`{d_list[0]:+#x}`..`{d_list[-1]:+#x}` ({len(d_list)} distinct)"
        out.append(
            f"| {idx} | `{seg}` | `0x{r1:08X}`–`0x{r2:08X}` | `0x{b1:08X}`–`0x{b2:08X}` | {cnt} | "
            f"`0x{rw0:08X}` → `0x{bw0:08X}` | `0x{t_min:08X}`–`0x{t_max:08X}` | {d_str} |"
        )
    out.append("")

    # Section 4.6: Incrom constant byte clusters
    out.append(f"### 4.6 Constant / Table Differences Inside `incrom` Blocks (538 Bytes Across {len(data['incrom_clusters'])} Clusters)")
    out.append("")
    out.append("Every remaining non-pointer byte difference inside `header` and `incrom` data blocks:")
    out.append("")
    out.append("| # | Segment | RS Range | BM Range | Diff Bytes | Symbol / Context | RS Sample Bytes | BM Sample Bytes |")
    out.append("|---:|---|---|---|---:|---|---|---|")
    for idx, (seg, r1, r2, b1, b2, cnt, fn) in enumerate(data["incrom_clusters"], 1):
        r_hex = rs_rom[r1 - BASE : min(r2, r1 + 8) - BASE].hex(" ")
        b_hex = bm_rom[b1 - BASE : min(b2, b1 + 8) - BASE].hex(" ")
        if r2 - r1 > 8:
            r_hex += " ..."
            b_hex += " ..."
        out.append(f"| {idx} | `{seg}` | `0x{r1:08X}`–`0x{r2:08X}` | `0x{b1:08X}`–`0x{b2:08X}` | {cnt} | {fn} | `{r_hex}` | `{b_hex}` |")
    out.append("")
    return "\n".join(out) + "\n"


def main() -> None:
    data = analyze()
    if "--markdown" in sys.argv:
        sys.stdout.write(generate_markdown(data))
        return
    cfg_rs = data["cfg_rs"]
    counts = data["counts_by_seg"]
    print("Per-segment summary:")
    header_cols = [
        "bl_shifted",
        "bl_shifted_incrom",
        "code_ptr_shifted",
        "code_ptr_shifted_incrom",
        "local_disp_shifted",
        "imm_insn",
        "const_word",
        "const_byte_incrom",
        "code_ptr_diff",
        "data_ptr_pool",
        "data_ptr_incrom",
    ]
    print(f"{'Segment':10s} | " + " | ".join(f"{c:10s}" for c in header_cols))
    print("-" * 160)
    totals = Counter()
    for s in cfg_rs["segment"]:
        name = s["name"]
        c = counts[name]
        totals.update(c)
        print(f"{name:10s} | " + " | ".join(f"{c[col]:10d}" for col in header_cols))
    print("-" * 160)
    print(f"{'TOTAL':10s} | " + " | ".join(f"{totals[col]:10d}" for col in header_cols))
    print(f"\nDecoded immediate/opcode diffs: {len(data['sites_imm'])}")
    print(f"Decoded local branch/ldr displacement diffs: {len(data['sites_branch_local'])}")
    print(f"Decoded .word constant diffs: {len(data['sites_word_const'])}")
    print(f"Non-trivial code pointer diffs: {len(data['sites_code_ptr_diff'])}")
    print(f"Data-region pointer diffs: {len(data['sites_data_ptr'])} (pool: {totals['data_ptr_pool']}, incrom: {totals['data_ptr_incrom']})")
    print(f"Incrom constant byte diffs: {len(data['sites_incrom_const'])} ({len(data['incrom_clusters'])} clusters)")


if __name__ == "__main__":
    main()
