# How the inventory is generated

## Sources and trust

| Source | Where | Used for | Trust |
|---|---|---|---|
| Both US ROMs, SHA-1 checked | `roms/` | Chip ids, class (`+0x08`), flags (`+0x09`: `0x04` Navi, `0x20` Dark, `0x40` in Library), codes (`+0x00`–`+0x03`), Library number (`+0x1C`) | Authoritative |
| `mmbn-chip-tables` | `ext/` (cloned) | Chip names (the comments in `bn4r_chip_table.asm`) | High; names only |
| DX_Xetrov's Battle Chip Guide v2.7 | `ref/guides/` | Per-code sources, `(RS)`/`(BM)` tags | Medium: a 2004 fan guide, some gaps |
| Mega Boy's net maps | `ref/maps/` → `legend.csv` | Mystery Data, shops, traders by area | Medium: one playthrough's view |
| The game's code | `docs/re/` | Soul flags and pseudo-chips, Library flags, New Game+ | Authoritative where marked V or R |

## Classification rules (`patch/tools/inventory.py`)

1. **Kind:**
   - Soul pseudo-chips are `0x161`–`0x16C`.
   - PA is Library class 4.
   - Dark means the `0x20` flag.
   - A record without the Library flag is internal, unless the guide lists it (Duo, Grand Prix Power).
   - Otherwise the kind is the class.
2. **Guide match:** by normalized name first (the guide reuses numbers 19–36 for each version's Navi chips), then by the class-relative Library number.
3. **Per code:** the guide's source list is split on commas.
   - A part naming a link, trade, promotion or Navi Soul data counts as link/trade.
   - Every other part counts as in-game.
   - `(RS)` or `(BM)` limits a part to one version.
   - Navi chips take the version of their Navi (the six Soul Navis of each version).
4. **Status:**
   - in-game both / RS only / BM only;
   - partial (some codes have no source);
   - no in-game source (link/trade, or none listed);
   - unknown (no reference entry).

## Known limits

- The guide's sources are per code, but not per playthrough tier. The CSV keeps the raw text, so "(Hard Mode)" notes stay visible.
- Folder contents (starting, Family, Extra folders) are incomplete in the guide.
- Until RE-07 (Mystery Data / GMD tables), RE-09 (shops) and the trader tables are decoded, "no listed source" means *not found in the references*, not *absent from the game*.

## Next steps

1. RE-07 and RE-09: read the GMD, BMD/PMD, shop and trader tables from the ROM, and add per-version, per-tier columns from the game's own data.
2. Add a Navi/boss sheet from the enemy records (RE-06).
3. Regenerate after each patch change; an item that loses its source is a regression (DEFINITIVE-EDITION §5).
