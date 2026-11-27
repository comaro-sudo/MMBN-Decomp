# Claude decomp handoff (2026-11-27)

## Goal & Current Status
- **Done (verified: `make -B compare` OK for RS and BM after each commit):**
  - `29a19f7`: **`BN4_ENEMY_HELD_CHIPS`** (`code_09`, RS `0x080F8A80` / BM `0x080F8A8C`, 7,808 B).
    - 244 enemy ids × 16 `u16` `chip | code << 9` (`0xFFFF` = none).
    - `GetEnemyHeldChips` (`0x080F8A70`) indexes it. `BattleAssignEnemyHeldChip` (`0x080F576C`) gives one random enemy a random entry at `obj+0x2A` at battle start.
    - Tool: `tools/emit_enemy_held_chips.py`.
  - `4801e6c`: **story-battle formation lists** (`code_09`, RS `0x080FB47C`–`0x080FC138`, BM +0xC, 3,260 B).
    - 243 byte-packed lists of `formation id, col, row, kind` + `formation_end`, with the 242 `dat_` labels `BN4_STORY_BATTLES` points to.
    - Also removes two false pointer runs that `emit_desc_tables.py` had carved from formation bytes.
    - Tool: `tools/emit_formations.py`.
  - `82e6274`: **`BN4_CHIP_CODE_LISTS`** (`code_03`, RS `0x0804CFF8`–`0x0804DAF0`, BM +8, 2,808 B).
    - 468 `chip_codes id, c0..c3` records in 4 ascending lists, via the descriptor table `dat_0804DE0C`.
    - Purpose not traced (named neutrally).
    - Tool: `tools/emit_chip_code_lists.py`.
- **Coverage:** 82.6% → **83.8%**. `code_09` 85.7% → 96.1%; `code_03` 86.0% → 88.7%.

## Key Decisions & Constraints
- All three are edited **in place** (the current convention since the emitters, see below). Each tool asserts the exact `incrom` line(s) it replaces and that RS == BM content, so it can't run twice or on a changed file.
- Macros (`chip_code`, `formation`, `formation_end`, `chip_codes`) are defined at the top of the segment after `.thumb`. Move them to an include if you prefer.
- New `names.txt` lines (`CLAUDE`): `BattleAssignEnemyHeldChip`, `GetEnemyHeldChips`, `BN4_ENEMY_HELD_CHIPS`, `BN4_CHIP_CODE_LISTS`. They're also `@ NAME` comments on the labels.

## Modified & Relevant Files
- `decomp/asm/code_09.s`, `decomp/asm/code_03.s`, `decomp/symbols/names.txt`.
- New tools: `decomp/tools/emit_enemy_held_chips.py`, `emit_formations.py`, `emit_chip_code_lists.py`.

## Open Issues / Blockers
- **The emitters can't be re-run from a clean `disasm.py` output** in a known order: there is no pipeline script. A full regeneration of a segment would drop every emitter's output, mine included. Worth a `tools/regen_segment.sh` that runs `disasm.py rs/bm` + `merge_bm.py` + every emitter in order, then compares.
- **False positives in `emit_cutscene_scripts.py`** (`code_05`):
  - About 8 KB of `incrom` right after `csc_end` lines are **`Object_SpawnList` lists** (`0x080034E4`): 20-byte records `{u8 spawn kind, u8 arg, u16 pad, s32 x, s32 y, s32 z (16.16), u32 param}`, ended by a kind of `0xFF`.
  - The emitter read their `05 06 00` field bytes as `csc_b1 0x05, 0x06; csc_end`, e.g. at `0x080715D8` and `0x080716EC`.
  - Example: `sub_08071574` → `dat_0807158C`, 16 list pointers indexed by map.
  - Fixing it needs the cutscene emitter to skip `Object_SpawnList` targets, then a regeneration (see above). Bytes are still identical, so there's no build risk, only wrong labels.
- `code_03` `0x0804620C` (2,952 B, two pointers from the 16-byte descriptor table at `0x0804563C`) looks like a byte grid (`00`/`01`). Not decoded.

## Immediate Next Steps
1. A regeneration pipeline script; then fix the `Object_SpawnList` false positives and emit those lists (≈8 KB in `code_05`, likely more in `code_04*`/`code_06`).
2. The remaining large single tables: `code_06` `0x080A72E0` (1,472 B), `0x080AEA00` (1,184 B), `0x080A1C40` (1,000 B); `code_05` `0x0808C71A` (1,374 B).
3. Trace what `BN4_CHIP_CODE_LISTS`' four lists are for (the readers of `dat_0804DE0C`).
