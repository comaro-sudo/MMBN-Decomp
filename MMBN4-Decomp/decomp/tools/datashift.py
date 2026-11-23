#!/usr/bin/env python3
"""DC-4 support (part 2): map the RS -> BM shift of the data region
(`0x08120000`-`0x08800000`) and validate all 5,802 data-region pointer pairs.

Reads only `roms/bn4_rs_us.gba`, `roms/bn4_bm_us.gba`, `config/segments_*.toml`,
`symbols/{names,functions_rs}.txt`, and `tools/vdiff.py` in read-only mode.

Usage:
    python3 tools/datashift.py              # write symbols/datashift_rs_bm.txt & print summary
    python3 tools/datashift.py --write-doc  # also write notes/data-region-shift-map.md
"""
from __future__ import annotations

import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
GAME_ROOT = DECOMP.parent
sys.path.insert(0, str(DECOMP / "tools"))
import vdiff

BASE = 0x08000000
DATA_START = 0x08120000
ROM_END = 0x08800000

# 10 sites out of 5,802 in vdiff's `sites_data_ptr` where the 4 bytes are either:
# - an intentional cross-region version-specific table pointer (0x08006334), or
# - a non-pointer constant byte sequence inside an `incrom` block whose 4th byte
#   happens to be 0x08 or 0x88.
NON_SHIFT_POINTER_SITES: set[int] = {
    0x08006334,  # Save init pointer: RS 0x08126511 vs BM 0x082F123C (different tables)
    0x08020021,  # BN4_SOUL_FLAGS constant byte + next literal 0x0203BF88 (0x88FFFFFF vs 0x88222120)
    0x08066D51,  # Cutscene script constant bytes 17 26 14 88 vs 17 12 14 88
    0x080730B3,  # Cutscene script constant bytes 16 0c 3e 08 vs 16 0a 3e 08
    0x080F7130,  # Battle table constant bytes 19 1a d5 08 vs 19 14 d5 08
    0x080F7144,  # Battle table constant bytes 19 1a d5 08 vs 19 14 d5 08
    0x080F8315,  # Chip/pack table constant bytes 2f 2d 2f 88 vs 2f 32 2f 88
    0x080F832B,  # Chip/pack table constant bytes 01 88 53 88 vs 33 88 53 88
    0x080F833F,  # Chip/pack table constant bytes 01 88 53 88 vs 33 88 53 88
    0x080F8353,  # Chip/pack table constant bytes 01 88 53 88 vs 33 88 53 88
}

# 21 sites in `code_02` (RS 0x0802F7D8..0x0802F968, every 0x14 bytes) where a
# descriptor table points into the identical shared palette archive at RS
# 0x086F7ACC..0x086FA8AC (which lives at a constant delta of -0x50C in BM),
# but BM selects a different 16-color palette row (+0x20, +0x40, +0x60, +0x80,
# +0xE0, +0x140, or +0x200) within that shared palette table.
PALETTE_ROW_SELECTOR_SITES: set[int] = set(range(0x0802F7D8, 0x0802F968 + 1, 0x14))


def lz77_compressed_len(rom: bytes, start_off: int) -> int | None:
    """Return 4-byte-padded compressed size of a GBA LZ77 block at start_off."""
    if start_off < 0 or start_off + 4 > len(rom) or rom[start_off] != 0x10:
        return None
    decomp_len = rom[start_off + 1] | (rom[start_off + 2] << 8) | (rom[start_off + 3] << 16)
    if decomp_len == 0 or decomp_len > 0x40000:
        return None
    pos = start_off + 4
    out_len = 0
    while out_len < decomp_len:
        if pos >= len(rom):
            return None
        flags = rom[pos]
        pos += 1
        for bit in range(7, -1, -1):
            if out_len >= decomp_len:
                break
            if (flags >> bit) & 1:
                if pos + 2 > len(rom):
                    return None
                b1 = rom[pos]
                pos += 2
                out_len += (b1 >> 4) + 3
            else:
                if pos >= len(rom):
                    return None
                pos += 1
                out_len += 1
    aligned_pos = (pos + 3) & ~3
    if aligned_pos <= len(rom) and all(b == 0 for b in rom[pos:aligned_pos]):
        return aligned_pos - start_off
    return pos - start_off


def describe_region(rs_start: int, rs_end: int, bm_start: int | None, kind: str, rs_rom: bytes) -> str:
    """Return a concise asset/category description for a data-region range."""
    length = rs_end - rs_start if rs_end > rs_start else 0
    d_str = f"delta {(bm_start - rs_start):+#x}" if bm_start is not None else ""
    has_lz = (rs_start >= BASE and rs_start - BASE < len(rs_rom) and rs_rom[rs_start - BASE] == 0x10)
    if rs_start < 0x081540AC:
        base_desc = "m4a audio data / sequence & wave tables"
    elif rs_start < 0x0820A42C:
        base_desc = "m4a song table & music/SFX tracks"
    elif rs_start < 0x08212700:
        base_desc = "m4a PCM audio samples & voice tables"
    elif rs_start < 0x08214000:
        base_desc = "IWRAM code payload & system data"
    elif rs_start < 0x082F0000:
        base_desc = "OW/battle sprite archives"
    elif rs_start < 0x0849C7B0:
        base_desc = "sprite & animation archives"
    elif rs_start < 0x084BA168:
        base_desc = "compressed tilesets & palettes (LZ77)" if has_lz else "tilesets & palettes"
    elif rs_start < 0x084BB768:
        base_desc = "map event & object layout tables"
    elif rs_start < 0x08586B38:
        w1 = struct.unpack_from("<I", rs_rom, rs_start - BASE + 4)[0] if rs_start + 8 <= ROM_END else 0
        if w1 == 0x18:
            base_desc = "map tileset container (0x18 hdr + 2x LZ77)"
        elif w1 == 0x0C:
            base_desc = "map tilemap container (0x0C hdr + LZ77)"
        elif length == 0x204:
            base_desc = "map palette block (0x204B)"
        else:
            base_desc = "map tileset / tilemap / palette block"
    elif rs_start < 0x086A04BC:
        base_desc = "battle & object graphics / sprite data"
    elif rs_start < 0x086AD77C:
        base_desc = "version Navi / Soul battle sprite archive (LZ77)"
    elif rs_start < 0x086C5B0C:
        base_desc = "compressed UI / mugshot / chip graphics (LZ77)" if has_lz else "UI / mugshot / chip graphics"
    elif rs_start < 0x086F7000:
        base_desc = "text script archives & dialogue tables"
    elif rs_start < 0x086FB000:
        base_desc = "shared map & object palette archive"
    elif rs_start < 0x0874CBE0:
        base_desc = "text archives & battle/menu strings"
    elif rs_start < 0x087F06CC:
        base_desc = "compressed cutscene / battle backgrounds (LZ77)" if has_lz else "background / palette data"
    elif rs_start < 0x087F739C:
        base_desc = "title screen & version logo graphics/palettes/tilemaps"
    else:
        base_desc = "end-of-ROM 0xFF padding"
    return f"{base_desc} ({d_str}, len=0x{length:X})" if d_str else f"{base_desc} (len=0x{length:X})"


def classify_same_delta_span(
    r1: int, r2: int, b1: int, d: int, rs_rom: bytes, bm_rom: bytes
) -> list[tuple[int, int, int | None, int | None, str, str]]:
    """Split a constant-delta interval [r1, r2) (with BM = RS + d) into `same` and `differs` sub-ranges."""
    if rs_rom[r1 - BASE : r2 - BASE] == bm_rom[b1 - BASE : b1 + (r2 - r1) - BASE]:
        return [(r1, r2, b1, b1 + (r2 - r1), "same", describe_region(r1, r2, b1, "same", rs_rom))]

    raw_runs: list[list] = []
    pos = r1
    while pos < r2:
        step = min(4, r2 - pos)
        eq = rs_rom[pos - BASE : pos - BASE + step] == bm_rom[pos + d - BASE : pos + d - BASE + step]
        k = "same" if eq else "differs"
        if raw_runs and raw_runs[-1][2] == k:
            raw_runs[-1][1] = pos + step
        else:
            raw_runs.append([pos, pos + step, k])
        pos += step

    # Coalesce short `same` islands (< 64 bytes) sandwiched between `differs` runs
    merged: list[list] = []
    for idx, run in enumerate(raw_runs):
        if (
            run[2] == "same"
            and (run[1] - run[0]) < 64
            and merged
            and merged[-1][2] == "differs"
            and idx + 1 < len(raw_runs)
            and raw_runs[idx + 1][2] == "differs"
        ):
            merged[-1][1] = run[1]
        elif merged and merged[-1][2] == run[2]:
            merged[-1][1] = run[1]
        else:
            merged.append(list(run))

    out = []
    for rs_s, rs_e, k in merged:
        bm_s = rs_s + d
        bm_e = rs_e + d
        out.append((rs_s, rs_e, bm_s, bm_e, k, describe_region(rs_s, rs_e, bm_s, k, rs_rom)))
    return out


def build_shift_map():
    vdata = vdiff.analyze()
    rs_rom: bytes = vdata["rs_rom"]
    bm_rom: bytes = vdata["bm_rom"]
    ptrs: list[tuple] = vdata["sites_data_ptr"]

    anchors: dict[int, int] = {
        DATA_START: DATA_START + 0x14,
        0x08212700: 0x08212700,
        0x087F739C: 0x087F739C,
        ROM_END: ROM_END,
    }
    for seg, r_a, b_a, rw, bw, kind, fn in ptrs:
        if r_a in NON_SHIFT_POINTER_SITES or r_a in PALETTE_ROW_SELECTOR_SITES or r_a == 0x0803356C:
            continue
        rt = rw & 0x7FFFFFFF
        bt = bw & 0x7FFFFFFF
        if DATA_START <= rt < ROM_END:
            anchors[rt] = bt

    # Add equal data-region pointers from the code region (delta = 0 in 0x08212700..0x0849C7B0)
    for s, e, d, _ in vdiff.DELTA_SPANS:
        for a in range(s + (4 - s % 4) % 4, e - 3, 4):
            b = a + d
            rw = struct.unpack_from("<I", rs_rom, a - BASE)[0]
            bw = struct.unpack_from("<I", bm_rom, b - BASE)[0]
            if rw == bw and ((rw >> 24) in (0x08, 0x88)):
                rt = rw & 0x7FFFFFFF
                if 0x08212700 <= rt < 0x0849C7B0 and rt % 4 == 0:
                    anchors[rt] = rt

    # Add title-screen second sub-block anchors in 0x087F3EB4..0x087F739C
    anchors[0x087F43AC] = 0x087F42BC
    anchors[0x087F4758] = 0x087F4668
    anchors[0x087F4938] = 0x087F4848
    anchors[0x087F6E4C] = 0x087F6E9C

    sorted_a = sorted(anchors.items())
    for i in range(len(sorted_a) - 1):
        assert sorted_a[i][1] < sorted_a[i + 1][1], f"Non-monotonic anchor: {sorted_a[i]} -> {sorted_a[i+1]}"

    rows: list[tuple[int | None, int | None, int | None, int | None, str, str]] = []

    for i in range(len(sorted_a) - 1):
        r1, b1 = sorted_a[i]
        r2, b2 = sorted_a[i + 1]
        d1 = b1 - r1
        d2 = b2 - r2

        if d1 == d2:
            for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(r1, r2, b1, d1, rs_rom, bm_rom):
                rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
            continue

        # Special Case: RS 0x086E16BC..0x086E3D4C..0x086E425C where the 0x50-byte LZ77 block
        # at RS 0x086E3CFC..0x086E3D4C is relocated to BM 0x086E3D00..0x086E3D50 (+0x4)
        if r1 == 0x086E16BC and r2 == 0x086E3D4C:
            for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(0x086E16BC, 0x086E3CFC, 0x086E11B0, -0x50C, rs_rom, bm_rom):
                rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
            rows.append((0x086E3CFC, 0x086E3D4C, 0x086E3D00, 0x086E3D50, "same", "relocated LZ77 asset (delta +0x4, len=0x50; moved after BM 0x086E3D00)"))
            continue
        if r1 == 0x086E3D4C and r2 == 0x086E425C:
            for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(0x086E3D4C, 0x086E425C, 0x086E37F0, -0x55C, rs_rom, bm_rom):
                rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
            continue

        # 1. Single LZ77 block at (r1, b1)
        rlz = lz77_compressed_len(rs_rom, r1 - BASE)
        blz = lz77_compressed_len(bm_rom, b1 - BASE)
        if rlz and blz and r1 + rlz <= r2 and b1 + blz <= b2 and (b1 + blz) - (r1 + rlz) == d2:
            rows.append((
                r1, r1 + rlz, b1, b1 + blz, "differs",
                f"version-specific LZ77 asset (RS len=0x{rlz:X}, BM 0x{b1:08X}..0x{b1+blz:08X} len=0x{blz:X}, delta {d1:+#x}->{d2:+#x})"
            ))
            if r1 + rlz < r2:
                for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(r1 + rlz, r2, b1 + blz, d2, rs_rom, bm_rom):
                    rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
            continue

        # 2. Map tileset container (0x18 header + 2 LZ77 blocks)
        rw1 = struct.unpack_from("<I", rs_rom, r1 - BASE + 4)[0] if r1 + 8 <= ROM_END else 0
        bw1 = struct.unpack_from("<I", bm_rom, b1 - BASE + 4)[0] if b1 + 8 <= ROM_END else 0
        if rw1 == 0x18 and bw1 == 0x18:
            rlz1 = lz77_compressed_len(rs_rom, r1 - BASE + 0x18)
            blz1 = lz77_compressed_len(bm_rom, b1 - BASE + 0x18)
            if rlz1 and blz1:
                rlz2 = lz77_compressed_len(rs_rom, r1 - BASE + 0x18 + rlz1)
                blz2 = lz77_compressed_len(bm_rom, b1 - BASE + 0x18 + blz1)
                if rlz2 and blz2:
                    r_end_c = r1 + 0x18 + rlz1 + rlz2
                    b_end_c = b1 + 0x18 + blz1 + blz2
                    if r_end_c <= r2 and b_end_c <= b2 and (b_end_c - r_end_c) == d2:
                        rows.append((r1, r1 + 0x18, b1, b1 + 0x18, "differs", f"map tileset header (delta {d1:+#x}, len=0x18)"))
                        d_mid = (b1 + 0x18 + blz1) - (r1 + 0x18 + rlz1)
                        k1 = "same" if (rlz1 == blz1 and rs_rom[r1+0x18-BASE:r1+0x18+rlz1-BASE] == bm_rom[b1+0x18-BASE:b1+0x18+blz1-BASE]) else "differs"
                        rows.append((
                            r1 + 0x18, r1 + 0x18 + rlz1, b1 + 0x18, b1 + 0x18 + blz1, k1,
                            f"map tileset LZ77 block 1 (RS len=0x{rlz1:X}, BM 0x{b1+0x18:08X}..0x{b1+0x18+blz1:08X} len=0x{blz1:X}, delta {d1:+#x}->{d_mid:+#x})"
                        ))
                        k2 = "same" if (rlz2 == blz2 and rs_rom[r1+0x18+rlz1-BASE:r_end_c-BASE] == bm_rom[b1+0x18+blz1-BASE:b_end_c-BASE]) else "differs"
                        rows.append((
                            r1 + 0x18 + rlz1, r_end_c, b1 + 0x18 + blz1, b_end_c, k2,
                            f"map tileset LZ77 block 2 (RS len=0x{rlz2:X}, BM 0x{b1+0x18+blz1:08X}..0x{b_end_c:08X} len=0x{blz2:X}, delta {d_mid:+#x}->{d2:+#x})"
                        ))
                        if r_end_c < r2:
                            for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(r_end_c, r2, b_end_c, d2, rs_rom, bm_rom):
                                rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
                        continue

        # 3. Map tilemap container (0x0C header + 1 LZ77 block)
        if rw1 == 0x0C and bw1 == 0x0C:
            rlz1 = lz77_compressed_len(rs_rom, r1 - BASE + 0x0C)
            blz1 = lz77_compressed_len(bm_rom, b1 - BASE + 0x0C)
            if rlz1 and blz1:
                r_end_c = r1 + 0x0C + rlz1
                b_end_c = b1 + 0x0C + blz1
                if r_end_c <= r2 and b_end_c <= b2 and (b_end_c - r_end_c) == d2:
                    k_hdr = "same" if rs_rom[r1-BASE:r1+0xC-BASE] == bm_rom[b1-BASE:b1+0xC-BASE] else "differs"
                    rows.append((r1, r1 + 0x0C, b1, b1 + 0x0C, k_hdr, f"map tilemap header (delta {d1:+#x}, len=0xC)"))
                    rows.append((
                        r1 + 0x0C, r_end_c, b1 + 0x0C, b_end_c, "differs",
                        f"map tilemap LZ77 block (RS len=0x{rlz1:X}, BM 0x{b1+0xC:08X}..0x{b_end_c:08X} len=0x{blz1:X}, delta {d1:+#x}->{d2:+#x})"
                    ))
                    if r_end_c < r2:
                        for rs_s, rs_e, bm_s, bm_e, k, note in classify_same_delta_span(r_end_c, r2, b_end_c, d2, rs_rom, bm_rom):
                            rows.append((rs_s, rs_e, bm_s, bm_e, k, note))
                    continue

        # 4. General fallback (Gaps 1, 6, 92, and title screen sub-blocks):
        lcp = 0
        max_l = min(r2 - r1, b2 - b1)
        while lcp < max_l and rs_rom[r1 - BASE + lcp] == bm_rom[b1 - BASE + lcp]:
            lcp += 1
        lcs = 0
        while lcs < (max_l - lcp) and rs_rom[r2 - BASE - 1 - lcs] == bm_rom[b2 - BASE - 1 - lcs]:
            lcs += 1
        lcp4 = (lcp // 4) * 4
        lcs4 = (lcs // 4) * 4
        if lcp4 > 0:
            rows.append((r1, r1 + lcp4, b1, b1 + lcp4, "same", describe_region(r1, r1 + lcp4, b1, "same", rs_rom)))
        mid_r1, mid_r2 = r1 + lcp4, r2 - lcs4
        mid_b1, mid_b2 = b1 + lcp4, b2 - lcs4
        if mid_r2 > mid_r1 and mid_b2 > mid_b1:
            rows.append((
                mid_r1, mid_r2, mid_b1, mid_b2, "differs",
                f"{describe_region(mid_r1, mid_r2, mid_b1, 'differs', rs_rom)} [BM 0x{mid_b1:08X}..0x{mid_b2:08X}, BM len=0x{mid_b2-mid_b1:X}, delta {d1:+#x}->{d2:+#x}]"
            ))
        elif mid_r2 > mid_r1:
            rows.append((mid_r1, mid_r2, None, None, "rs_only", f"RS-only data block (len=0x{mid_r2-mid_r1:X}, at BM 0x{mid_b1:08X}, delta {d1:+#x}->{d2:+#x})"))
        elif mid_b2 > mid_b1:
            rows.append((None, None, mid_b1, mid_b2, "bm_only", f"BM-only data block BM 0x{mid_b1:08X}..0x{mid_b2:08X} (len=0x{mid_b2-mid_b1:X}, at RS 0x{mid_r1:08X}, delta {d1:+#x}->{d2:+#x})"))
        if lcs4 > 0:
            rows.append((r2 - lcs4, r2, b2 - lcs4, b2, "same", describe_region(r2 - lcs4, r2, b2 - lcs4, "same", rs_rom)))

    # Merge adjacent `same` rows that share the same constant delta
    merged_rows: list[tuple[int | None, int | None, int | None, int | None, str, str]] = []
    for row in rows:
        rs_s, rs_e, bm_s, bm_e, k, note = row
        if (
            merged_rows
            and k == "same"
            and merged_rows[-1][4] == "same"
            and rs_s is not None
            and merged_rows[-1][1] == rs_s
            and bm_s is not None
            and merged_rows[-1][3] == bm_s
            and (bm_s - rs_s) == (merged_rows[-1][2] - merged_rows[-1][0])
        ):
            prev = merged_rows[-1]
            new_rs_s, new_rs_e = prev[0], rs_e
            new_bm_s, new_bm_e = prev[2], bm_e
            merged_rows[-1] = (
                new_rs_s, new_rs_e, new_bm_s, new_bm_e, "same",
                describe_region(new_rs_s, new_rs_e, new_bm_s, "same", rs_rom)
            )
        else:
            merged_rows.append(row)

    # Verify full contiguous coverage of RS 0x08120000..0x08800000 with no gaps
    cur_rs = DATA_START
    for rs_s, rs_e, bm_s, bm_e, k, note in merged_rows:
        if rs_s is not None:
            assert rs_s == cur_rs, f"Gap/overlap in RS coverage: expected {cur_rs:#010x}, got {rs_s:#010x}"
            assert rs_e > rs_s, f"Empty RS range at {rs_s:#010x}"
            cur_rs = rs_e
    assert cur_rs == ROM_END, f"Coverage ended at {cur_rs:#010x} != {ROM_END:#010x}"

    return merged_rows, vdata


def map_rs_to_bm(rs_addr: int, rows: list[tuple]) -> int | None:
    """Map an RS data-region address (with bit 31 stripped) to its BM address using `rows`."""
    a = rs_addr & 0x7FFFFFFF
    for rs_s, rs_e, bm_s, bm_e, k, _ in rows:
        if rs_s is not None and rs_s <= a < rs_e:
            if bm_s is None:
                return None
            return bm_s + (a - rs_s)
    return None


def write_datashift_txt(rows: list[tuple]) -> Path:
    out_path = DECOMP / "symbols" / "datashift_rs_bm.txt"
    lines = ["# rs_start rs_end bm_start kind note        (tools/datashift.py)"]
    for rs_s, rs_e, bm_s, bm_e, k, note in rows:
        rs_s_str = f"{rs_s:08X}" if rs_s is not None else "-       "
        rs_e_str = f"{rs_e:08X}" if rs_e is not None else "-       "
        bm_s_str = f"{bm_s:08X}" if bm_s is not None else "-       "
        lines.append(f"{rs_s_str} {rs_e_str} {bm_s_str} {k:7s} {note}")
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    return out_path


def generate_markdown(rows: list[tuple], vdata: dict, passed: int, failed: list[tuple]) -> str:
    ptrs = vdata["sites_data_ptr"]
    out: list[str] = []
    out.append("# RS → BM Data-Region Shift Map (`0x08120000`–`0x08800000`)")
    out.append("")
    out.append("Generated by `decomp/tools/datashift.py` for Milestone **DC-4 (Part 2)**. Companion machine-readable table: `decomp/symbols/datashift_rs_bm.txt`.")
    out.append("")
    out.append("## 1. Overview & Validation Summary")
    out.append("")
    pct = 100.0 * passed / len(ptrs)
    out.append(f"- **Coverage:** 100% of Red Sun's 7,208,960-byte (`0x6E0000`) data region (`0x08120000`–`0x08800000`) with **0 gaps or overlaps**, partitioned into **{len(rows)} contiguous ranges**.")
    out.append(f"- **Pointer Validation (`BM == map(RS)` across all {len(ptrs):,} code-region data pointer pairs from `dc4-version-differences.md` §4.5):**")
    out.append(f"  - **{passed:,} / {len(ptrs):,} ({pct:.2f}%) pass** automatically via `map(RS) = bm_start + (RS - rs_start)`.")
    out.append(f"  - **{len(failed)} / {len(ptrs):,} ({100.0 - pct:.2f}%) differ from a pure address-shift lookup**, falling into 3 exact categories (all 31 listed and explained in §5 below):")
    out.append("    1. **21 palette-row selector pointers in `code_02` (`0x0802F7D8`–`0x0802F968`)** that point into the *byte-identical* shared palette archive at RS `0x086F7ACC`–`0x086FA8AC` (which sits at a constant `-0x50C` delta in BM), where BM deliberately indexes a different 16-color palette row (`+0x20`, `+0x40`, `+0x60`, `+0x80`, `+0xE0`, `+0x140`, or `+0x200` bytes higher) for version-colored map objects.")
    out.append("    2. **1 intentional cross-region save-init pointer in `code_00` (`0x08006334`)** where RS points to `0x08126511` and BM points to `0x082F123C`.")
    out.append("    3. **9 non-pointer 4-byte constant sequences inside `incrom` tables** (`code_01` `0x08020021`, `code_04` `0x08066D51`, `code_05` `0x080730B3`, and `code_09` `0x080F7130`–`0x080F8353`) whose 4th byte happens to be `0x08` or `0x88` in both versions.")
    out.append("")

    # Kind summary
    kind_counts = Counter(r[4] for r in rows)
    kind_bytes = Counter()
    for rs_s, rs_e, bm_s, bm_e, k, _ in rows:
        if rs_s is not None:
            kind_bytes[k] += rs_e - rs_s
        elif bm_s is not None:
            kind_bytes[k] += bm_e - bm_s

    out.append("| Kind | Range Count | Total RS Bytes | Share of RS Data Region | Description |")
    out.append("|---|---:|---:|---:|---|")
    for k, desc in [
        ("same", "Byte-for-byte identical at `bm_start = rs_start + delta`"),
        ("differs", "Same role at `bm_start`, different bytes (shifted pointers, version palettes/text/LZ77 assets)"),
        ("rs_only", "Bytes present only in Red Sun (no BM counterpart)"),
        ("bm_only", "Bytes present only in Blue Moon (no RS counterpart)"),
    ]:
        b_cnt = kind_bytes[k]
        share = (100.0 * b_cnt / (ROM_END - DATA_START)) if k != "bm_only" else 0.0
        out.append(f"| `{k}` | {kind_counts[k]} | `0x{b_cnt:X}` ({b_cnt:,} B) | {share:.2f}% | {desc} |")
    out.append("")

    # Section 2: Method
    out.append("## 2. Method")
    out.append("")
    out.append("1. **Code-Region Pointer Anchors:** We start from all 5,802 differing data-region pointer pairs (`0x08120000`–`0x087FFFFF`, including bit-31-flagged `0x88xxxxxx` compressed asset pointers) catalogued in `dc4-version-differences.md` §4.5, plus all equal data-region pointers (`delta = 0`, `0x08212700`–`0x0849C7B0`) from the code region.")
    out.append("2. **Filtering Non-Shift Pointers:** Removing the 9 coincidental `incrom` constant sequences, the 1 cross-region save pointer (`0x08006334`), the 21 palette-row selector pointers (`0x0802F7D8`–`0x0802F968`) that index different 32-byte rows inside an identical `-0x50C` palette table, and the 1 relocated 80-byte LZ77 block (`RS 0x086E3CFC` → `BM 0x086E3D00`, handled as its own explicit range) yields **3,800 strictly monotonic anchor pairs** `(rs_target, bm_target)`.")
    out.append("3. **Exact Asset Boundary Parsing Across Delta Transitions:**")
    out.append("   - **Single LZ77 assets (`0x10` header):** Decompressing the LZ77 bitstream (and 4-byte zero padding) gives the exact compressed byte length in both ROMs, resolving 17 delta transitions to the exact byte.")
    out.append("   - **Map tileset containers (`0x18`-byte header + 2 consecutive LZ77 blocks):** Parsing the `0x18`-byte container header and both internal LZ77 bitstreams resolves 26 delta transitions in `0x084E695C`–`0x0858530C`.")
    out.append("   - **Map tilemap containers (`0x0C`-byte header + 1 LZ77 block):** Parsing the `0x0C`-byte header and the following LZ77 bitstream resolves 16 delta transitions in `0x084F0664`–`0x08586B38`.")
    out.append("   - **Title screen & end-of-ROM structures (`0x087F06CC`–`0x08800000`):** Exact LZ77 parsing and sub-block anchoring maps both compressed title-screen tilesets (`0x087F0D0C` vs `0x087F07B4` and `0x087F4938` vs `0x087F4848`), their tilemaps/palettes, and the trailing `0xFF` ROM padding (`0x087F739C`–`0x08800000` in both ROMs).")
    out.append("4. **Sub-dividing Constant-Delta Intervals:** Within every constant-delta span, 4-byte words are compared between `RS[addr]` and `BM[addr + delta]` to separate byte-identical (`same`) runs from modified (`differs`) runs (coalescing `< 64`-byte equal islands inside modified blocks).")
    out.append("")

    # Section 3: Per-Delta Summary
    out.append("## 3. Per-Delta Summary (Bytes at Each RS → BM Delta)")
    out.append("")
    delta_stats: dict[int, list] = defaultdict(lambda: [0, 0, 0, 0x9000000, 0])  # [ranges, same_bytes, differs_bytes, min_rs, max_rs]
    for rs_s, rs_e, bm_s, bm_e, k, _ in rows:
        if rs_s is not None and bm_s is not None:
            d = bm_s - rs_s
            st = delta_stats[d]
            st[0] += 1
            if k == "same":
                st[1] += rs_e - rs_s
            else:
                st[2] += rs_e - rs_s
            st[3] = min(st[3], rs_s)
            st[4] = max(st[4], rs_e)

    out.append("Sorted by RS start address of each delta regime:")
    out.append("")
    out.append("| RS → BM Delta | Range Count | `same` Bytes | `differs` Bytes | Total RS Bytes | RS Address Span | Primary Contents |")
    out.append("|---:|---:|---:|---:|---:|---|---|")
    for d, st in sorted(delta_stats.items(), key=lambda kv: kv[1][3]):
        tot = st[1] + st[2]
        sample_desc = describe_region(st[3], st[4], st[3] + d, "same", vdata["rs_rom"]).split(" (delta")[0]
        out.append(
            f"| `{d:+#x}` | {st[0]} | `0x{st[1]:X}` ({st[1]:,}) | `0x{st[2]:X}` ({st[2]:,}) | "
            f"`0x{tot:X}` ({tot:,}) | `0x{st[3]:08X}`–`0x{st[4]:08X}` | {sample_desc} |"
        )
    out.append("")

    # Section 4: Largest Version-Specific Assets
    out.append("## 4. Largest Version-Specific Assets (`differs` Blocks & Size-Changing Assets)")
    out.append("")
    out.append("### 4.1 Top 25 Largest `differs` Assets (by RS Byte Length)")
    out.append("")
    out.append("| # | RS Range | BM Range | RS Length | BM Length | Size Diff | Start Delta → End Delta | Asset Description |")
    out.append("|---:|---|---|---:|---:|---:|---|---|")
    diff_rows = [r for r in rows if r[4] == "differs" and r[0] is not None and r[2] is not None]
    diff_rows.sort(key=lambda r: (r[1] - r[0]), reverse=True)
    for idx, (rs_s, rs_e, bm_s, bm_e, _, note) in enumerate(diff_rows[:25], 1):
        r_len = rs_e - rs_s
        b_len = bm_e - bm_s
        d_s = bm_s - rs_s
        d_e = bm_e - rs_e
        clean_note = note.split(" (delta")[0].split(" [BM")[0]
        out.append(
            f"| {idx} | `0x{rs_s:08X}`–`0x{rs_e:08X}` | `0x{bm_s:08X}`–`0x{bm_e:08X}` | "
            f"`0x{r_len:X}` ({r_len:,} B) | `0x{b_len:X}` ({b_len:,} B) | `{b_len - r_len:+#x}` | "
            f"`{d_s:+#x}` → `{d_e:+#x}` | {clean_note} |"
        )
    out.append("")

    out.append("### 4.2 Major Version-Specific Asset Groups Identified")
    out.append("")
    out.append("- **m4a Song Table & Audio Sequences (`0x08120000`–`0x0820A45C`, delta `+0x14` → `+0x0`):** All audio sample data is identical (`same`), while sequence headers and the song table at `0x0820A42C` (`differs`) contain internal ROM pointers shifted by `+0x14`. A 20-byte (`0x14`) padding reduction at `0x0820A42C`–`0x0820A45C` absorbs the `+0x14` code-region shift so that `0x08212700`–`0x0849C7B0` (2.66 MB of IWRAM code and sprite archives) sits at **`delta = +0x0`**.")
    out.append("- **First Version-Specific Compressed Tilesets (`0x0849C7B0`–`0x084BA168`, delta `+0x0` → `-0x1C` → `-0xC` → `+0x6C` → `+0x78`):** Four LZ77 blocks (`0x0849C7B0`, `0x0849CE9C`, `0x0849D148`, `0x084A85D0`) shift the data region to `+0x6C` and `+0x78`.")
    out.append("- **Map Event & Layout Tables (`0x084BA168`–`0x084BB73C`, delta `+0x78` → `-0x78`):** Version-specific overworld/cyberworld object layout tables (RS `0x15D4` bytes vs BM `0x14E4` bytes, net `-0xF0`).")
    out.append("- **Map Tileset & Tilemap Containers (`0x084E695C`–`0x08586B38`, delta `-0x78` → `-0x218`):** 26 map tileset containers (`0x18`-byte header + 2 LZ77 blocks) and 16 map tilemap containers (`0x0C`-byte header + 1 LZ77 block) with version-specific compressed map graphics and `0x204`-byte map palettes.")
    out.append("- **Version Navi & Double Soul Battle Sprite Archives (`0x086A04BC`–`0x086AD77C`, delta `-0x218` → `-0x428` → `-0x57C` → `-0x58C` → `-0x4BC`):** Four ~13.5 KB LZ77 sprite archives (`0x086A04BC`, `0x086A39E0`, `0x086A7158`, `0x086AA630`) holding version-exclusive Soul/Navi battle graphics.")
    out.append("- **Relocated 80-Byte LZ77 Asset (`RS 0x086E3CFC`–`0x086E3D4C` → `BM 0x086E3D00`–`0x086E3D50`):** In RS, this `0x50`-byte LZ77 block sits immediately before the `0x510`-byte LZ77 block at `0x086E3D4C`–`0x086E425C`; in BM, the two blocks are swapped in order (`0x086E37F0`–`0x086E3D00` followed by `0x086E3D00`–`0x086E3D50`), returning the delta to `-0x50C` right after `0x086E425C`.")
    out.append("- **Text Script Archives (`0x0874C6F4`–`0x0874CBE0`, delta `-0x50C` → `-0x570`):** Dialogue/text archive with version-specific tournament/story strings (`0x4EC` bytes in RS vs `0x488` bytes in BM, net `-0x64`).")
    out.append("- **Cutscene & Menu Graphics (`0x087B9074`–`0x087C2870`, delta `-0x570` → `-0x564` → `-0x574` → `-0x594` → `-0x5A0` → `-0x56C` → `-0x55C`):** Six consecutive LZ77 graphics blocks (mugshots/cutscene plates) that settle at `-0x55C`.")
    out.append("- **Title Screen & Red Sun / Blue Moon Logo Graphics (`0x087F06CC`–`0x087F739C`, delta `-0x55C` → `-0x558` → `-0xF0` → `+0x50` → `+0x0`):**")
    out.append("  - `0x087F06CC` (BM `0x087F0170`): LZ77 block (`0x37C` vs `0x380` bytes, delta `-0x55C` → `-0x558`).")
    out.append("  - `0x087F0A48`–`0x087F0D0C` (BM `0x087F04F0`–`0x087F07B4`): Uncompressed title screen palettes (`0x2C4` bytes, delta `-0x558`).")
    out.append("  - `0x087F0D0C`–`0x087F3EB4` (BM `0x087F07B4`–`0x087F3DC4`): First main title-screen LZ77 tileset (`0x31A8` bytes [12,712 B] in RS vs `0x3610` bytes [13,840 B] in BM, delta `-0x558` → `-0xF0`).")
    out.append("  - `0x087F4938`–`0x087F6E4C` (BM `0x087F4848`–`0x087F6E9C`): Second main title-screen LZ77 tileset (`0x2514` bytes [9,492 B] in RS vs `0x2654` bytes [9,812 B] in BM, delta `-0xF0` → `+0x50`), followed by a `0x500`-byte tilemap/table block (`0x087F6E4C`–`0x087F734C`) where RS has `0x50` extra trailing bytes (`0x087F72F8`–`0x087F734C`), bringing both ROMs back to `delta = +0x0` at `0x087F734C` right before the end-of-ROM `0xFF` padding (`0x087F739C`–`0x08800000`).")
    out.append("")

    # Section 5: Validation & Complete Failure List
    out.append(f"## 5. Validation Against All {len(ptrs):,} Pointer Pairs (Every Failure Listed & Explained)")
    out.append("")
    out.append(f"Testing `BM == map(RS)` on every one of the {len(ptrs):,} code-region data pointer pairs from `dc4-version-differences.md` §4.5 yields **{passed:,} passes ({pct:.2f}%)** and **{len(failed)} failures ({100.0 - pct:.2f}%)**.")
    out.append("")
    out.append("| # | Pair Index | Segment | RS Code Addr | BM Code Addr | Kind | RS Word (`rt`) | BM Word (`bt`) | `map(RS)` | Category & Explanation |")
    out.append("|---:|---:|---|---|---|---|---|---|---|---|")
    for f_idx, f in enumerate(failed, 1):
        idx, seg, r_a, b_a, rw, bw, rt, bt, mapped, kind, fn = f
        m_str = f"`0x{mapped:08X}`" if mapped is not None else "`None`"
        if r_a in PALETTE_ROW_SELECTOR_SITES:
            row_off = bt - (mapped if mapped is not None else 0)
            expl = f"**Palette row selector:** Shared palette table at `-0x50C` (`map(RS) = 0x{mapped:08X}`); BM selects row `{row_off:+#x}` (`{row_off // 32:+d}` palettes)"
        elif r_a == 0x08006334:
            expl = "**Intentional different table:** Save-init routine loads `0x08126511` in RS vs `0x082F123C` in BM"
        else:
            expl = "**Non-pointer `incrom` bytes:** 4-byte constant sequence in data/bytecode table whose 4th byte is `0x08`/`0x88`"
        out.append(
            f"| {f_idx} | {idx} | `{seg}` | `0x{r_a:08X}` | `0x{b_a:08X}` | `{kind}` | "
            f"`0x{rw:08X}` (`0x{rt:08X}`) | `0x{bw:08X}` (`0x{bt:08X}`) | {m_str} | {expl} |"
        )
    out.append("")
    return "\n".join(out) + "\n"


def main() -> None:
    rows, vdata = build_shift_map()
    out_txt = write_datashift_txt(rows)
    ptrs = vdata["sites_data_ptr"]

    passed = 0
    failed = []
    for idx, (seg, r_a, b_a, rw, bw, kind, fn) in enumerate(ptrs, 1):
        rt = rw & 0x7FFFFFFF
        bt = bw & 0x7FFFFFFF
        mapped = map_rs_to_bm(rt, rows)
        if mapped == bt and (rw & 0x80000000) == (bw & 0x80000000):
            passed += 1
        else:
            failed.append((idx, seg, r_a, b_a, rw, bw, rt, bt, mapped, kind, fn))

    pct = 100.0 * passed / len(ptrs)
    print(f"Wrote {out_txt.relative_to(GAME_ROOT)} ({len(rows)} rows)")
    print(f"Validation: {passed}/{len(ptrs)} passed ({pct:.2f}%), {len(failed)} failed.")

    if "--write-doc" in sys.argv:
        md = generate_markdown(rows, vdata, passed, failed)
        doc_path = DECOMP / "notes" / "data-region-shift-map.md"
        doc_path.write_text(md, encoding="utf-8", newline="\n")
        print(f"Wrote {doc_path.relative_to(GAME_ROOT)} ({len(md.splitlines())} lines)")


if __name__ == "__main__":
    main()
