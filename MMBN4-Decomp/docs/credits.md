# Credits and licences (2026-11-27)

Every tool and third-party work that the Violet Eclipse patch and the ports use or learn from, with its author, its licence, the source of that licence fact, and what attribution it asks for.

**How to read this.** The "Source" column says where the licence was read:
- **local** = the project's own file in the main checkout's gitignored `tools/` or `ext/` (read only, nothing copied into this repo);
- **web** = the project's public repository (GitHub raw file or API), read 2026-11-27;
- **README** = a licence sentence in the project's README, not a licence file.

Anything I could not confirm is marked **UNCONFIRMED**. This is a fact sheet, not legal advice.

## 1. What a released `.bps` contains

A `.bps` holds only the bytes that differ from the player's ROM, plus checksums. It contains no code or data from any tool below: the tools run on the build machine and only produce our bytes. So the tools' licences constrain whoever ships *the tools themselves*, not the `.bps`. The one third-party *work* that is reassembled into the patch is Prof. 9's WoodMan fix (row 1).

## 2. Used by the patch

| # | Item | Author | Licence | Source | What it requires of us |
|---|---|---|---|---|---|
| 1 | **WoodMan Glitch Fix v2.0** (`ext/BN4-WoodMan-Fix`, repo `RockmanEXEZone/BN4-WoodMan-Fix`). `wp_a1_woodman_fix.asm` re-creates it byte-identical | Prof. 9 | No licence file. The readme says: "You are free to use these fixes in any way you like. Attribution is welcome, but not necessary." | local `ext/BN4-WoodMan-Fix/readme.md`; web (the repo has no LICENSE file) | Nothing binding. We credit him anyway. A permission in a readme is not a formal licence |
| 2 | **armips** v0.11.0 (assembler) | Kingcom | MIT, "Copyright (c) 2009-2020 Kingcom" | local `ext/armips/LICENSE.txt`, and `Readme.md` section 7.3 | Only if we redistribute armips (keep the notice). The patch does not ship it |
| 3 | **TextPet** v1.0.0 (text archives), its BN4 plugins and indexes | Prof. 9 ("Copyright (c) 2016 Prof. 9") | MIT | web `Prof9/TextPet` `LICENSE.txt`. The local `tools/textpet/` copy has no licence file | Only if we redistribute TextPet. Not shipped |
| 4 | **Flips** v198 (BPS creation; also what players use to apply) | Alcaro (the Sir-Walrus fork is the current home; archived 2025-05) | GPL-3.0-or-later. `COPYING` says "GNU General Public License, version 3.0 or higher": credit Alcaro, label modified versions, and any application containing it must give its source under the same licence. GitHub's API reports `NOASSERTION` because `COPYING` is a custom wrapper around `COPYING.gpl3` | local `tools/flips/COPYING` and `COPYING.gpl3` | Only if we bundle Flips in a release zip: then ship its licence and a source pointer. Telling players to download it separately needs nothing |
| 5 | **mmbn-chip-tables** (`bn4_chip_macros.asm`, `bn4b/bn4r_chip_table.asm`; used only by `build.py --check` as the identity test, and for chip names in `inventory.py`) | StraDaMa | **No licence.** The repo has no LICENSE file and no licence sentence in its README (the API reports none). With no licence the default is "all rights reserved" | local `ext/mmbn-chip-tables/` (README only); web (file list has no LICENSE) | We must not redistribute its files, and we don't: `ext/` is gitignored and `build.py` reads it from the developer's machine. The chip IDs in `defines.asm` are facts about the game. Credit StraDaMa, and ask him if his tables are ever to ship |
| 6 | **Community GameShark SP saves** (13 unique, GameFAQs, 2004-2012), used for testing and for `docs/re/community-saves.md` | Uploaders IshiMikado, Cybertic_Dragon, Sonic_De_Hedgeh, LiquidVee, jgkrogers, yodaexe, _Ranmyaku and others (names from `ref/saves/saves.txt`) | **No licence stated** (user uploads). They hold game data | local `ref/saves/saves.txt` | Not redistributed (`ref/` is gitignored). A thank-you in the README is courtesy, not a requirement. Do not ship them |
| 7 | **Symbol names and addresses from Cyberworld Endless** (`docs/symbols/bn4-red-sun-us.sym`, about 700 names): used to find addresses and in the PC port's RAM map | Sascha Becker (`saschb2b`) | MIT, "Copyright (c) 2026 Sascha Becker" | local `ext/Mega-Man-Battle-Network-Cyberworld-Endless/LICENSE` and its README | MIT: keep the copyright and permission notice in any copy of a substantial portion. If names are copied into `decomp/symbols/names.txt` (Gemini's area), a credit and the MIT notice there is the safe course |
| 8 | **bn6f**, the BN6 Falzar disassembly (naming conventions, engine knowledge) | the `dism-exe` community | **No licence file** in the repo, and the README states none | local `ext/bn6f/` (README, no LICENSE); web (the file list has no LICENSE) | Read-only reference, none of its files are in this repo. If names are copied from it, no permission is stated: **UNCONFIRMED** whether that is allowed |
| 9 | **exe4rs** (JP Red Sun IDA disassembly) | `dism-exe` | **No licence file** | local `ext/exe4rs/` (README only); web (no LICENSE) | Function-boundary reference only; nothing is copied |
| 10 | **nettai** (engine documentation, memory addresses) | `nettaisen` | MPL-2.0 | local `ext/nettai/LICENSE`; web API reports MPL-2.0 | Reference only. Any of its files we copy stay MPL-2.0 |
| 11 | **MMBN4.5 English Translation** (build pipeline used as a model) | Darknet, Jumi, Lex, MegaRockEXE, MidniteW, Prof. 9, Tim MacKenzie | BSD 3-Clause, "Copyright (c) 2019" | local `ext/MMBN45-English-Translation/license.txt` | Reference only; nothing copied. If code is ever copied, keep the notice |

## 3. Used by the ports

The ports are GPL-3.0 (owner decision `D-EL3`, 2026-11-25).

| # | Item | Author | Licence | Source | What it requires of us |
|---|---|---|---|---|---|
| 12 | **mGBA** 0.10.5, linked unmodified as a static library (PC, Wii U, 3DS builds) | Jeffrey Pfau and mGBA contributors | MPL-2.0 ("Copyright 2013-2022 Jeffrey Pfau"). Compatible with GPL-3.0 (`plan/emulation-layer/06-milestones-and-risks.md`, D-EL3) | local `ext/mgba/LICENSE`, `README.md` ("Copyright" section); the binary's `tools/mgba/.../LICENSE.txt` | Binary releases must ship mGBA's licence and point to its source (`ports/pc/README.md` already says so). Its bundled libraries have their own notices (README lists inih BSD-3, blip-buf LGPL, others): ship the matching notices with any binary that links them |
| 13 | **SDL3** 3.4.20 (PC port) | Sam Lantinga and SDL contributors | zlib: "Copyright (C) 1997-2026 Sam Lantinga" | local `tools/sdl3/SDL3-3.4.20/LICENSE.txt` | zlib: no notice needed in binaries; keep it in source copies. We credit it as courtesy |
| 14 | **wiiu-sdl2, WUT, devkitPPC** (Wii U build, in WSL `/opt/devkitpro`) | devkitPro and Wii U homebrew community | **UNCONFIRMED.** Not read: the packages live in WSL, not in the repo or `tools/` | none | Check each package's licence before a Wii U release |
| 15 | **libctru, citro2d, citro3d, devkitARM** (3DS build) | devkitPro and contributors | **UNCONFIRMED.** Not installed here | none | Check before a 3DS release |
| 16 | **Silent Hill Wii U GX2/GamePad code** (reused for the Wii U port, per `D-EL3`) | the owner | GPL-3.0 | owner decision `D-EL3` in `docs/pending/owner-decisions-2026-11-24.md`. Not checked against the code itself | It is why the ports are GPL-3.0 |
| 17 | **Cyberworld Endless** (the embedded-mGBA architecture, Red Sun RAM map) | Sascha Becker | MIT | local `LICENSE` | As row 7 |

## 4. Testing only (never shipped)

| Item | Author | Licence | Source |
|---|---|---|---|
| **Cemu** (Wii U), ARM64 fork `rhemfur/Cemu` | Cemu team; fork by rhemfur | MPL-2.0 (bundled dependencies keep their own) | local `ext/cemu/LICENSE.txt` and README |
| **Dolphin** (GameCube/Wii) | Dolphin team | "Most ... GPLv2+", mixed overall | local `tools/dolphin/Dolphin-arm64/COPYING` |
| **PPSSPP** (PSP) | Henrik Rydgard and contributors | GPL-2.0-or-later ("Originally released under the GPL 2.0 (and later)") | local `tools/ppsspp/README.md` |
| **Vita3K** (PS Vita) | Vita3K team | GPL-2.0 (the shipped `COPYING.txt` is GPLv2) | local `tools/vita3k/COPYING.txt` |

None of these is part of the patch or of a port release, so none puts an obligation on our files. Dolphin, PPSSPP and Vita3K are not used for BN4 today; they came with the machine setup.

## 5. Not covered by any licence above

- **Capcom** owns *Mega Man Battle Network 4*, its names, art and text. The patch contains none of it (the `--public` build leaves out the local Duo and Grand Prix Power art). The player README says so.
- The Duo and Grand Prix Power chip *art* (`ref/chip-art/`) is the owner's local material and is not released.

## 6. Open points

1. **StraDaMa's chip tables** have no licence. Today only the developer's machine reads them. If the identity test is shipped or the table is copied into the repo, ask him first.
2. **bn6f / exe4rs** have no licence. Names taken from them need an owner decision (credit, or re-derive).
3. **Prof. 9's WoodMan fix** is "free to use", in a readme only. Attribution is in the source header and the player README.
4. **devkitPro packages** for the Wii U and 3DS builds: licences not confirmed.
5. **Flips:** the player README only links to it. Bundling it in a release zip would add GPL-3.0-or-later notice duties.
