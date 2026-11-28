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
- **Second batch (same day):**
  - `dbe86c0`: **`BN4_EVENT_SPAWN_LISTS`** (`code_06`, RS `0x080A1C34`, 1,000 B): 3 lists of 57 `event_spawn x, y, z, unk, params` records plus labelled parameter blobs. `EventSpawnListObjects` (`0x080A203A`) spawns them as object kind `0x16` in map group `0x92` between flags 9:`0x52` and 9:`0x53`. Tool: `tools/emit_event_spawns.py`.
  - `5f85c4c`: **`BN4_FLAG_PROPS`** (`code_06`, RS `0x080AEA00`, 1,184 B): 74 `flag_prop` records (sprite, animation, palette, attribute, a tested flag and two mirrored flags). `FlagProp_Update` is `0x080AEEA0`. Tool: `tools/emit_flag_props.py`.
  - `5ef49e4`: the 8 `u16` step tables of `dat_080A72C0` (`code_06`, 1,472 B), as labelled `.hword` arrays. Tool: `tools/emit_u16_curves.py`.
  - (next commit): **`BN4_CUTSCENE_ACTOR_SPAWNS`** (`code_06`, RS `0x0809CEC8`, 840 B): 42 `obj_spawn` records in `Object_SpawnList` format, without a terminator. `GetCutsceneActorSpawn` (`0x0802A760`) returns entry n, and cutscene code keeps it at `[r5,#0x34]`. Tool: `tools/emit_spawn_records.py`, which is general (a `BLOCKS` list, terminated or fixed arrays).
  - A scan for whole `incrom` blocks that parse as spawn lists found only 16 small blocks (1.1 KB, `code_04*`–`code_06`), none confirmed by a reader. Left alone.
- **Coverage:** 82.6% → **84.1%**+. `code_09` 85.7% → 96.1%; `code_03` 86.0% → 88.7%; `code_06` 71.4% → 74.6%.

## Key Decisions & Constraints
- All three are edited **in place** (the current convention since the emitters, see below). Each tool asserts the exact `incrom` line(s) it replaces and that RS == BM content, so it can't run twice or on a changed file.
- Macros (`chip_code`, `formation`, `formation_end`, `chip_codes`) are defined at the top of the segment after `.thumb`. Move them to an include if you prefer.
- New `names.txt` lines (`CLAUDE`): `BattleAssignEnemyHeldChip`, `GetEnemyHeldChips`, `BN4_ENEMY_HELD_CHIPS`, `BN4_CHIP_CODE_LISTS`. They're also `@ NAME` comments on the labels.

## Modified & Relevant Files
- `decomp/asm/code_09.s`, `decomp/asm/code_03.s`, `decomp/symbols/names.txt`.
- New tools: `decomp/tools/emit_enemy_held_chips.py`, `emit_formations.py`, `emit_chip_code_lists.py`.

## Regeneration pipeline (done, `27de1e7`): read this before editing asm
- **`tools/regen.sh` rebuilds the shared asm tree from the two ROMs alone** and keeps both builds byte-identical:
  1. a fast `disasm.py` pass over every segment, then a verified pass. The verify loop needs a consistent raw tree, so the passes can't be merged;
  2. `merge_bm.py`;
  3. every emitter, repeated until nothing changes (they seed from each other's labels). `emit_spawn_lists.py` runs first, so the cutscene emitters can't claim its lists;
  4. the single-table emitters, then both compares.
- **Hand-maintained segments, skipped by default:** `intr_table` (carved by hand) and `code_10` (the matching C in `src/*.c` is wired in as `.include "c/*.s"`). Naming them on the command line regenerates them, and you then redo those hand edits.
- **What it changed when adopted (84.3% → 86.5%):**
  - 9 segments are identical except for name comments: `disasm.py` applies `names.txt` at every call and label, so **add names to `names.txt` and regenerate** rather than editing labels by hand;
  - `code_04`–`code_06` gained 25.9 KB: spawn lists 6,432 B, plus more cutscene and map-script passes;
  - the only 76 bytes lost were false cutscene decodes of `BN4_MAP_BACKDROPS` data.
- **Rules for new tools:**
  - an emitter must only replace `incrom` lines, and must be safe to run again;
  - add it to `regen.sh` in the right place;
  - data macros go in `include/data_macros.inc` (loaded once by `macros.inc`: GNU as needs a macro defined before its first use, and `rom.s` assembles the segments in order).
- New this round: `BN4_FONT_WIDTHS` (2 fonts × 441 glyph widths; `Text_DrawGlyph` `0x08053B74`), via `tools/emit_font_widths.py`.

## Open Issues / Blockers
- Remaining false positives from `emit_cutscene_scripts.py` (e.g. `csc_b1 0x0A, 0x0A` runs inside `BN4_MAP_BACKDROPS` at `0x08085C24`). They're byte-safe, but the labels are wrong. A stricter seed rule (e.g. require a real terminator within N ops, or skip seeds that are entries of known data tables) would remove them; then rerun `regen.sh`.
- `code_03` `0x0804620C` (2,952 B, two pointers from the 16-byte descriptor table at `0x0804563C`) looks like a byte grid. Not decoded.

## Immediate Next Steps
1. Use `tools/regen.sh` for every regeneration from now on. Commit `asm/` only after both compares are OK.
2. Tighten the cutscene emitter's seed rule (above), then regenerate.
3. Remaining single tables: `code_05` `0x0808C71A` (1,374 B, after code, mixed bytecode); `code_03` `0x0804620C`; `code_06` `0x080A481C` (956 B), `0x080AE050` (888 B).
4. Trace what `BN4_CHIP_CODE_LISTS`' four lists are for (the readers of `dat_0804DE0C`).
