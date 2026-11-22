# 07 — Testing & QA

## 1. Test save library (`test/saves/`)

Create save files (and mGBA save states) for both US versions at these points. Use them for fast regression testing:

| ID | Point in the game | Used by |
|---|---|---|
| S01 | New game, just after intro | A2, A5, B2 |
| S02 | Before the first tournament (Den) | B1, B9 |
| S03 | Before the WoodMan scenario (BM) | A1 |
| S04 | Before the ToyRobo slide | A6 |
| S05 | Before Eagle/Hawk tournament | B1 |
| S06 | Before RS/BM tournament | B1 |
| S07 | Before Duo | B8 |
| S08 | Post-game, Dark Karma active | B7 |
| S09 | Cleared save (NG+ trigger) | B5 |
| S10 | Two saves for link/Waiting Room tests | B6 |

Rule: the original save layout must stay compatible, or the patch must document that it breaks old saves. Recommended: keep it compatible, and store new variables in unused save bytes (found during RE-01).

## 2. Automated checks (mGBA Lua)

- **Boot smoke test:** boot the ROM headless, reach the title screen, load S01, walk N frames with no crash.
- **Data audits (Python, offline):** extract chip, GMD and shop tables from the built ROM and assert the expected values. For example: every GMD chip is reachable in some tier; Zenny values = original × 1.75.
- **Flag equivalence for skips (A5):** watch the cutscene vs. skip it, dump the flag region, diff. They must match.
- **Determinism (B1):** run the bracket reveal from S02 with different RNG seeds. The opponents must match.
- **Build determinism:** the same commit always gives the same output hash.

## 3. Manual playthrough tests

| Test | When | Platforms |
|---|---|---|
| Feature test of each WP | After each WP | mGBA |
| Full playthrough BM | Before each release | mGBA + one hardware route |
| Full playthrough RS | Before each release | mGBA |
| Hardware pass | Before releases from v1.0 | GBA (flashcart) if available, GBARunner3, open_agb_firm |

Use mGBA's fast-forward plus the test saves to keep full playthroughs to a reasonable time.

## 4. Bug tracking

- Solo project: a local `docs/bugs.md` table (ID, version bm/rs, WP, platform, status). Move to GitHub Issues only if the project goes public and gets players reporting bugs.
- Every bug report includes the patch build hash and, ideally, a save file.
