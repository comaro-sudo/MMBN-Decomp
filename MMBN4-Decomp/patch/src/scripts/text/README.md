# Text patches (TextPet)

Each `.tpl` here holds **only our scripts** for one existing text archive of the game. The archive is named symbolically on its `@archive` line (`CHIP_NAMES_1`, `CHIP_DESCS_1`); `patch/build.py` replaces the name with the version's offset (`VERSIONS[...]["text_archives"]`), builds an index with only those archives, and runs TextPet with `--patch`, so every other script of the archive is kept. Archives that grow move to `FS_TEXT` and are repointed. Recipe verified by Gemini (`decomp/notes/chip-text-design.md`). Save as UTF-8 **without BOM**; no comments in the `.tpl` files.

| File | Archive | Scripts |
|---|---|---|
| `chip_names_1.tpl` | Chip names, bank 1 (chips `0x100`–`0x184`; RS `0x748C14` / BM `0x748708`) | 55 Duo, 56 GP Power (blank e-Reader names in the US ROMs) |
| `chip_descs_1.tpl` | Chip descriptions, bank 1 (`0x100`–`0x170`; RS `0x74F03C` / BM `0x74EACC`) | 55 Duo ("Fist of Justice: MetKnukl!", owner), 56 GP Power ("Speedy / trio / attack!", Gemini's wording, approved by the owner) |

Names are at most 8 characters; descriptions are 3 lines of up to 9.
