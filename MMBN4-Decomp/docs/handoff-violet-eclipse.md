# Handoff: Claude, decomp + Violet Eclipse design and patches (2026-11-22/23)

> **Latest state (2026-11-23, later):**
> - `TODO.md` holds the current ordered backlog, and `patch/README.md` the status of every package. Read those first.
> - Since this file was written:
>   - Gemini's patch sprint (owner-approved, in `patch/`) added the Z-Saver Mystery Data, WP-B1 fixed tournaments, the Free Tournament fix, the lobby rematches with JunkMan's trash can, and the Frequency Tuner. Claude reviewed it: builds pass, and the flags don't collide.
>   - Claude added `FC 0C <idx>` (grant any Soul), the NPC lists in free space and the Duo drop rows. The Duo rows have **no effect**: the fight skips the results screen, and the chip is blank in the US ROMs.
>   - **The TextPet text pipeline works and is wired into `build.py`** (`plan/text-insertion.md`).
>   - Names were merged through Gemini's round 7.
> - Owner decisions since:
>   - the Black Earth gate keeps 6 Souls;
>   - Z-Saver in a Blue Mystery Data (done), Grand Prix Power in ACDC Area 2; Duo from Duo's highest tier;
>   - the lobby is 9 operators inside Higsby's and 2 outside with the trash can; no waiting room;
>   - **SP+**: a 5th lobby step, 4,095 HP, attack × 1.5, speed × 1.25, a big Zenny/BugFrag reward; shared Navis get it via their roaming SP ghost (`docs/pending/boss-double-hp.md`);
>   - the MMBN Principia Trilogy plan (BN1–3 merged, ports only).
> - Gemini round 8 (in progress): lobby fixes (sprites, the "met" flags, JunkMan's map), SP+ attack/speed research, Duo and Grand Prix Power art from the JP ROM.
> - Next for Claude: the friendly-match dialogues as TextPet scripts with `FC 0C`, once the tournament-cutscene hook is known; then SP+ and Duo/Grand Prix Power from Gemini's research.

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

## Status-screen two-page patch: implementation plan (checked by Claude, 2026-11-23, V)

Based on Gemini's `decomp/notes/status-screen-two-pages.md`, with these corrections and checks:
- **The per-frame stub call is at RS `0x08033674` / BM `0x08033678`** (`bl sub_08034EB0`), not `0x0803367A`. The next call, `sub_08034E64`, doesn't read `r1`–`r3`, so a veneer that clobbers `r3` is safe. The toggle must keep `r4`–`r7` (`r5` = status state).
- **Each of the four old tables has exactly one reference**, so moving them frees their bytes:
  - status list RS `0x08034A78` (8 bytes, ref `0x08034A6C`);
  - icon flags, anims and pals RS `0x0810DAB8` / `DAC5` / `DACB` (refs `0x0810DA78` / `DA88` / `DA8C`; about 28 free bytes up to `0x0810DAD3`).
  - BM addresses: `+4` in `code_02`, `+0x14` in `code_10`.
- **Placement** (the free space at `0x087F9F40+` is more than 4 MiB from the hooks, out of `bl` range):
  - `StatusSoulLoadFlag` (about 14 bytes + 1 literal) goes **directly into the freed `0x0810DAB8` bytes**, within `bl` range of `StatusSoulIconInit`;
  - a page-toggle veneer goes in the freed 8 bytes at `0x08034A78`: `ldr r3,[pc]; bx r3; .word Toggle|1`, with `StatusSoulPageToggle` (~96 bytes) in free space, returning with `pop {r4-r7, pc}`;
  - an init-page veneer goes in the old star list at `0x08000DB4` (14 bytes, unreferenced since WP-D4), pointing at an `InitPage` in free space that clears `0x0201D018` and tail-jumps (`bx`) to `sub_08053F8C` with `lr` intact.
- **The new tables (same in both versions):** flags `17..22 FF`, anims `00..0B`, pals `00 01 01 01 00 00 00 01 02 00 02 01`. RS's own were anims `00..05` / pals `000101010000`; BM's anims `06..0B` / pals `000102000201`.

## Immediate Next Steps

1. When the regeneration finishes: both compares, then commit the 14 names (`symbols/names.txt` + `asm/`).
2. Read Gemini's round-3 notes when they land:
   - `decomp/notes/lobby-npcs.md` (the lobby map, NPC format, a trash can);
   - `free-tournament-battle-data.md` (does the battle need the traded stats and AI?);
   - `status-screen-souls.md`.

   Then design the lobby-rematch and status-screen patches.
3. Test the four untested packages in mGBA. If WP-D2's extra RNG draw causes visible side effects, consider using a frame counter instead.
4. Ask the owner whether to push to GitHub.
