# Handoff: Claude, decomp + Violet Eclipse design and patches (2026-11-22/23)

Replaces `docs/handoff-re.md` as Claude's current handoff (that file covers the earlier RE lane, 2026-11-22). Paths are inside `MMBN4-Decomp/` unless they start with `/`.

## Goal & Current Status

**Repo.** `C:\proyectos\MMBN-Decomp` is now a collection of MMBN decomps (`AGENTS.md` §0). BN4 lives in `MMBN4-Decomp/`; BN1, 2, 3, 5 and 6 are planned.
- Remote: `origin` = `github.com/comaro-sudo/MMBN-Decomp`. **Only up to `6e47d43` is pushed.** Everything later is local; the owner hasn't asked for a push.
- Ownership: Gemini owns `decomp/` and `TODO.md`, and Claude may also work in `decomp/`. Claude owns every new game folder by default.

**Decomp (verified: both ROMs `make -B compare` OK at every commit below):**
- **DC-4 met:** RS and BM build from one shared `asm/` tree (`a5ed4bb`). Symbols are named after RS addresses; version differences are `.if VERSION_BM` blocks written by `tools/merge_bm.py`.
- **Segment split** at the true RS/BM shift points (`36a6b5a`): `code_04` became `code_04`/`04b`/`04c`/`04d`, and the `code_09`/`code_10` boundary moved.
- **Code- and data-region pointers are symbols**; BM data addresses map through Gemini's `symbols/datashift_rs_bm.txt` (`1fd88fd`).
- Gemini then folded the version constants into `include/macros.inc` (`b5b0873`).
- **DC-2 table carving:**
  - the encounter tables (`db01bda`);
  - 4 big pointer tables (`1c8fca7`);
  - 204 small tables that are 90%+ pointers (`43ad295`).
  - **Coverage went 50.8% → 59.4%.**
- **Uncommitted at the time of writing:** 14 new names appended to `symbols/names.txt` (Gemini's round-2 names + `ChipGive`), with a full regeneration running. Commit after both compares are OK.

**Violet Eclipse patch** (`patch/`, see `patch/README.md`; built and `--check`-passing; disassembly-checked):

| Package | Status |
|---|---|
| WP-A1 WoodMan fix | Matches the official fix |
| WP-A2 battle Zenny × 1.5 | In-game check pending |
| WP-D4 all 12 Souls unite | Confirmed in-game |
| WP-D3 Bass gives both chips (`8cb8da8`) | Not tested in-game |
| WP-D4 Souls star needs 12 (`fae06e1`) | Not tested in-game |
| WP-D2 random RS/BM drops (`f57407d`) | Not tested in-game |

Building now **needs both ROMs**: the other version's drop table is copied at build time.

**RE written this session (`docs/re/`, `docs/completion/`):**
- shops: stock is copied into the save at new game, prices are × 100, which shop is which, the Number Trader;
- free space: 35.9 KiB at `0x087F739C`–`0x08800000`;
- the Mystery Data flag correction (it's `0x0D00`–`0x10FF`);
- all save slots named;
- bosses from the ROM: every Navi is in both ROMs, and only 64 drop rows differ;
- the 36 secret chips come from the Free Tournament.

## Key Decisions & Constraints (owner)

- **Drops:** a drop that differs by version is random, 50/50 per drop (implemented).
- **Bass:** the order is: drop both chips (implemented) → one chip + an Unlocker with a purple Mystery Data nearby → a blue + a purple Mystery Data → roaming. BassΩ and BassXX already roam forever (Gemini).
- **Lobby rematches** (`docs/pending/free-tournament-secret-chips.md`):
  - the 12 Soul Navis' operators stand in the Free Tournament lobby, and appear after Lan meets their Navi (fallback: unmet ones appear in the post-game);
  - each Navi climbs V1 → V2 → V3 → SP with S-rank wins, in the lobby only;
  - JunkMan has no operator: Lan jacks into a **trash can** (an existing one in or near the lobby; otherwise add one to the lobby). His overworld sprite is in both ROMs.
- **Tournaments:** one canon story fight + 3 friendly matches per tournament (the lineup is in `decomp/notes/canon-navis.md` §6).
  - The friendly matches all happen before the final, 2 or 3 of them before the canon fight.
  - Winning one gives that Navi's Soul; it can be retried, and it counts as meeting the Navi.
- **Never commit ROM bytes.** Only carve tables that are mostly pointers; mixed data stays `incrom`.
- Gotcha: from PowerShell, write `~/.venvs/mmbn4-decomp/bin/python` out in full in `bash -lc '...'`; a `$P` variable gets eaten. Double quotes in commit messages break PowerShell 5.1's argument passing.

## Modified & Relevant Files

- `patch/build.py`: `other_reward_rows()`.
- `patch/src/main.asm`, `src/{rs,bm}/version.asm`, and `src/common/defines.asm`, which holds the free-space allocation table: next free slot `0x087F9F40`.
- Three new packages: `src/common/wp_d3_bass_both_chips.asm`, `wp_d4_star_twelve_souls.asm`, `wp_d2_random_version_drops.asm`.
- Both dead 86-byte blocks in `code_03` (RS `0x08037C58`, `0x08037D30`) are now used by patches.
- `docs/pending/free-tournament-secret-chips.md`: the open owner decisions.
- `docs/completion/bosses.md`, `chips.md` and `mystery-data-shops-traders.md`; `docs/re/shops.md`, `free-space.md`, `save-and-flags.md` and `tournaments.md`.
- Gemini's notes this session, in `decomp/notes/`:
  - `dc4-version-differences.md`, `data-region-shift-map.md`;
  - `re07-mystery-data.md`, `re06-encounters.md`, `re04-tournaments-remainder.md`;
  - `save-slots.md`, `bass-rematch.md`, `soul-grant.md`;
  - `shops-in-the-world.md`, `canon-navis.md`.

## Open Issues / Blockers

- **In-game testing** of WP-A2, WP-D3, WP-D4 star and WP-D2 (mGBA, or the PC port's `--test`).
  - Done so far: a **boot smoke test** (2026-11-23, PC port `--test 1500`, START then A). The patched RS and BM runs match their originals: the same final frame, the same audio sample count, and in RS the same RNG state.
  - That only shows boot isn't affected. The packages act in battles and on menus, so they need test saves at the right point (e.g. `ref/saves/` converted with `patch/tools/savetool.py`, plus `--state-in`).
- **The status screen** lists only 6 Souls. Gemini confirmed at runtime that a 2 × 6 grid isn't possible: the NAVIGATOR box starts at Y = 94. The plan is **two pages of 6**; Gemini's round-5 task 3 designs it, and Claude writes the patch.
- **WP-D4 star:** a runtime bug was found by Gemini and fixed (`79398a4`: the `cmp r5, #6` at `0x08000D56` is now 12). Re-test pending (Gemini round 5).
- **Lobby plan:**
  - the waiting room (option A, preferred) is being researched by Gemini (round 5, task 1);
  - fallback B is inside and outside Higsby's;
  - JunkMan's trash can is to be added **outside Higsby's entrance**: BN4 has no trash-can comp, and Higsby's has no jack-in objects (Gemini round 4).
- **Owner decisions still open:**
  - whether to use the Free Tournament fix at all (pending doc);
  - the RS/BM version choice in the merged ROM (`DEFINITIVE-EDITION.md` §7 q1; the canon lineup assumes one merged story);
  - sources for Z-Saver, Grand Prix Power and Duo (the Number Trader is the natural place);
  - key-item prices (WP-A3).
- `TODO.md` (Gemini's) doesn't list this session's patches yet.

## Immediate Next Steps

1. When the regeneration finishes: both compares, then commit the 14 names (`symbols/names.txt` + `asm/`).
2. Read Gemini's round-3 notes when they land:
   - `decomp/notes/lobby-npcs.md` (the lobby map, NPC format, a trash can);
   - `free-tournament-battle-data.md` (does the battle need the traded stats and AI?);
   - `status-screen-souls.md`.

   Then design the lobby-rematch and status-screen patches.
3. Test the four untested packages in mGBA. If WP-D2's extra RNG draw causes visible side effects, consider using a frame counter instead.
4. Ask the owner whether to push to GitHub.
