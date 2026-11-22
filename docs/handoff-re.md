# Handoff: Claude's reverse-engineering lane (2026-11-22)

## Goal & Current Status

- **Goal:** map the game systems that *Violet Eclipse* depends on: save, flags, playthrough tier, Souls, economy, tournaments, encounters, Mystery Data.
  - The work is reverse engineering only. It reads `decomp/asm/` without editing it (see `AGENTS.md`, "Claude's reverse-engineering lane").
  - Findings go in `docs/re/`. Proposed symbol names go in `docs/re/proposed-names.txt`, for Gemini to merge into `decomp/symbols/names.txt`.
- **Done** (commits `31e92a0`, `cf318b4`, `f28d5f0`, plus the commit that adds this file):
  - The save format is decoded and **verified on real RS and BM saves**: relocation, XOR scrambling, checksum `+0x16` in Red Sun and `+0x22` in Blue Moon.
  - The event-flag routines are mapped.
  - The playthrough counter is at progress `+0x0D`; the scan found 33 readers.
  - The Soul flag table: both ROMs have the 12-Soul code path, and the other version's six Souls are blocked only by `0xFF` table entries.
  - Library flags are `0x16C0 + chip id` (verified at runtime).
  - Zenny is at game state `+0x74`, BugFrags at `+0x78`; both have anti-cheat shadows (verified).
  - The folder slot is identified.
  - The WoodMan fix mechanism is understood and confirmed in both ROMs.
  - The PC port's `--test` mode gained `--dump`, `--state-in` and `--state-out`.
- **Main document:** [`docs/re/save-and-flags.md`](re/save-and-flags.md). Each claim is marked V (code read), R (confirmed at runtime) or T (tentative).

## Key Decisions & Constraints

- The owner runs Gemini only when Claude's usage limit is reached, so the sessions are sequential. Ownership is unchanged: Gemini owns `decomp/` and `TODO.md`, Claude owns `plan/`, `docs/`, `patch/`, `ports/`.
- Never commit ROM data. The throwaway scripts read `roms/*.gba` and RAM dumps locally; only addresses, offsets and constants go in the docs.
- Runtime outputs go in `build/re/` (gitignored): states `t*.ss` (RS) and `b*.ss` (BM), dumps `*.ewram`, and saves `t1.sav` and `b.sav`.
  - `build/re/t12.ss` is Red Sun just after the first save: PET obtained, in Lan's room.
  - Jack-in is still story-locked there ("Let's do it later!").
- Gemini's `names.txt` labels `0x0800045E` as `HBlankIntr`, which looks wrong (it's boot or soft-reset init). It's flagged in `proposed-names.txt`.

## Modified & Relevant Files

- `docs/re/save-and-flags.md`: RE-01, RE-02 and RE-03, the first half of RE-05, and the economy section §4b.
- `docs/re/proposed-names.txt`: about 59 proposed names. Gemini merges them and deletes the merged lines.
- `ports/pc/src/main.c`, `ports/pc/README.md`: the test-mode options.
- `AGENTS.md`: the RE lane paragraph.
- Not in the repo: the throwaway scripts in Claude's scratchpad (`run.py`, `savdec.py`, `fdiff.py`, `chk.py`, `tkfields.py`). Their logic is described in the note; recreate them if needed.

## Open Issues / Blockers

- Flags `0x17`–`0x22` haven't been seen getting set by a Soul event. That needs a late-game state.
- The Navi-stat shadow (`+0x98`) and the `+0x94` shadows don't match on a new game, so it's unknown when they're written.
- Unnamed save slots: `+0x50` (`0xF04` bytes, probably the pack), `+0x54`, `+0x58` (about 440 8-byte records), `+0x5C`–`+0x74`, `+0x80`.
- Flag ranges `0x18C0` and `0x1AC0`: chip-related, meaning unknown.
- The battle-reward caller (`sub_08021684`) is identified from code; it hasn't been confirmed in a real battle yet.

## Immediate Next Steps

0b. **Confirmed both ways:** WoodSoul unites in Red Sun and SearchSoul in Blue Moon with `wp_d4_twelve_souls.asm` (in both builds). See `docs/re/soul-merge-experiment.md`. WP-A1, the WoodMan fix, is integrated and matches the official IPS (`5f61730`). Next: the grant without a trade (open: Blue Moon's key storage), then WP-A2 once the owner decides how Zenny scaling works (fixed or an in-game option).
0a. **13 community saves** (`ref/saves/`, local) decode with `python patch/tools/savetool.py`: [`docs/re/community-saves.md`](re/community-saves.md). RE-04 is half-mapped in [`docs/re/tournaments.md`](re/tournaments.md). The key lead: Red Sun can already field Blue Moon's Navis. Test it: 12-entry Soul flag table + save `22565` + unite ProtoSoul.

0. The content inventory has started (commit `2938b52`): run `python patch/tools/inventory.py`; the conclusions are in `docs/completion/`. Its next step is RE-07/RE-09 (the ROM's own GMD, Mystery Data, shop and trader tables), which replace the guide- and map-based sources.

1. RE-08: the battle-reward grant is `sub_08021684` (§4b of the note). Still to do: find who fills the reward word at `[obj+0x10]` from `BN4_REWARD_ROWS`, which sets the Busting Level row, and the "no reward" rule for scripted battles. Confirm it in a real battle once the story allows jack-in.
2. New Game+ and completion stars are mapped in [`docs/re/completion-and-new-game-plus.md`](re/completion-and-new-game-plus.md). Leftover debug scripts show how to grant any Soul (flag plus pseudo-chip key). The story's own Soul grant is still not found; the best route is a runtime watch on the Soul flag byte from a late-game state.
3. (old step 2) RE-05, second half: find the event-script opcode that sets a flag (TextPet script commands) and the scripts that use flags `0x17`–`0x22`.
3. RE-04: follow `BN4_START_TOURNAMENT`'s pool fill (`0x08043FE4`, `0x08044058`, `0x0804422C`, `0x0804428C`) to the RNG.
4. Name the remaining save slots: use the PC port test mode to diff saves around actions once the story allows jack-in. An alternative is reading the PET screens' code.
5. Then WP-A1 (the WoodMan fix in `patch/`) and WP-A2 (Zenny), using these findings.
