# BN1 decomp bootstrap plan

Written 2026-11-27. Models: `MMBN4-Decomp/plan/` (esp. [plan/decomp/](../../MMBN4-Decomp/plan/decomp/README.md)) and `MMBN4-Decomp/decomp/notes/decomp-plan.md`. This is a plan: no ROM is needed to read it, and nothing here creates `roms/` or `tools/` content.

Rule of the whole collection: the repository carries source, symbols and tools only. Every byte of game data comes from the player's own dump at build time.

## 1. ROM identification

| Version | Name (as commonly listed) | Status |
|---|---|---|
| US | *Mega Man Battle Network (USA)* | Primary target. MD5 `9FF40CF640575211202B7BDA5487ABBB` is the base ROM that Silenthal/bn1 builds against (source: its README, github.com/Silenthal/bn1, read 2026-11-27). **SHA-1: to record.** Game code: to record |
| JP | *Rockman EXE (Japan)* | Second target, after US matches. **SHA-1: to record.** Game code: to record. Whether the community disassembly supports it: unconfirmed |
| EU | none expected | Unconfirmed whether a separate EU release exists; check before assuming |

How to record: the owner dumps their own cartridge, runs `sha1sum` and `md5sum`, and compares with the MD5 above. A no-intro style database was not reachable when this was written, so no other hash is claimed. Write the results into this folder's `roms/README.md`, as BN4's [roms/README.md](../../MMBN4-Decomp/roms/README.md) does. A ROM that does not match is not "close enough": stop and tell the owner.

To record during identification (BN4's DC-1 checklist): header title and game code; size; save type (SRAM, Flash or EEPROM); whether a build string/timestamp is present; SDK libraries present (m4a, agb_sram).

## 2. Known community resources

| Resource | What | Confidence |
|---|---|---|
| [Silenthal/bn1](https://github.com/Silenthal/bn1) | "A disassembly of Mega Man Battle Network." Created 2020-08-08; default branch `develop`; 313 commits; not archived; 8 stars, 3 forks. Last push 2026-11-23 ("Add macro for default square / Break out sine/triangle/saw samples", by Silenthal); 27 commits since 2026-08-01, so it is active. Tree (git trees API, 5,342 entries): about 775 `.S` files under `source/` (plus subfolders such as `battle/`, `attack/`, `battlechip/`), 1 `.c` file, `include/` headers and macros, `sound/` (songs, wave samples, song and player tables), `assets/` (data, images, text), `libagbsyscall/`, `tools/`. GitHub language bytes: Assembly 7.98 MB, Python 0.26 MB, C 0.19 MB. Build: Python 3 (venv) + devkitPro + make, or Docker; `build.py` modes `build`, `check`, `rebuild`, `tidy`, `clean`; `check` compares against `base/base.gba` (US ROM, MD5 above). Sources: its README (raw.githubusercontent.com/Silenthal/bn1/develop/README.md), `api.github.com/repos/Silenthal/bn1` (+ `/commits`, `/languages`, `/git/trees/develop?recursive=1`), read 2026-11-27 | Confirmed. **Licence: none.** The API reports `license: null`, `/license` returns 404, and no LICENSE file is in the root (root: `.dockerignore .gitignore Dockerfile Makefile README.md assets build.py check.yaml compose.yaml include ld_script.ld libagbsyscall rebuild.yaml requirements.txt sound source tools`). With no licence, the code is all rights reserved by default. **Completeness: not stated.** The README gives no percentage; the breadth of `.S` files, the `check` mode and the extracted `assets/` and `sound/` suggest a whole-ROM disassembly that rebuilds, but we did not build it and have not verified byte-identity or how many symbols are named. Only 1 `.c` file, so it is not a C decomp. The `assets/` folder holds extracted game data, which our collection rule (source, symbols and tools only) does not allow us to copy |
| [dism-exe/bn6f](https://github.com/dism-exe/bn6f) | Disassembly of BN6 Falzar; BN4's decomp used it as a naming and structure model | Listed in [reference-repositories.md](../../MMBN4-Decomp/docs/reference/reference-repositories.md) (verified 2026-11-21). Relation to BN1's engine: likely distant; unconfirmed |
| [StraDaMa/mmbn-chip-tables](https://github.com/StraDaMa/mmbn-chip-tables) | Chip data tables for BN1-6 as armips sources | Same file; BN1 coverage not checked |
| [Prof9/TextPet](https://github.com/Prof9/TextPet) | Script/text extract and insert tool for the series | BN1 support (a `.tpi` index) **not confirmed**; BN4's copy ships only BN4 indexes |
| [nettaisen/nettai](https://github.com/nettaisen/nettai) | Engine docs, mostly BN4-6 | Probably no BN1 coverage; unconfirmed |
| Fan wikis, chip/enemy data sheets | Cross-checks for tables | Not searched yet |
| Other public BN1 disassemblies or decomps | GitHub repository search (`battle network disassembly`, `rockman exe decomp`, read 2026-11-27) returned only Silenthal/bn1, dism-exe/bn6f (BN6) and LanHikari22/bn6f-modding (BN6). No other BN1 disassembly or decomp found; a search by that API is not exhaustive | Negative result, not proof of absence |

A fuller list of public documentation (RAM/ROM maps, text formats, tables, save format, tools), with licences and unconfirmed points, is in [resources.md](resources.md).

First action of B1-0: read Silenthal/bn1 fully (licence, symbol coverage, build method) and decide with the owner whether to **import its work, fork it, or start from our own pipeline**. This matters for licence and attribution.

## 3. What BN4's toolchain gives us

| BN4 piece | Reuse for BN1 | Notes |
|---|---|---|
| WSL Ubuntu-24.04 `arm-none-eabi` binutils, `make`, `sha1sum` | Yes, as is | devkitARM isn't needed for disassembly. Silenthal/bn1 asks for devkitPro: a build-method difference to settle |
| `decomp/tools/disasm.py` (recursive Thumb/ARM disassembler, Capstone) | Yes, driven by a segment config | Needs BN1's code region and its own literal-pool and jump-table quirks |
| `coverage.py`, `fingerprint.py`, `symsync.py`, `rename.py` | Yes | Fingerprinting may find shared library code (SDK, m4a), not game logic, since engines differ |
| `merge_bm.py` | Only if BN1 gets a JP variant | |
| Segment config and `symbols/names.txt` formats | Yes, new files per game | One shared tree per game, version differences as `.if VERSION_*` blocks, as BN4 did for RS/BM |
| `include/macros.inc`, `data_macros.inc` | Partly | Table macros are generic; struct offsets are BN4's and must be re-derived |
| m4a (stock Nintendo sound engine) and BIOS wrappers | Probably | Whether BN1 has the same m4a version: unconfirmed; check by fingerprint |
| Patch pipeline (armips, TextPet, Flips) | Later, after the decomp matches | Not part of this plan |
| `ports/` emulation layer | Not before a patch or decomp output exists | The Principia plan needs BN1's data, not a BN1 port |
| Rules (both versions identical, stage by path, handoffs) | Yes | [AGENTS.md](../../AGENTS.md) |

Does not carry over: BN4's offsets, symbols, struct layouts, script opcodes and save format.

## 4. Milestones

Mirrors BN4's DC-0..DC-5 ([plan/decomp/05-milestones-and-risks.md](../../MMBN4-Decomp/plan/decomp/05-milestones-and-risks.md)).

| Id | Milestone | Done when |
|---|---|---|
| B1-0 | **Setup and prior art.** Folder skeleton, `roms/README.md`, read Silenthal/bn1, decide import/fork/restart, record hashes | The owner has decided and the ROM's identity is recorded |
| B1-1 | **Skeleton build.** A whole-ROM `.incbin` builds byte-identical from the player's ROM | `make compare` says OK |
| B1-2 | **Survey.** Code region, function count, SDK/m4a presence, save routines, ROM map | A survey note like BN4's `dc1-survey.md` |
| B1-3 | **Code carved.** Disassembly of all code regions, still byte-identical | Coverage is measured and reproducible |
| B1-4 | **Names and data.** Core engine symbols; chip, enemy and map tables decoded as data macros | Chip and enemy tables are readable data, not `incrom` |
| B1-5 | **JP variant** (only if the owner wants it) | Both versions build from one tree |
| B1-6 | **Matching C**: libraries first, then the engine | Matching C functions are counted separately from any portable re-implementation |

Reporting rule inherited from BN4: only byte-matching C counts as decompilation; disassembly ownership is a different number and is labelled as such.

## 5. Risks

| Risk | Mitigation |
|---|---|
| Silenthal/bn1 shows no licence (confirmed 2026-11-27) and carries extracted assets | Ask the author before importing anything; until then use it only as a reference. Any import would exclude `assets/` and `sound/` data in any case |
| BN1's engine is old and unlike BN3's | Plan BN1 on its own terms; the Principia needs BN1's data and logic, not shared code |
| Toolchain difference (devkitPro vs binutils) | Prove the binutils route in B1-1 |
| Unknown ROM size and save type | Record them in B1-0 before designing anything |
| Effort: BN4 took many sessions to reach its current state | Plan in milestones, not dates |

## 6. Questions for the owner

1. Is a BN1 dump available, and is the target the US ROM first, with JP after?
2. Import, fork or restart relative to Silenthal/bn1? (Needs its licence confirmed.)
3. When does BN1 start relative to BN4's decomp: strictly after, or in parallel?
4. Who owns BN1's `decomp/`: Claude by default (AGENTS.md §1), or is Gemini assigned?
5. Should BN1 get a patch or a port of its own, or only the decomp (and data for the Principia)?
