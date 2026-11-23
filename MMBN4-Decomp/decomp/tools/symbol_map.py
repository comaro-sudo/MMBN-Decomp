#!/usr/bin/env python3
"""Cross-reference and establish the symbolic naming database for MMBN4 RS & BM.

Parses:
- ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/symbols/bn4-red-sun-us.csv
- decomp/symbols/map_rs_bm.txt
- decomp/symbols/shift_rs_bm.txt
- decomp/symbols/functions_rs.txt & functions_bm.txt
- ext/bn6f (canonical Capcom names)
- ext/exe4rs (disassembly labels & toolkit structures)

Outputs:
- decomp/symbols/names.txt
"""

import csv
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DECOMP = ROOT / "decomp"
SYMBOLS_DIR = DECOMP / "symbols"
CE_CSV = ROOT / "ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/symbols/bn4-red-sun-us.csv"
MAP_FILE = SYMBOLS_DIR / "map_rs_bm.txt"
SHIFT_FILE = SYMBOLS_DIR / "shift_rs_bm.txt"
FUNCS_RS = SYMBOLS_DIR / "functions_rs.txt"
FUNCS_BM = SYMBOLS_DIR / "functions_bm.txt"

RS_ROM_PATH = ROOT / "roms/bn4_rs_us.gba"
BM_ROM_PATH = ROOT / "roms/bn4_bm_us.gba"


def load_map_rs_bm():
    rs_to_bm = {}
    if MAP_FILE.exists():
        with MAP_FILE.open("r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split()
                rs = int(parts[0], 16)
                bm = int(parts[1], 16)
                size = int(parts[2])
                rs_to_bm[rs] = (bm, size)
    return rs_to_bm


def load_shifts():
    shifts = []
    if SHIFT_FILE.exists():
        with SHIFT_FILE.open("r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split()
                s = int(parts[0], 16)
                e = int(parts[1], 16)
                d = parts[2]
                delta = None if d == "None" else (int(d, 16) if not d.startswith("-") else -int(d[1:], 16))
                shifts.append((s, e, delta))
    return shifts


def map_rs_to_bm(rs_addr, map_rs_bm, shifts, rs_rom, bm_rom):
    """Calculate exact Blue Moon address for a Red Sun symbol."""
    # RAM is identical between RS and BM
    if 0x02000000 <= rs_addr < 0x04000000:
        return rs_addr, "RAM_EXACT"

    # Direct function map
    rs_even = rs_addr & ~1
    thumb_bit = rs_addr & 1
    if rs_even in map_rs_bm:
        return (map_rs_bm[rs_even][0] | thumb_bit), "MAP_RS_BM"

    # Known code shift intervals
    if 0x08000000 <= rs_addr < 0x08020D6C:
        return rs_addr, "CODE_DELTA_+0"
    elif 0x08020D6C <= rs_addr < 0x08035088:
        return rs_addr + 4, "CODE_DELTA_+4"
    elif 0x08035088 <= rs_addr < 0x0804F21C:
        return rs_addr + 8, "CODE_DELTA_+8"
    elif 0x0804F21C <= rs_addr < 0x0805EAA8:
        return rs_addr + 12, "CODE_DELTA_+12"
    elif 0x0805EAA8 <= rs_addr < 0x080612C8:
        return rs_addr + 24, "CODE_DELTA_+24"
    elif 0x080612C8 <= rs_addr < 0x08068AA4:
        return rs_addr + 20, "CODE_DELTA_+20"
    elif 0x08068AA4 <= rs_addr < 0x08109F18:
        return rs_addr + 12, "CODE_DELTA_+12"
    elif 0x08109F18 <= rs_addr < 0x0820B000:
        return rs_addr + 20, "CODE_DELTA_+20"

    # Data shifts past code
    for s, e, d in shifts:
        if s <= rs_addr < e and d is not None:
            return rs_addr + d, f"SHIFT_TABLE_{d:+x}"

    # Verbatim search in BM ROM for data blocks
    if 0x08000000 <= rs_addr < 0x08800000:
        off = rs_addr - 0x08000000
        pat = rs_rom[off : off + 16]
        pos = bm_rom.find(pat, max(0, off - 0x2000), min(len(bm_rom), off + 0x2000))
        if pos != -1:
            return 0x08000000 + pos, f"VERBATIM_FIND_{pos - off:+x}"

    return None, "UNMAPPED"


def main():
    rs_rom = RS_ROM_PATH.read_bytes()
    bm_rom = BM_ROM_PATH.read_bytes()
    map_rs_bm = load_map_rs_bm()
    shifts = load_shifts()

    symbols = {}  # (addr_rs, name) -> dict of properties

    # 1. Canonical Capcom Engine Routines (BN6F & EXE4RS)
    canonical_engine = [
        # System & Entry
        (0x080000C0, 0x080000C0, "ResetHandler", "code", "BN6F,EXE4", "ARM reset and CRT0 entry point"),
        (0x080001B0, 0x080001B0, "ClearRam_Entry", "code", "BN6F,EXE4", "CRT0 loop clearing EWRAM and IWRAM"),
        (0x080001C4, 0x080001C4, "CopyIwramCode_Entry", "code", "BN6F,EXE4", "CRT0 loop copying IWRAM code chunk"),
        (0x0800025C, 0x0800025C, "ThumbEntry", "code", "BN6F,EXE4", "Initial Thumb switch and stack preparation"),
        (0x08000284, 0x08000284, "InitToolkitAndSubsystems", "code", "BN6F,EXE4", "Initialize r10 toolkit pointer and subsystems"),
        (0x0800029C, 0x0800029C, "MainLoop", "code", "BN6F,EXE4", "Top-level frame execution loop"),
        (0x0800032C, 0x0800032C, "MainSubsystemJumpTable", "data", "BN6F,EXE4,CE", "Jump table for main engine subsystem modes"),
        (0x0800036C, 0x0800036C, "main_awaitFrame", "code", "BN6F,EXE4", "Wait for VBlank interval synchronization"),
        (0x0800039C, 0x0800039C, "main_pollGeneralLCDStatus", "code", "BN6F,EXE4", "Poll DISPSTAT VCounter match"),
        (0x080003B0, 0x080003B0, "Joypad_Poll", "code", "BN6F,EXE4", "Read KEYINPUT register, calculate held and repeat keys"),
        (0x08000418, 0x08000418, "VBlankIntr", "code", "BN6F,EXE4", "Vertical blank interrupt handler"),
        (0x0800045E, 0x0800045E, "HBlankIntr", "code", "BN6F,EXE4", "Horizontal blank interrupt handler"),
        (0x08000504, 0x08000504, "TimerIntr", "code", "BN6F,EXE4", "Timer 3 sound interrupt handler"),
        (0x08000568, 0x08000568, "SerialIntr", "code", "BN6F,EXE4", "Serial communication interrupt handler"),
        (0x08000C2C, 0x08000C2C, "CapIncrementGameTimeFrames", "code", "BN6F,EXE4", "Increment in-game play timer capped at 99:59:59"),
        (0x08001CD8, 0x08001CD8, "Render_CommitRegisters", "code", "BN6F", "Commit buffered display and background registers to hardware I/O"),
        (0x08001DA8, 0x08001DA8, "Clear_iBGTileIdBlocks", "code", "BN6F", "Zero-fill background tile ID buffer (0x2000 bytes)"),
        (0x08001EC0, 0x08001EC0, "ScreenFade_Update", "code", "BN6F", "Update screen fade transitions and palette brightness"),
        (0x080029E0, 0x080029E0, "Oam_ResetObjects", "code", "BN6F", "Reset OAM sprite attribute buffer entries"),

        # Memory Utilities
        (0x08000858, 0x08000858, "ProcessGFXTransferQueue", "code", "BN6F", "Drain and process queued GFX transfers to VRAM"),
        (0x0800090C, 0x0800090C, "ZeroFillByByte", "code", "BN6F", "Backward byte-by-byte zero fill"),
        (0x08000918, 0x08000918, "ZeroFillByHalfword", "code", "BN6F", "Halfword zero fill via CpuSet"),
        (0x08000938, 0x08000938, "ZeroFillByWord", "code", "BN6F", "Word zero fill via CpuSet"),
        (0x08000958, 0x08000958, "ZeroFillByEightWords", "code", "BN6F", "Fast 8-word zero fill via CpuFastSet"),
        (0x08000978, 0x08000978, "CopyBytes", "code", "BN6F", "Backward byte-by-byte memory copy"),
        (0x08000982, 0x08000982, "CopyHalfwords", "code", "BN6F", "Halfword memory copy via CpuSet"),
        (0x08000994, 0x08000994, "CopyWords", "code", "BN6F", "Word memory copy via CpuSet"),
        (0x080009A8, 0x080009A8, "CopyByEightWords", "code", "BN6F", "Fast 8-word memory copy via CpuFastSet"),
        (0x080009BC, 0x080009BC, "ByteFill", "code", "BN6F", "Forward byte fill with given value"),
        (0x080009C4, 0x080009C4, "HalfwordFill", "code", "BN6F", "Halfword fill with value via CpuSet"),
        (0x080009E4, 0x080009E4, "WordFill", "code", "BN6F", "Word fill with value via CpuSet"),
        (0x08000A04, 0x08000A04, "FillByEightWords", "code", "BN6F", "Fast 8-word fill with value via CpuFastSet"),
        (0x08000A38, 0x08000A38, "memory_bcdConvert", "code", "BN6F", "Convert integer to binary-coded decimal"),

        # BIOS SWI Wrappers
        (0x0811187C, 0x08111890, "SWI_CpuFastSet", "code", "BN6F", "GBA BIOS SWI 0x0C: 32-byte fast block copy/fill"),
        (0x08111880, 0x08111894, "SWI_CpuSet", "code", "BN6F", "GBA BIOS SWI 0x0B: 16/32-bit block copy/fill"),
        (0x08111884, 0x08111898, "SWI_Div", "code", "BN6F", "GBA BIOS SWI 0x06: signed integer division (r0 / r1)"),
        (0x08111888, 0x0811189C, "SWI_DivRem", "code", "BN6F", "GBA BIOS SWI 0x06: signed integer remainder (r0 % r1)"),
        (0x08111890, 0x081118A4, "SWI_LZ77UnCompWram", "code", "BN6F", "GBA BIOS SWI 0x12: LZ77 decompression to WRAM (8-bit)"),
        (0x08111894, 0x081118A8, "SWI_LZ77UnCompVram", "code", "BN6F", "GBA BIOS SWI 0x11: LZ77 decompression to VRAM (16-bit)"),
        (0x08111898, 0x081118AC, "SWI_Sqrt", "code", "BN6F", "GBA BIOS SWI 0x08: integer square root"),
        (0x0811189C, 0x081118B0, "MemCopyForward", "code", "BN6F", "Standard forward byte-by-byte memory copy"),

        # RNG
        (0x08001AF8, 0x08001AF8, "SeedRNG", "code", "BN6F", "Initialize primary RNG seed with default constant"),
        (0x08001B00, 0x08001B00, "GetRNG", "code", "BN6F,CE", "Step primary 32-bit RNG and return next pseudo-random value"),
        (0x08001B16, 0x08001B16, "GetPositiveSignedRNG", "code", "BN6F", "Step primary RNG and return positive 31-bit value"),
        (0x08001B30, 0x08001B30, "GetSecondaryRNG", "code", "BN6F,CE", "Step secondary RNG state"),
        (0x08001B46, 0x08001B46, "GetPositiveSignedSecondaryRNG", "code", "BN6F", "Step secondary RNG and return positive value"),
        (0x08001B60, 0x08001B60, "GetTertiaryRNG", "code", "BN6F", "Step tertiary RNG state"),
        (0x08000AAA, 0x08000AAA, "ShuffleByteList_PrimaryRNG", "code", "BN6F,CE", "Fisher-Yates byte array shuffle using primary RNG"),
        (0x08000AC6, 0x08000AC6, "ShuffleHwordList_PrimaryRNG", "code", "BN6F", "Fisher-Yates halfword array shuffle using primary RNG"),
        (0x08000B04, 0x08000B04, "ShuffleHwordList_SecondaryRNG", "code", "BN6F", "Fisher-Yates halfword array shuffle using secondary RNG"),

        # Sound (Music Player 2000 / m4a)
        (0x08000560, 0x08000560, "PlaySoundEffect", "code", "BN6F,CE", "Play sound effect index"),
        (0x08000586, 0x08000586, "PlayMusic", "code", "BN6F,CE", "Change background music track"),
        (0x08000702, 0x08000702, "SoundQueue_Clear", "code", "BN6F", "Clear sound command queue buffer"),
        (0x0800070E, 0x0800070E, "SoundQueue_Process", "code", "BN6F", "Drain and dispatch pending sound queue commands"),
        (0x0800073C, 0x0800073C, "SoundQueue_Enqueue", "code", "BN6F", "Enqueue sound command into circular buffer"),
        (0x08111B14, 0x08111B28, "SoundMain", "code", "BN6F", "Capcom sound engine core audio driver update"),
        (0x0811283C, 0x08112850, "m4aSoundMain", "code", "BN6F", "Music Player 2000 main tick update wrapper"),
        (0x08112848, 0x0811285C, "m4aSongNumStart", "code", "BN6F", "Music Player 2000 start song by sequence ID"),
        (0x0811297C, 0x08112990, "m4aSongNumStop", "code", "BN6F", "Music Player 2000 stop song by sequence ID"),

        # Engine Structs in EWRAM / IWRAM
        (0x02000670, 0x02000670, "eChipKeys", "ram", "CE", "Save-file chip validation keys (eChipKeys + id ^ 0x64)"),
        (0x02001460, 0x02001460, "eTournaments", "ram", "EXE4,CE", "Tournament bracket entrants and round states (Toolkit+0x3C)"),
        (0x020015D4, 0x020015D4, "eRngSecondarySeed", "ram", "BN6F,CE", "Secondary RNG state seed word"),
        (0x02001790, 0x02001790, "ePrimaryRngSeed", "ram", "BN6F,CE", "Primary RNG state seed word"),
        (0x02001E98, 0x02001E98, "eTertiaryRngSeed", "ram", "BN6F", "Tertiary RNG state seed word"),
        (0x02009E70, 0x02009E70, "eToolkit", "ram", "EXE4,BN6F", "Global engine toolkit structure (always held in r10)"),
        (0x02035810, 0x02035810, "eBattleState", "ram", "BN6F,CE", "Battle state structure (Toolkit+0x18)"),
        (0x0203581D, 0x0203581D, "eBattleSide", "ram", "CE", "Current battlefield side of player"),
        (0x02035850, 0x02035850, "eBattleTimer", "ram", "BN6F,CE", "Battle frame duration timer"),
    ]

    for rs_a, bm_a, name, kind, src, desc in canonical_engine:
        symbols[(rs_a, name)] = {
            "addr_rs": rs_a,
            "addr_bm": bm_a,
            "name": name,
            "kind": kind,
            "source": src,
            "desc": desc,
        }

    # 2. Port Cyberworld Endless Symbols
    ce_mapped = 0
    ce_total = 0
    with CE_CSV.open("r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            addr_str = row["address"].strip()
            if not addr_str:
                continue
            rs_addr = int(addr_str, 16)
            name = row["name"].strip()
            kind = row["kind"].strip()
            desc = row["description"].strip()
            source = "CE"

            # Skip non-memory constant offsets (e.g. 0x00000000..0x0000000B)
            if rs_addr < 0x02000000:
                continue

            ce_total += 1
            if rs_addr == 0x08800000:
                bm_addr = rs_addr
                method = "CONSTANT"
            else:
                bm_addr, method = map_rs_to_bm(rs_addr, map_rs_bm, shifts, rs_rom, bm_rom)

            if bm_addr is not None:
                ce_mapped += 1
                key = (rs_addr, name)
                if key in symbols:
                    # Merge source tags
                    symbols[key]["source"] += ",CE"
                    if not symbols[key]["desc"]:
                        symbols[key]["desc"] = desc
                else:
                    symbols[key] = {
                        "addr_rs": rs_addr,
                        "addr_bm": bm_addr,
                        "name": name,
                        "kind": kind,
                        "source": source,
                        "desc": desc,
                    }

    # Sort all symbols by addr_rs, then name
    sorted_symbols = sorted(symbols.values(), key=lambda s: (s["addr_rs"], s["name"]))

    # Write decomp/symbols/names.txt
    names_path = SYMBOLS_DIR / "names.txt"
    with names_path.open("w", encoding="utf-8") as f:
        f.write("# MMBN4 Unified Symbolic Naming Database (RS & BM)\n")
        f.write("# Generated for Milestone DC-2 / DC-3\n")
        f.write("# Format: addr_rs   addr_bm   kind   source   symbol_name   description\n")
        f.write("# -------------------------------------------------------------------------\n")
        for s in sorted_symbols:
            desc = s["desc"].replace("\n", " ").replace("\r", "")
            f.write(f"{s['addr_rs']:08X} {s['addr_bm']:08X} {s['kind']:<6} {s['source']:<12} {s['name']:<32} {desc}\n")

    print(f"Generated {names_path}")
    print(f"Total unified symbols: {len(sorted_symbols)}")
    print(f"CE symbols ported: {ce_mapped} / {ce_total} ({ce_mapped / ce_total * 100:.2f}%)")
    print(f"Canonical Capcom engine symbols: {len(canonical_engine)}")


if __name__ == "__main__":
    main()
