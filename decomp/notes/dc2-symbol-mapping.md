# DC-2 Symbolic Naming Database & Cross-Reference Report

**Date:** 2026-11-22
**Milestone:** DC-2 / DC-3 Transition
**Status:** Complete & Verified Byte-Identical
**Primary Artifacts:**
- Symbol Database: [`decomp/symbols/names.txt`](../symbols/names.txt)
- Generator Tool: [`decomp/tools/symbol_map.py`](../tools/symbol_map.py)

---

## 1. Executive Summary

As part of milestone DC-2, we established the unified symbolic naming database for *Mega Man Battle Network 4* (US Red Sun and US Blue Moon). This database cross-references:
1. **Cyberworld Endless (`CE`)**: The verified Red Sun reverse-engineering symbol catalog.
2. **Battle Network 6 Disassembly (`bn6f`)**: Canonical Capcom engine routine names, BIOS wrappers, memory utilities, and `oToolkit_*` architectural structures.
3. **EXE4 Japanese Disassembly (`exe4rs`)**: Original Japanese function boundaries, labels, and struct definitions.
4. **Automated Version Mappings**: `decomp/symbols/map_rs_bm.txt` (7,801 unique fingerprint-matched functions) and `decomp/symbols/shift_rs_bm.txt` (block-level shift regions).

### Key Metrics
- **Total Unified Symbols**: **753** entries in [`decomp/symbols/names.txt`](../symbols/names.txt).
- **CE Porting Match Rate**: **100.00%** (685 / 685 addressable symbols mapped from Red Sun to exact Blue Moon addresses).
- **Canonical Capcom Engine Routines**: **68** fully verified functions and core structures identified and cross-referenced with `bn6f` and `exe4rs`.
- **Non-Regression**: `make compare VERSION=rs` and `make compare VERSION=bm` both pass byte-for-byte (`OK`).

---

## 2. Version Shift Architecture & Mapping Methodology

Mega Man Battle Network 4 Red Sun (`B4WE`) and Blue Moon (`B4BE`) share nearly identical engine architecture, but diverge in tournament entrants, soul unities, battle chips, and graphics assets. This causes deterministic address shifts between versions:

### 2.1 RAM Symmetry (EWRAM & IWRAM)
- **EWRAM (`0x02000000 - 0x02040000`)** and **IWRAM (`0x03000000 - 0x03008000`)** maintain **identical addresses** between RS and BM.
- The global `eToolkit` pointer held in register `r10` resides at `0x02009E70` in both versions.
- Key RAM structures such as `ePrimaryRngSeed` (`0x02001790`), `eRngSecondarySeed` (`0x020015D4`), `eChipKeys` (`0x02000670`), `eTournaments` (`0x02001460`), and `eBattleState` (`0x02035810`) have 0-byte variance across versions.

### 2.2 ROM Code Region Piecewise Shifts (`0x08000000 - 0x0820B000`)
By analyzing the 7,801 matched function pairs in `decomp/symbols/map_rs_bm.txt`, we proved that code delta shifts occur in distinct contiguous intervals:

| RS Address Range | BM Address Range | Delta | Key Subsystems Present |
|---|---|:---:|---|
| `0x08000000 - 0x08020D6C` | `0x08000000 - 0x08020D6C` | **`+0x0`** | ResetHandler, CRT0, Interrupt handlers, MainLoop, Memory utilities, RNG routines, Sound queue, Joypad polling |
| `0x08020D6C - 0x08035088` | `0x08020D70 - 0x08035090` | **`+0x4`** | Mode 0 handlers, script interpreters, scene initialization |
| `0x08035088 - 0x0804F21C` | `0x08035090 - 0x0804F224` | **`+0x8`** | Overworld map subsystems, dialogue boxes, NPC interaction |
| `0x0804F21C - 0x0805EAA8` | `0x0804F228 - 0x0805EAB4` | **`+0xC`** | Menu subsystems, chip folder editor, tournament bracket logic |
| `0x0805EAA8 - 0x080612C8` | `0x0805EAC0 - 0x080612DC` | **`+0x18`** | Battle entity dispatcher transition routines |
| `0x080612C8 - 0x08068AA4` | `0x080612DC - 0x08068AB8` | **`+0x14`** | Chip execution routines and Soul Unison state tables |
| `0x08068AA4 - 0x08109F18` | `0x08068AB0 - 0x08109F24` | **`+0xC`** | Core battle system: navigation AI, enemy attacks, damage calculation |
| `0x08109F18 - 0x0820B000` | `0x08109F2C - 0x0820B014` | **`+0x14`** | BIOS SWI wrappers (`Div`, `Sqrt`, `CpuSet`, `CpuFastSet`), Flash ROM saving, and `m4a` sound engine |

### 2.3 ROM Data Region Shifts (`>= 0x0820B000`)
Data symbols beyond `0x0820B000` (palettes, tile sheets, compressed archives, text strings) shift based on version-specific assets. Using `decomp/symbols/shift_rs_bm.txt` and localized pattern recognition:
- `BN4_PLATE_SHEET_A` (`0x084BB768` RS) maps to `0x084BB6F0` BM (delta: `-0x78`).
- `BN4_BRACKET_PALETTE` (`0x086C5B0C` RS) maps to `0x086C5600` BM (delta: `-0x50C`).
- `BN4_BRACKET_ART_END` (`0x086C792C` RS) maps to `0x086C7420` BM (delta: `-0x50C`).
- `BN4_PANEL_TILESET` (`0x08704660` RS) maps to `0x08704154` BM (delta: `-0x50C`).
- `BN4_CHIP_NAMES` (`0x087481D8` RS) maps to `0x08747CCC` BM (delta: `-0x50C`).

---

## 3. Cross-Referenced Capcom Engine Subsystems

By cross-referencing against `ext/bn6f` and `ext/exe4rs`, we established community-canonical and Capcom-standard names for the major MMBN4 engine modules:

### 3.1 Memory Utilities (`mod_memory`)
Capcom used an identical suite of highly optimized memory operations across BN4, BN5, and BN6:
- `ZeroFillByByte` (`0x0800090C` RS/BM): Backward loop decrementing count and storing zero bytes (`subs r1, #1; strb r2, [r0, r1]; bne`).
- `ZeroFillByHalfword` (`0x08000918` RS/BM): Halfword fill using `SWI 0x0B` (`CpuSet`) with mask `0x01000000`.
- `ZeroFillByWord` (`0x08000938` RS/BM): Word fill using `SWI 0x0B` with mask `0x05000000`.
- `ZeroFillByEightWords` (`0x08000958` RS/BM): High-speed 32-byte chunk zeroing using `SWI 0x0C` (`CpuFastSet`).
- `CopyBytes` (`0x08000978` RS/BM): Backward byte copy (`subs r2, #1; ldrb r3, [r0, r2]; strb r3, [r1, r2]; bne`).
- `CopyHalfwords` (`0x08000982` RS/BM): Halfword copy via `SWI 0x0B`.
- `CopyWords` (`0x08000994` RS/BM): Word copy via `SWI 0x0B`.
- `CopyByEightWords` (`0x080009A8` RS/BM): Fast 32-byte chunk copy via `SWI 0x0C`.
- `ByteFill`, `HalfwordFill`, `WordFill`, `FillByEightWords` (`0x080009BC - 0x08000A04` RS/BM): Corresponding value fill routines.
- `ProcessGFXTransferQueue` (`0x08000858` RS/BM): FIFO queue handler committing queued VRAM block transfers.

### 3.2 GBA BIOS SWI Wrappers
In the `+0x14` shifted utility segment:
- `SWI_CpuFastSet` (`0x0811187C` RS / `0x08111890` BM): `svc #0x0C`.
- `SWI_CpuSet` (`0x08111880` RS / `0x08111894` BM): `svc #0x0B`.
- `SWI_Div` (`0x08111884` RS / `0x08111898` BM): `svc #0x06` returning quotient in `r0`.
- `SWI_DivRem` (`0x08111888` RS / `0x0811189C` BM): `svc #0x06` followed by `adds r0, r1, #0` returning remainder in `r0`.
- `SWI_LZ77UnCompWram` (`0x08111890` RS / `0x081118A4` BM): `svc #0x12`.
- `SWI_LZ77UnCompVram` (`0x08111894` RS / `0x081118A8` BM): `svc #0x11`.
- `SWI_Sqrt` (`0x08111898` RS / `0x081118AC` BM): `svc #0x08`.
- `MemCopyForward` (`0x0811189C` RS / `0x081118B0` BM): Forward byte memcpy loop.

### 3.3 Random Number Generation (`mod_rng`)
MMBN4 uses Capcom's dual-generator LCG system:
- Multiplier / XOR Constant: `0x873CA9E5` (identical in BN4, BN5, and BN6).
- `SeedRNG` (`0x08001AF8` RS/BM): Seeds `ePrimaryRngSeed` (`0x02001790`) with `0xA338244F`.
- `GetRNG` (`0x08001B00` RS/BM): Advances primary RNG state and returns next 32-bit integer.
- `GetPositiveSignedRNG` (`0x08001B16` RS/BM): Masks sign bit returning `0..0x7FFFFFFF`.
- `GetSecondaryRNG` (`0x08001B30` RS/BM): Advances secondary RNG state `eRngSecondarySeed` (`0x020015D4`).
- `ShuffleByteList_PrimaryRNG` (`0x08000AAA` RS/BM): Shuffles chip selection and drop arrays.

### 3.4 Sound Engine (`Music Player 2000` / `m4a`)
MMBN4 uses the Capcom-modified Nintendo m4a sound driver:
- `PlaySoundEffect` (`0x08000560` RS/BM): Enqueues sound effect command.
- `PlayMusic` (`0x08000586` RS/BM): Updates `oGameState_BGMusicIndicator` and triggers track change.
- `SoundQueue_Enqueue` (`0x0800073C` RS/BM): Pushes audio commands to circular FIFO.
- `SoundQueue_Process` (`0x0800070E` RS/BM): Drains and issues audio commands.
- `SoundMain` (`0x08111B14` RS / `0x08111B28` BM): Driver frame calculation.
- `m4aSoundMain` (`0x0811283C` RS / `0x08112850` BM): Main sound tick wrapper.
- `m4aSongNumStart` (`0x08112848` RS / `0x0811285C` BM): Starts playback by song table ID.
- `m4aSongNumStop` (`0x0811297C` RS / `0x08112990` BM): Stops playback by song table ID.

### 3.5 Toolkit Architecture (`r10`)
Throughout MMBN4 assembly, register `r10` (`sl`) is permanently reserved for the `Toolkit` pointer (`0x02009E70`), matching `Toolkit.inc` in `bn6f` and `Toolkit.h` in `exe4rs`:
- `oToolkit_MainJumptableIndexPtr` (`+0x00`)
- `oToolkit_JoypadPtr` (`+0x04`)
- `oToolkit_RenderInfoPtr` (`+0x08`)
- `oToolkit_CurFramePtr` (`+0x24`)
- `oToolkit_iBGTileIdBlocks_Ptr` (`+0x28`)
- `oToolkit_ChatboxPtr` (`+0x2C`)
- `oToolkit_CollisionDataPtr` (`+0x30`)
- `oToolkit_SubmenuPtr` (`+0x34`)
- `oToolkit_GameStatePtr` (`+0x3C` BN6 / `+0x40` BN4)
- `oToolkit_EventFlagsPtr` (`+0x44` BN4)

---

## 4. Symbol Database Structure (`decomp/symbols/names.txt`)

The database is formatted with whitespace-delimited columns designed for both automated parsing and human inspection:

```text
# addr_rs   addr_bm   kind   source       symbol_name                      description
080000C0 080000C0 code   BN6F,EXE4    ResetHandler                     ARM reset and CRT0 entry point
0800029C 0800029C code   BN6F,EXE4    MainLoop                         Top-level frame execution loop
08000560 08000560 code   BN6F,CE      PlaySoundEffect                  Play sound effect index
08000586 08000586 code   BN6F,CE      PlayMusic                        Change background music track
0800090C 0800090C code   BN6F         ZeroFillByByte                   Backward byte-by-byte zero fill
08001B00 08001B00 code   BN6F,CE      GetRNG                           Step primary 32-bit RNG and return next pseudo-random value
0811187C 08111890 code   BN6F         SWI_CpuFastSet                   GBA BIOS SWI 0x0C: 32-byte fast block copy/fill
08111884 08111898 code   BN6F         SWI_Div                          GBA BIOS SWI 0x06: signed integer division (r0 / r1)
0811283C 08112850 code   BN6F         m4aSoundMain                     Music Player 2000 main tick update wrapper
```

### Breakdown by Category

| Kind | Total Symbols | RS Addr Range | BM Addr Range | Description |
|---|:---:|---|---|---|
| `code` | 237 | `0x080000C0 - 0x08706640` | `0x080000C0 - 0x08706134` | Executable subroutines & dispatch entries |
| `data` | 219 | `0x0800032C - 0x0874F03C` | `0x0800032C - 0x0874EACC` | Lookup tables, pointers, palettes, art sheets |
| `rom` | 248 | `0x080000AC - 0x086C792C` | `0x080000AC - 0x086C7420` | Header literals, ROM boundaries, end-markers |
| `ram` | 48 | `0x02000670 - 0x02035850` | `0x02000670 - 0x02035850` | Global state variables, hardware buffers, seeds |
| `constant` | 1 | `0x08800000` | `0x08800000` | Guest core boundary constant |
| **Total** | **753** | | | |

---

## 5. Recommendations for Milestone DC-3

With the symbolic database established, the following steps are recommended for Milestone DC-3:

1. **Integrate into `tools/disasm.py` and `tools/split.py`**:
   - Update the disassembler to consume `decomp/symbols/names.txt` when generating labels in `decomp/asm/*.s` and `decomp/asm/bm/*.s`.
   - Prefer canonical Capcom names (`ResetHandler`, `ZeroFillByByte`, `GetRNG`) over autogenerated `sub_0800090C` labels.
2. **Versioned Linker Script Symbols (`symbols.ld`)**:
   - Update `tools/ldsyms.py` to emit human-readable symbols into `decomp/build/rs/symbols.ld` and `decomp/build/bm/symbols.ld` so the generated ELF map files display symbolic call graphs.
3. **Struct & Constant Definitions Header**:
   - Create `decomp/include/toolkit.inc` and `decomp/include/constants.inc` based on `ext/bn6f/include/structs/` and `ext/exe4rs/structs/` to standardize register offsets (e.g., `#oToolkit_JoypadPtr` instead of `#0x4`).
4. **Gradual Symbol Propagation**:
   - Replace raw hexadecimal addresses in literal pools with symbolic equates (`.equ ePrimaryRngSeed, 0x02001790`). Verify that each substitution maintains identical `.word` encodings in the built GBA ROMs.
