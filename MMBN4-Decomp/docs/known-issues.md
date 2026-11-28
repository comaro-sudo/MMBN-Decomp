# Violet Eclipse: Known Issues (2026-11-27)

Player-visible issues, limits, and items waiting on an owner decision across the patch and ports. Resolved items (Frequency Tuner soft-lock, status screen page 3 with 9–12 Souls) are excluded.

## Patch (`.bps`)

| Issue | Kind | Source |
|---|---|---|
| **Public builds (`--public`): Duo and Grand Prix Power have no chip picture.** Names and descriptions are present, but the picture/icon slots are left blank to avoid shipping Capcom art. | Waiting on owner | `docs/pending/public-chip-art.md`, `patch/README.md` |
| **Duo and Grand Prix Power do not appear in the Chip Library (all builds).** Both chips can be obtained and used in battle, but vanilla omits e-Reader chips from the Library and listing them safely requires new slot-table entries and updated completion-star count checks. | Waiting on owner / limit | `docs/pending/findings-2026-11-27.md` §1, `docs/tests/library-flags.md` |
| **Bass dual-chip drop names only one chip on the results screen.** Both versions' Bass chips are added to the pack; the second is added silently. | Limit | `patch/README.md` (WP-D3) |
| **Duo Soul reuses MetalSoul's status icon and sprite** (drawn with a custom Duo palette in battle). | Limit | `patch/README.md` (WP-D4) |
| **HP Memory total after the 40% price cut is 83,100Z** rather than an exact 83,000Z, because each of the 14 shop entries is rounded to 100Z. | Limit | `docs/pending/findings-2026-11-27.md` §4 |
| **Not yet tested on real GBA hardware or a full continuous playthrough.** Both `--public` `.bps` builds passed standalone mGBA 0.10.5 and automated PC-port smoke checks, but a start-to-postgame human playthrough and real cartridge/flashcart check remain open. | Testing gap | `docs/tests/mgba.md`, `docs/release-checklist.md` §3 |

## Ports (`ports/`)

| Issue | Kind | Source |
|---|---|---|
| **PET Status tab footer shows PC keyboard hints on Wii U and New 3DS** (`Arrows move X=A Z=B...` in `ports/common/pet_ui.c`). | Bug (low) | `docs/ports-3ds-review.md` |
| **PET Folder tab shows `--` for chip IDs >= 389** (`cid < 389` in `ports/common/pet_model.c`). | Bug (low) | `docs/ports-3ds-review.md` |
| **New 3DS port is untested on real hardware.** Requires `sdmc:/3ds/dspfirm.cdc` for audio (runs silent without it), and closing from the HOME menu does not force an immediate SRAM flush. | Limit | `docs/ports-3ds-review.md` |
