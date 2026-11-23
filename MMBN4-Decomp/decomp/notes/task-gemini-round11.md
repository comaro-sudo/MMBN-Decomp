# Gemini round 11: seven tasks (written by Claude, 2026-11-23, for when the owner assigns them)

Round 10 was very useful: the chip-text recipe, the Duo Soul framework and the rematch research are now built. Claude built these since round 10 (all in `patch/`, `--check` PASS, bytes or disassembly checked in both builds, **none tested in-game**):

| Commit | Package |
|---|---|
| `933f853` | Duo / GP Power chip art (`wp_d2_special_chip_art.asm`; private builds) |
| `c3a787f` | Trash can animation 0 (your round-10 fix) |
| `a16ea98` | Duo rematch in Meteor Comp 1 (`wp_d3_duo_rematch.asm`, **`FC 0D 00`**, talk index `0xFD`, flags `0x0E5A`–`0x0E5B`) |
| `24786cd` | GP Power Blue Mystery Data, ACDC Area 2 (`wp_d2_gpp_md.asm`, flag `0x0D0B`) |
| `8ba3c18` | Duo / GP Power names and descriptions (TextPet `--patch`, `patch/src/scripts/text/`) |
| `146b2a8` | **Duo Soul** (`wp_d4_duo_soul.asm`) + Duo deletion grant (`wp_d2_duo_story_grant.asm`: Duo Soul if not owned, Duo D once with guard flag `0x0E72`, then the original `0x006F`) |
| `653233d` | Status screen page 3 (R next, L previous) |

Rules as before: **don't edit `patch/`, `docs/` or `TODO.md`**; `decomp/` is yours (keep both ROMs byte-identical); test outputs outside the repo; stage by path, don't push; one handoff at the end. Every design gives exact RS and BM values.

Corrections to round-10 notes, for the record (`docs/pending/duo-soul.md`):
- `0x0800E1CA` isn't SuperArmor: it adds +30 to hand chips of class `0x0C`/`0x08` (GutsSoul).
- MetalFist's `[r7,#0xC]` word is `0x06050601`, not `0x0808`.
- `FC 0D` is now the Duo rematch. Friendly matches need **`FC 0E`**.
- Flag `0x0E72` is taken (Duo chip guard).
- Duo's `r5[0x04]` is its version (0 = V1 … 3 = Ω), so flag `0x006F` is never set by a V1 win.

## 1. In-game test of the seven packages above (`decomp/notes/patch-test-results-4.md`)
- **Chip art + text:**
  - Duo and GP Power in the Library, folder and pack: picture, icon, name and description;
  - and a `--public` build (art absent, names present).
- **GP Power BMD:** it shows at (106, 326) in ACDC Area 2, gives GP Power G once, and flag `0x0D0B` is set afterwards.
- **Duo story grant** (a save just before Duo, PT1 and PT3 if you have them): after the win, you have the Duo Soul (flag `0x0023`), Duo D is in the pack, flag `0x0E72` is set, and `0x006F` is set only for a harder Duo.
- **Duo rematch:**
  - the NPC appears only after Game Clear `0x0500`, and the spot and sprite are sane;
  - "Fight Duo?" starts the battle at the right tier;
  - the results screen appears;
  - an S rank climbs V1 → α → β → Ω;
  - β/Ω drop Duo D;
  - running away does no harm.
- **Duo Soul:**
  - Guard1–3, Barrier, HolyPanl, Anti*, LifeAura or Duo offer the unite, and it plays (no crash);
  - MetalSoul's look;
  - the charge shot hits for 180;
  - the Soul ends cleanly;
  - status page 3 (R / L).
- **Trash can:** the junk-pile sprite.
- **Stale marker** (`0x020015EF`): lose or flee a lobby rematch, then win a random battle with an S rank. Does that Navi's tier climb or a secret chip come? Does a game over / reload clear the byte?

## 2. Battle start / exit point for per-battle EWRAM bytes (`decomp/notes/battle-exit-hook.md`)
`0x020015EF` (rematch marker) and the coming SP+ flag `0x020015EE` must be cleared on **every** battle end: win, loss, flee, game over. Find one point that runs on every battle exit (or every battle start that isn't ours), with:
- the exact instruction bytes to replace;
- the live registers;
- a veneer spot in `bl` range.

The round-10 addresses `0x08005478` / `0x080082B0` aren't function starts: check them.

## 3. Duo palette: where MegaMan's form palette is loaded (`decomp/notes/form-palette-hook.md`)
For form 13 (drawn with MetalSoul's sprite via the `BattleGetFormSprite` thunk), find where the battle loads the form's OBJ palette, and how to substitute 32 bytes (`duo_soul_pal` from `duo-soul-design.md` §1) only for form 13. Duo's own init uses `BN4_SPRITE_PALETTE_SET` (`0x08002DEC`) with an index: is that the path for MegaMan too? Exact hook, RS and BM.

## 4. Duo Soul passive: verified options (`decomp/notes/duo-soul-passive.md`)
List what each existing `BN4_FORM_ENTERS` routine really does (disassembly-checked, with the helper each calls). Then propose 2–3 Duo passives that reuse verified helpers and work with MetalSoul's sprite. For each:
- the exact code;
- how the effect is removed when the Soul ends.

## 5. SP+ gaps (`decomp/notes/sp-plus-design-2.md`)
Complete `sp-plus-design.md`:
- **Hook 1:** the full roaming-SP table scan (no `...`).
- **Hook 3 (speed ×1.25):** exact code at `BN4_OBJECT_RUNNER` (`0x08003194`), the per-frame phase counter, and the side/alive checks, with register liveness.
- **Claude's hook-site changes (please confirm):**
  - **HP:** `bl` replaces `ldrh r1,[r7]; adds r2,r1,#0` at `0x08006802`, returning `r1` raw and `r2 = r1 | 0x0FFF` when SP+ is active;
  - **attack:** hook `bl sub_08013410` at `0x08013A5A` and scale `[r6,#0x28]` *after* it, since damage is read only after both calls;
  - **veneers:** the 20 dead bytes after Duo's deletion thunk (RS `0x080C4788` / BM `0x080C4794`).
- **Duo:** its SP+ step uses flag `0x0E5C` ("Duo SP+ unlocked").
- **Reward (owner, 2026-11-23, revised): random, 80% 15,000 Zenny (`0x7A98`), 20% 50 BugFrags (`0xC032`).** (Replaces 50/50.) Give the exact code at the reward point: where `BN4_BATTLE_REWARD` is written for the results screen, and which RNG to use (`GET_RNG` `0x08001B16`, as the random-drops patch does).

## 6. Friendly matches, updated for FC 0E (`decomp/notes/friendly-matches-design-2.md`)
Re-issue `friendly-matches-design.md` with `FC 0E`.
- The talk indices `0xE0`–`0xE8`: give the exact change to the lobby's `OpenMapDialogue` redirect (today: `≥ 0xF0` → the lobby archive). Are `0xE0`–`0xEF` ever used by a vanilla map?
- A second archive, or the lobby archive grown to 32 entries?
- How each friendly match's dialogue `.tpl` (compiled by `build.py` from `patch/src/scripts/new/`) is reached.

## 7. Names
Merge the round-10 proposed names (`duo-soul-design.md` §8, the others), correcting `BattleSetSuperArmor` from task 4's findings. Full regeneration, both compares.
