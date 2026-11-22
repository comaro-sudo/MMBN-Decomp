# Violet Eclipse content inventory (WP-D1)

This folder answers the question in [`plan/DEFINITIVE-EDITION.md`](../../plan/DEFINITIVE-EDITION.md) §2: **what has to be obtainable in one save, and what blocks it today.** It covers criteria C1 (Souls), C2 (chips) and C3 (bosses).

| File | Criterion | Contents |
|---|---|---|
| [chips.md](chips.md) | C2 | All 389 chip records, sorted by how each one is obtained, and the gaps |
| [souls.md](souls.md) | C1 | The 12 Double Souls: flags, pseudo-chips, versions, and what the merged ROM needs |
| [bosses.md](bosses.md) | C3 | Every Navi and tier the game has, per version, and rematch status |
| [mystery-data-shops-traders.md](mystery-data-shops-traders.md) | C2, C4 | Mystery Data, shops and BugFrag traders per net area; missable items |
| [method.md](method.md) | | How the inventory is generated and checked, and how far each source can be trusted |

## How it is made

```powershell
python patch/tools/inventory.py      # writes build/inventory/chips.csv and prints the counts below
```

- **The ROM is authoritative** for what exists: chip ids, classes, codes, flags. It reads both US ROMs' chip tables. The two tables turned out **identical** (0 of 389 records differ), so a chip "exclusive" to a version exists in both ROMs and is only unobtainable in one.
- **Third-party references** (local only, in `ref/`) say where each code is obtained, until the ROM's own source tables (GMD tables, shops, traders, scripts) are decoded. See [method.md](method.md).
- **The CSV stays in `build/`:** it holds chip names (game text) and facts from the references. Only the script, the counts and the conclusions are tracked.

## Headline numbers (2026-11-22)

| Class | Records | In-game in both | One version only | No in-game source | To check |
|---|---|---|---|---|---|
| Standard (1–150) | 150 | 142 | 0 | 0 | 8 (6 codes without a listed source, 2 matched only by the maps) |
| Secret (151–186) | 36 | 0 | 0 | **36** (Navi Soul trading, 35; 1 not listed) | |
| Mega | 81 | 42 | **18 RS + 18 BM** | **2** (Z-Saver, Grand Prix Power) | 1 (BurnMnDS) |
| Giga | 12 | 0 | **5 RS + 5 BM** | **1** (Duo) | 1 (FinalGun) |
| Program Advance | 30 | combinations, no source needed | | | |
| Dark | 10 | 8 (random, while Worried or Dark) | | | 2 unnamed records |
| Double Soul pseudo-chips | 12 | | **6 RS + 6 BM** | | |
| Internal (not in the Library) | 57 | Soul attacks, modes, unused records | | | |

**So one save today is missing:**
- the other version's 18 Navi chips, 5 Giga chips and 6 Souls;
- all 36 secret chips;
- 3 distribution or link chips (Z-Saver, Grand Prix Power, Duo).

That makes 68 items for WP-D2/WP-D4 to give an in-game source, plus up to 4 to confirm.

**2026-11-22: a Blue Moon Soul already works in Red Sun** once granted, with a 6-byte patch (the mirror case, Red Sun Souls in Blue Moon, is untested) ([../re/soul-merge-experiment.md](../re/soul-merge-experiment.md)). The merge work for C1 is the grant, not the Soul data.
