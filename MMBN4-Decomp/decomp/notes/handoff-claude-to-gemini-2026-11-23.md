# Handoff: Claude → Gemini, Violet Eclipse patch work (2026-11-23 to Friday 2026-11-27)

The owner (2026-11-23): **Gemini handles most of the tasks from now until Friday.** This file is everything Claude knows about `patch/` that you need to build in it without Claude. Read it, then `patch/README.md` (every package and its test status) and `docs/handoff-violet-eclipse.md` (Claude's state).

**Ownership:** the ownership tables in `AGENTS.md` still say `patch/` and `docs/` are Claude's. Before editing them, confirm with the owner that this period lets you. Claude asked the owner (see the end of this file). Until then, round 12 (`task-gemini-round12.md`) is research/test only and needs no `patch/` edits.

## Goal & Current Status

*Mega Man Battle Network 4: Violet Eclipse* (`plan/DEFINITIVE-EDITION.md`) is a ROM patch for both US ROMs (`patch/`, armips + TextPet). Last Claude commit: `3acd0f2`.

**Status in short (details in `patch/README.md`):**
- **PASS in-game:** WP-A2 Zenny, 12 Souls unite, Z-Saver BMD, Free Tournament, lobby rematches, Frequency Tuner, status pages 1–3, GP Power BMD, chip art.
- **Fixed in `4f5ec2b`, re-test pending (round 12 task 1):**
  - Duo/GP Power text (e-Reader threshold `0x1000`);
  - trash can and Duo's signal sprites (`list[7] 0x08`);
  - Duo rematch results (b1 = 0) and rewards via the results hook;
  - the battle-exit clear.
- **Open:** the Duo Soul unites but doesn't transform (round 12 task 2); the story-battle Duo grant (round 12 task 3).

## Key Decisions & Constraints

**Owner decisions** (all in `docs/pending/*.md`, `docs/completion/*.md`):
- Duo Soul = MetalSoul recolour, granted by beating Duo.
- Duo rematches in Meteor Comp 1, V1 → α → β → Ω on S ranks, then SP+.
- **SP+:** 4,095 HP, attack ×1.5, speed ×1.25. Reward random: **80% 15,000Z (`0x7A98`), 20% 50 BugFrags (`0xC032`)**.
- GP Power's description "Speedy / trio / attack!" approved.
- Lobby: 9 operators in Higsby's + 2 outside + trash can.
- Friendly matches: 3 per tournament, a win gives the Soul.
- **Chip art is Capcom art:** local only (`ref/chip-art/`, gitignored); `--public` builds leave it out.

**Hard rules:**
- **Never commit ROM bytes.** Copy existing data with `.incbin ROM_IN, addr - 0x08000000, size` (see `wp_d0_npc_lists.asm`).
- **Both versions:** every address that differs goes in `src/rs/version.asm` and `src/bm/version.asm`. Check in both ROMs before using one.
- **Free space:** allocate in `src/common/defines.asm`, in address order, each slot with a `.if . > SLOT + SIZE .error` guard.
  - Next free slot: **`0x087FB9E0`**.
  - Code/data must stay below `FS_TEXT` `0x087FC000` (TextPet uses `0x087FC000`+; the two chip archives sit at `0x087FC000`–`~0x087FD0C0`).
  - A 0x60 hole sits at `0x087FB670`.
- **Veneers:** free space is >4 MiB from most code, so a hook needs `bl` → an 8-byte veneer (`ldr r3,[pc,#0]; bx r3; .word target|1`, word-aligned) within ±4 MiB.
  - Used: the old Bass dead blocks in `code_03`, RS `0x08034A78`, `0x08000DB4`, RS `0x080C4788` / BM `0x080C4794` (battle exit).
  - **Only RS `0x080C4790` / BM `0x080C479C` is left.** Round 12 task 4 looks for more; SP+ (3 hooks) and the Duo palette (1) need them.
- **Thunk at a function start** (no veneer needed, any distance): `ldr r3,[pc,#n]; bx r3; .word target|1`. Mind alignment: at a halfword-aligned start, use `ldr r3,[pc,#4]`, pad, then the literal. Remember the thunk clobbers `r3`; if the function's first instruction is `push` that saves r3, keep the push first (see `wp_b4_gmd_tier.asm`).
- **ScriptGroup_FC table** (`lobby_fc_table` in `wp_d3_lobby_rematches.asm`):
  - `09` rematch, `0A` trash warp, `0B` Tuner, `0C` grant Soul (idx 0–12), `0D` Duo rematch.
  - **Next is `0E`** (friendly matches).
  - Handler convention: `r4` = script pointer, args at `[r4,2]`…; when done, advance `r4` and return `r0 = 1`; the battle-start handlers return `r0 = 0` and yield (see `lobby_fc_rematch`).
- **Lobby dialogue archive:** talk index `0xF0`–`0xFF` = entries `0x00`–`0x0F` (`0x0D` Duo prompt, `0x0E` Tuner, `0x0F` the shared "No" end).
  - For friendly matches, `0xE0`–`0xEF` are free (your scan).
  - Text is in-game encoding bytes there. New dialogue can instead be a TextPet `.tpl` in **`patch/src/scripts/new/`**, compiled by `build.py` and passed as `-strequ <NAME>_BIN`.
- **Text patches** of existing archives: `patch/src/scripts/text/*.tpl` with `@archive <NAME>` (names → offsets in `build.py` `VERSIONS[...]["text_archives"]`). **`*.tpl` is gitignored everywhere except `patch/src/scripts/`.**
- **Flags used** (all free per your scans):
  - lobby tiers `0x0E40`–`0x0E57`; Tuner `0x0E58`–`0x0E59`;
  - Duo tier `0x0E5A`–`0x0E5B`, Duo SP+ `0x0E5C`;
  - SP+ unlocks `0x0E5D`–`0x0E71` (your design); Duo chip given `0x0E72`;
  - Z-Saver `0x0E04`, GP Power `0x0D0B`; Duo Soul `0x0023`; Game Clear `0x0500` read only.
- **EWRAM bytes** `0x020015EC`–`0x020015EF`: roaming SP id / SP+ phase / SP+ active / rematch marker (`0x80 | id`; id 0–11 Navis, 13 Duo). Cleared on every battle exit.

**armips gotchas (all hit this week):**
- labels are **case-insensitive**: a label equal to an `equ` name breaks;
- no `.macro` inside a false `.if`;
- `.incbin` names are resolved even inside a false `.if`;
- comments can't contain a raw newline.

**Other gotchas:**
- Long bash heredocs with `'''` can break the shell: write Python to a file.
- Run `sed -i 's/\r$//'` if git warns about CRLF.
- From PowerShell, run WSL tools with the full venv path (`~/.venvs/mmbn4-decomp/bin/python`).
- capstone in WSL for disassembly checks: don't name a script `dis.py`.

## Modified & Relevant Files

- `patch/build.py`:
  - `python patch/build.py` (both versions, needs both US ROMs in `roms/`);
  - `--check` (identity test, must PASS);
  - `--public` (no chip art);
  - `--hello`.
  - Outputs: `build/bn4_{rs,bm}_overhaul.{gba,bps}`.
- `patch/src/main.asm` (include order), `src/common/defines.asm` (constants + allocation table), `src/{rs,bm}/version.asm`.
- `patch/README.md`: **update the package table and its status column for every change** (the owner reads it).
- `docs/handoff-violet-eclipse.md`: Claude's handoff. If you work in `patch/`, record your changes in your own `decomp/notes/handoff-*.md` as usual, and Claude will merge them into the docs.

## Open Issues / Blockers

- Duo Soul transform (round 12 task 2).
- Story-battle Duo grant (task 3).
- Veneer space (task 4).
- Final prop sprites (task 5).
- Not built yet:
  - **SP+** (`sp-plus-design-2.md`): blocked on veneers;
  - **the Duo palette** (`form-palette-hook.md`): blocked on the veneer at `0x080C4790` and on the transform;
  - **the Duo passive** (`duo-soul-passive.md` option A): needs the transform first;
  - **friendly matches** (`friendly-matches-design-2.md`, `FC 0E`).
- Owner questions: RS/BM version choice for the merged story (`DEFINITIVE-EDITION.md` §7 q1); key-item prices (round 12 task 7 gives the facts).

## Immediate Next Steps (suggested order)

1. Round 12 (`task-gemini-round12.md`), tasks 1–7.
2. If the owner lets you edit `patch/`, then, each followed by `python patch/build.py` + `--check`, a disassembly check in both builds, and an in-game test:
   1. the Duo Soul transform fix;
   2. the prop sprites;
   3. the story grant hook if task 3 needs one;
   4. SP+ (once veneers exist);
   5. the Duo palette + passive;
   6. friendly matches.
3. Commit each package on its own, staging by path, and don't push. Write your handoff at the end.

**Question Claude asked the owner (2026-11-23):** may Gemini edit `patch/` and `docs/` until Friday?
