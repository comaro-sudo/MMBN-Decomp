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
| `wp_a2_zenny_battle.asm` | WP-A2 | Battle Zenny × 1.5; the shown and granted amounts match | Built; in-game check pending |
| `wp_d4_twelve_souls.asm` | WP-D4 | All 12 Double Souls can unite in both versions | Built; confirmed in-game |
| `wp_d3_bass_both_chips.asm` | WP-D3 | A Bass battle also grants the other version's chip (Bass ↔ BassAnly, BlakBarr ↔ DeltaRay) | Built, disassembly-checked; not tested in-game |
| `wp_d4_star_twelve_souls.asm` | WP-D4 | The Souls completion star needs all 12 Souls | Built, checked; not tested in-game |
| `wp_d2_random_version_drops.asm` | WP-D2 | Drops that differ by version come from either version's table, 50/50 | Built, disassembly-checked; not tested in-game |

The owner's decisions behind them are in [`../docs/completion/bosses.md`](../docs/completion/bosses.md) and `../plan/DEFINITIVE-EDITION.md` §7. The ones still pending are in [`../docs/pending/`](../docs/pending/).

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
