# Handoff: Claude, Violet Eclipse patch, ports and docs (updated 2026-11-27)

Claude's handoff for `patch/`, `ports/`, `plan/` and `docs/`. Paths are inside `MMBN4-Decomp/` unless they start with `/`. Read with it:
- `TODO.md` (Gemini's tracker, the ordered backlog);
- `patch/README.md` (every package and its test status);
- `decomp/notes/handoff-2026-11-26-gemini-decomp.md` (the decomp).

## Goal & Current Status

**Goal:** *Mega Man Battle Network 4: Violet Eclipse* (`plan/DEFINITIVE-EDITION.md`; the owner confirmed the name on 2026-11-25): every Soul, chip and boss in one save. First as a ROM patch (`patch/`), then on the ports.

**Repo:** `main` = `origin/main`. Verified 2026-11-27 by Claude:
- `python patch/build.py` and `--check` PASS for both versions;
- the decomp `make -B compare` is OK for both RS and BM.

**Ownership:** from 2026-11-23 to Friday 2026-11-27, Gemini led and could edit `patch/` and `docs/` (`AGENTS.md` §1). **After Friday, the normal tables apply again:** Claude owns `patch/`, `docs/`, `plan/`, `ports/`; Gemini owns `decomp/` and `TODO.md`.

**Patch (Phase 1):** every package PASSes in-game (`patch/README.md`, `decomp/notes/patch-test-results-6.md`). The automated C1–C5 matrix is 16/16 on RS and BM (`patch-test-results-7-c1-c5.md`). What's in it:
- **Economy:** WoodMan fix; battle Zenny ×1.5; key-item and upgrade prices at 40% (option 2), with existing saves synced on load; random RS/BM drops; Bass gives both chips.
- **Souls:** all 12 + the **Duo Soul**: unite, MetalSoul sprite with a Duo palette, AirShoes/FloatShoes + Reflect passive, a 3-page status screen. The completion star needs 12.
- **Tournaments:** fixed canon pairs; the Free Tournament fields all 12 Soul Navis; **9 friendly matches** (`FC 0E`).
- **Rematches:** a lobby with 11 operators and the trash can to JunkMan, climbing V1 → V2 → V3 → SP → **SP+**. **Duo rematches** in Meteor Comp 1 climb V1 → α → β → Ω → SP+ on S ranks. The Duo story/rematch grant gives the Duo Soul and Duo D.
- **SP+:** 4,095 HP, ATK ×1.5, speed ×1.25, reward 80% 15,000Z / 20% 50 BugFrags.
- **Chips & Mystery Data:**
  - Z-Saver and GP Power Blue Mystery Data;
  - Duo and GP Power names and descriptions, plus local chip art (`--public` leaves it out);
  - the Frequency Tuner, and GMDs that follow it.
- **Safety & infrastructure:** the Waiting Room slot guard; lazy LZ77 NPC sprites; the battle-exit clear.

**Ports:**
- **PC:** EL-0 to PC-6 done (`ports/common/`, the SDL3 frontend, ARM64 + x64 zips).
- **Wii U:** WU-1 to WU-6 verified in Cemu.
- **New 3DS:** source written (`ports/3ds/`); not compiled yet (needs devkitPro `3ds-dev`).

**Owner decisions recorded since 2026-11-24:**
- RS/BM = **Option C**: two complete patches, RS and BM;
- hardware `D-EL1` = Wii U, New 3DS, DSi;
- ports licence `D-EL3` = GPL-3.0;
- Principia Trilogy later;
- Tier A leftovers saved for later (`docs/pending/tier-a-leftovers-and-qol.md`).

## Key Decisions & Constraints

- **Owner decisions:** `docs/pending/*.md` (latest: `owner-decisions-2026-11-24.md`), `docs/completion/*.md`, `plan/00-decisions-and-feasibility.md`.
- **Patch conventions:** see `decomp/notes/handoff-claude-to-gemini-2026-11-23.md`.
  - No ROM bytes; both versions.
  - A free-space slot with an overrun guard in `defines.asm`.
  - Veneers in the verified dead pools `sub_0800A144` and `sub_08001784`.
  - `FS_TEXT` is now `0x087FD000`.
  - The FC commands are `09`–`0E`.
  - armips labels are case-insensitive.
  - `*.tpl` only under `patch/src/scripts/`.
- **Chip art is Capcom art:** local only (`ref/chip-art/`); public builds use `--public` until placeholder art is decided.
- **Decomp metrics:** only the 63 functions in `decomp/src/*.c` are matching decompilation. Most of `decomp/src/modern/` (247 functions) is re-implemented game logic that isn't traced to ROM routines (the provenance note in Gemini's 2026-11-26 handoff), so count it as port scaffolding, not decomp progress.

## Modified & Relevant Files

- `patch/`: see `patch/README.md` (its package table is the reference). Round-13 commits: `4de9524` (prices), `71d149e` (Waiting Room guard), `acf65ad` (Duo S-rank climb), `a6cf221` (results-screen fix), `ede0993` (lazy sprites, existing-save prices).
- `ports/common/`, `ports/pc/`, `ports/wiiu/`, `ports/3ds/`: see each folder's README and TODO "Done".
- `docs/pending/`: open and recorded owner decisions.

## Open Issues / Blockers

- **Owner:**
  - placeholder art for `--public`;
  - the GBARunner licence for DSi/NDS (`D-EL5`, ask Gericom);
  - Tier A leftovers (later).
- **Needs the owner's hardware or setup:**
  - a real Wii U `bench.log` (gate G-WU);
  - devkitPro `3ds-dev` for the `.3dsx` (`apt.devkitpro.org` blocks CLI installs).
- **Not yet done:** a full continuous human playthrough (C1–C5) on the PC port, RS and BM.

## Immediate Next Steps

1. Full playthrough testing on the PC port (TODO step 3).
2. Ports: the 3DS build once devkitPro is installed; the Wii U hardware bench; DSi after `D-EL5`.
3. Public-release prep: placeholder Duo/GP Power art, a release checklist, the patch README for players.
4. Decomp (Gemini): Phase A, and matching C traced to ROM addresses.
