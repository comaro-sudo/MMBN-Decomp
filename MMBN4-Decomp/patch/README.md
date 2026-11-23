# Violet Eclipse ROM patch (Phase 1)

armips patches that turn the player's own *Mega Man Battle Network 4* ROMs into *Violet Eclipse*. The goal is in [`../plan/DEFINITIVE-EDITION.md`](../plan/DEFINITIVE-EDITION.md); the work packages are in [`../plan/03-phase1-rom-overhaul.md`](../plan/03-phase1-rom-overhaul.md).

**No ROM data is in this folder.** Everything is built from the player's dumps, and the outputs go to the gitignored `../build/`.

## Build

From `MMBN4-Decomp/`:

```powershell
python patch/build.py            # both versions: build/bn4_{rs,bm}_overhaul.gba + .bps
python patch/build.py rs         # one version
python patch/build.py --check    # identity tests (chip table re-assembles byte-identical)
python patch/build.py --hello    # test patch (Cannon 40 -> 99) instead of the overhaul
```

Needs:
- **Both US ROMs** in `../roms/`, `bn4_rs_us.gba` and `bn4_bm_us.gba`, which are hash-checked. The random-drop package copies the other version's drop table from the player's other ROM at build time, so building either version needs both.
- `tools/armips/armips.exe` and `tools/flips/` ([`../docs/reference/toolchain.md`](../docs/reference/toolchain.md)).

The pipeline: verify the ROM hashes, run armips, fix the header checksum, then create the `.bps` with flips and check that it round-trips.

## What's in it

| File (`src/common/`) | Package | What it does | Status |
|---|---|---|---|
| `wp_a1_woodman_fix.asm` | WP-A1 | WoodMan scenario fix (Prof. 9's), byte-identical to his IPS | Built; matches the official fix |
| `wp_a2_zenny_battle.asm` | WP-A2 | Battle Zenny × 1.5; the shown and granted amounts match | **PASS in-game** (Gemini, PC port: 50 Z → 75 Z, shown and granted) |
| `wp_d4_twelve_souls.asm` | WP-D4 | All 12 Double Souls can unite in both versions | Built; confirmed in-game |
| `wp_d3_bass_both_chips.asm` | WP-D3 | A Bass battle also grants the other version's chip (Bass ↔ BassAnly, BlakBarr ↔ DeltaRay) | **PASS in-game** (Bass V1 and BassΩ: both Library flags set, both pack counts +1) |
| `wp_d4_star_twelve_souls.asm` | WP-D4 | The Souls completion star needs all 12 Souls (12-entry list, and the count compare raised to 12) | **PASS in-game after the fix** (`79398a4`): 6 Souls → no star, 12 → star, RS and BM |
| `wp_d2_random_version_drops.asm` | WP-D2 | Drops that differ by version come from either version's table, 50/50 | **PASS in-game** (Billy on BM, 30 seeds: vanilla 6 P / 0 L; patched 4 P / 4 L) |
| `wp_d2_duo_chip_reward.asm` | WP-D2 | Duoβ and DuoΩ (the highest tiers) drop the Duo Giga chip in every slot, in both drop tables | **No effect yet:** Gemini found that the Duo fights skip the results screen, and the Duo chip has blank name and art in the US ROMs. Harmless; to be redone (art and name from the player's JP ROM, plus a post-Duo grant) |
| `wp_d2_zsaver_md.asm` | WP-D2 | Z-Saver Z in a Blue Mystery Data in an Undernet 1 dead end (flag `0x0E04`) | **PASS in-game** (Gemini, `7c0f9b8`) |
| `wp_b1_tournaments.asm` | WP-B1 | Fixed tournaments: canon Soul Navi pairs (AquaMan, Roll, ProtoMan), no pair coin flip or playthrough-2 swap, deterministic bracket; RS's Den tutorial gives AquaSoul | Bytes verified in RS and BM, smoke-tested (Gemini, `bac8ce9`) |
| `wp_d4_free_tournament.asm` | WP-D4 | The Free Tournament fields the 12 Soul Navis without trades (missing chips first; type-0x01 battles), so the 36 secret chips are obtainable | **PASS in-game** (7 Soul Navis with no traded Navis; Gemini, `c9d36cf`) |
| `wp_d3_lobby_rematches.asm` | WP-D3 | 9 operators in Higsby's, 2 + JunkMan's trash can on the ACDC street; rematches climb V1 → V2 → V3 → SP on S ranks (tier flags `0x0E40`–`0x0E57`); the trash can warps to JunkMan in `WtrGodComp1 (8E, 00)` | **PASS in-game** (Gemini, `ce9d481`). 7 operators still use placeholder sprites. The "met" gate uses flags `0x0001 + id` (T: confirm their meaning) |
| `wp_d4_grant_soul.asm` | WP-D4 | Script command **`FC 0C <idx>`** grants any of the 12 Souls in either version (flag `0x15` + flag `0x17 + idx` + `WriteChipKey(0x161 + idx)`); for the friendly matches | Built, disassembly-checked; not yet called by any script |
| `wp_b2_frequency_tuner.asm` | WP-B2 | Frequency Tuner: an NPC in Higsby's cycles the virus tier Auto / V1 / V2 / V3 (flags `0x0E58`–`0x0E59`), read by `BN4_ID_RAISE` | Toggle tested in-game (Gemini, `ec5e084`) |
| `wp_d0_npc_lists.asm` | infrastructure | Higsby's and ACDC Town's NPC lists move to free space, with room for 16 | Built; byte-identical copies; boots |
| `wp_d4_status_two_pages.asm` | WP-D4 | The status screen shows all 12 Souls on two pages of 6 (L or R switches, once 7+ are owned) | Built, disassembly-checked, boots; **not yet tested on the status screen** |

Test evidence: `../decomp/notes/patch-test-results.md` (Gemini, 2026-11-23). The owner's decisions behind them are in [`../docs/completion/bosses.md`](../docs/completion/bosses.md) and `../plan/DEFINITIVE-EDITION.md` §7. The ones still pending are in [`../docs/pending/`](../docs/pending/).

## Layout

| Path | Contents |
|---|---|
| `src/main.asm` | Entry point: includes the version file, the defines, then every package |
| `src/rs/version.asm`, `src/bm/version.asm` | Per-version addresses (`VER_RS` / `VER_BM`) |
| `src/common/defines.asm` | Version-agnostic constants, and the **free-space allocation table** |
| `src/common/wp_*.asm` | One file per work package |
| `test/` | The identity test and the hello patch |
| `tools/` | `inventory.py` (completion inventory) and `savetool.py` (save decode/convert) |

## Rules for adding a package

- Find each address in **both** ROMs, and put it in both `version.asm` files.
- New code or data goes into one of two places:
  - **free space** (`docs/re/free-space.md`): take the next slot from the allocation table in `defines.asm`, and add an `.error` guard for the slot's size;
  - **verified dead code**: so far the two 86-byte blocks in `code_03`, both now used.
- Check the emitted bytes (disassemble the built ROM), and run `--check` before committing.
- Data from the player's *other* ROM is read at build time only (as `build.py` does for the drop table). Never commit ROM bytes.
