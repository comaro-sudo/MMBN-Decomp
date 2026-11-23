# ref/: third-party reference material (local only)

This folder is gitignored except for this README. It holds material by other people that helps reverse engineering and the *Violet Eclipse* content inventory but can't be redistributed:

| Path | What | Source |
|---|---|---|
| `guides/bn4-battle-chip-guide-dx_xetrov.txt` | Every chip: class, codes, and where each code is obtained (RS/BM exclusives marked) | GameFAQs, DX_Xetrov, v2.7 (2004) |
| `guides/bn4-zero3-link-faq-shift_breaker.txt` | The *Mega Man Zero 3* link and the Z-Saver chip | GameFAQs, Shift_Breaker, v1.20 (2005) |
| `guides/bn4-chip-attributes-faq-phlemingo.txt` | Chip elements and attributes | GameFAQs, Phlemingo |
| `saves/*.sps` | 15 community saves (13 unique: `…22565(1)` = `…22565`, `…7830` = `…4579`) (GameShark SP / SharkPort format), Red Sun and Blue Moon, from a first playthrough to a fifth; `saves/saves.txt` has the uploaders' descriptions, `saves/index.md` what each file actually contains | GameFAQs save uploads (2004–2012) |
| `maps/*.png` | Net area maps with Mystery Data locations (32 areas) | Mega Boy (maps ©Mega Boy, game ©Capcom) |
| `chip-art/*.png` | Official 56 x 48 chip pictures for Duo and Grand Prix Power, which are in no ROM (event-only). Supplied by the owner from a wiki; converted at build time for private builds only (`docs/completion/chips.md` §3) | Capcom (via MMKB / wiki) |

Convert or inspect a save: `python patch/tools/savetool.py [--convert] ref/saves/*.sps` (converted `.sav` files go to `build/saves/`).

Rules:
- Use these to cross-check findings: chip locations, Mystery Data, exclusives. Docs may state facts learned from them, but don't copy their text or images into tracked files.
- The authoritative lists come from the ROM's own tables (see `plan/DEFINITIVE-EDITION.md` §2).
