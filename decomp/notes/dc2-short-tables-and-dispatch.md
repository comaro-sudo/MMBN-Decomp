# DC-2: Short Pointer Tables and Indirect Dispatch Verification

Date: 2026-11-22
Status: Landed in `tools/survey.py`, symbols updated, all segments regenerated and verified byte-identical.

---

## 1. Executive Summary

We advanced milestone DC-2 by broadening short pointer table discovery and incorporating preceded `0xB4xx` (`push {r...}`) leaf prologues into `tools/survey.py`.

- **Coverage Increase**:
  - **Red Sun**: **52.4%** code ownership (617,882 / 1,179,456 bytes, +4,298 source bytes).
  - **Blue Moon**: **52.0%** code ownership (613,314 / 1,179,476 bytes, +1,494 source bytes).
- **Segment Highlights**:
  - `code_00`: RS **85.0%** / BM **84.2%**
  - `code_01`: RS **53.9%** / BM **53.2%**
  - `code_02`: RS **73.4%** / BM **73.4%**
  - `code_03`: RS **75.2%** / BM **75.2%**
  - `code_07`: RS **90.1%** / BM **90.0%**
  - `code_08`: RS **89.9%** / BM **89.8%**
  - `code_10`: RS **46.8%** / BM **43.5%**
- **Byte-Identical Verification**:
  - `build/rs/bn4_rs.gba: OK` (SHA-1: `a97e96a7d9036c1e95ba138e67cfcfad566df3f2`)
  - `build/bm/bn4_bm.gba: OK` (SHA-1: `5e017c80521e8e4584988775f0a0dca0a52f8623`)

---

## 2. Methodology & Changes

### A. Broadening Short Pointer Tables (`short_prologue_pointer_tables`)
Previously, `short_prologue_pointer_tables()` in `tools/survey.py` required that:
1. Every target in a 2- to 3-entry run of odd pointers started with `push {..., lr}` (`0xB500`).
2. None of the targets were already in `self.code`.

This caused valid callback tables (such as Table 4 at `0x0807738C` and Table 6 at `0x0807CC5C` in `code_05`) to be rejected because their targets are small stub/leaf functions that return via `mov pc, lr` (`0x46F7`), or because one entry was already discovered via another call path.

**Fix**:
- Replaced the strict `0xB500` requirement with `self.verify_clean_thumb_func(start)` for all targets.
- Required `BASE + 0x25C <= start < code_limit` and `start not in self.literals`.
- Required `any(start not in self.funcs for start in starts)` so new targets are admitted even if one target was already known.

**Result**:
- Short tables increased from 838 tables / 1,981 targets to **1,162 tables / 2,631 targets** in RS (+650 targets) and **1,163 tables / 2,645 targets** in BM.

### B. Preceded `0xB4xx` Discovery (`clean_gap_prologue_pass`)
In ARM Thumb, non-calling leaf functions frequently save only callee registers (`push {r4-r7}`, `0xB4xx`) and return with `pop {r4-r7}; mov pc, lr` or `bx lr`.
- Scanned undecoded gaps for `0xB4xx` instructions immediately preceded by standard function termination or alignment padding: `0x0000`, `0x46F7` (`mov pc, lr`), `0x4770` (`bx lr`), `0xBDxx` (`pop {..., pc}`), or `0xE0xx` (`b <imm>`).
- Required that `self.verify_clean_thumb_func(addr)` decodes to an authentic return.

---

## 3. Deep Dive: `code_05` and the 9 Indirect Dispatch Sites

`code_05` ownership remained at ~6.6% (RS 9,600 / 144,756 bytes; BM 9,392 bytes). We conducted an in-depth audit of all 9 indirect register-call dispatch sites (`bx r7` / `bx r0`):

| RS Site | BM Site | Dispatch Form | Literal | Table | Entries | All Clean? |
|---|---|---|---|---|---:|:---:|
| `0x080714D0` | `0x080714DC` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x080714D4` | `0x080714D8` | 16 | **Yes** |
| `0x08073B4C` | `0x08073B58` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x08073B50` | `0x08073B54` | 12 | **Yes** |
| `0x08075B78` | `0x08075B84` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x08075B7C` | `0x08075B80` | 16 | **Yes** |
| `0x08077384` | `0x08077390` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x08077388` | `0x0807738C` | 3 | **Yes** |
| `0x080798D0` | `0x080798DC` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x080798D4` | `0x080798D8` | 4 | **Yes** |
| `0x0807CC54` | `0x0807CC60` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x0807CC58` | `0x0807CC5C` | 3 | **Yes** |
| `0x0807FCC4` | `0x0807FCD0` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x0807FCC8` | `0x0807FCCC` | 4 | **Yes** |
| `0x080826EC` | `0x080826F8` | `ldr r7, [r7, r6]; mov lr, pc; bx r7` | `0x080826F0` | `0x080826F4` | 8 | **Yes** |
| `0x0809473C` | `0x08094748` | `ldr r0, [r0, r1]; mov lr, pc; bx r0` | `0x08094744` | `0x08094748` | 4 | **Yes** |

**Findings**:
1. **100% Target Cleanliness**: All 70 target routines across all 9 tables in both Red Sun and Blue Moon are verified clean Thumb routines, and all are seeded in `symbols/functions_<v>.txt`.
2. **Exact BM Delta**: Every single table, site, and target has an exact `+0xC` delta in Blue Moon.
3. **Data Content of `code_05`**: The remaining ~135 KB in `code_05` consists of battle background tile scripts (`BN4_BATTLE_BGS`), animation structures, backdrop palettes, and scroll parameter tables (e.g. gaps `0x08073B9A - 0x08074CC6` and `0x0807CC82 - 0x0807D902` containing even data pointers and animation records). `code_05` is therefore primarily a data-heavy background subsystem rather than uncarved executable code.

---

## 4. Analysis of Remaining Gaps (`code_04`, `code_06`, `code_09`, `code_10`)

1. **`code_04` (13.6% ownership)**: Contains interleaved chip parameter tables, object behavior scripts, and animation descriptors with even ROM pointers.
2. **`code_06` (28.3% ownership)**: Contains extensive EWRAM/IWRAM buffer reference tables (`0x0200xxxx`, `0x0300xxxx`) and object state structures.
3. **`code_09` & `code_10` (43-46% ownership)**:
   - Contains the primary sound engine / m4a track and voice data region spanning `0x080F8A7A - 0x08102FC8` (~42.3 KB of voice tables, sample headers, and padding).
   - In `code_10`, the first function start does not occur until `0x08102FC8`, confirming that `0x080FE7E4 - 0x08102FC8` is sound bank data.
   - Preserves the Capcom build timestamp string at `0x08113CB6` ("Apr 16 2004 14...").
