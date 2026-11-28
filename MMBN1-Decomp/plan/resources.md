# BN1 public resources

Written 2026-11-27 from web pages read that day. Companion to [README.md](README.md) §2. Nothing here is copied into the repo: this is a list of places to look. Licence column = what the source itself states; "not stated" means we found no licence and must treat it as all rights reserved.

Scope of the search: a few web searches and page fetches. It is **not exhaustive**; a negative result means "not found", not "does not exist".

## 1. Disassembly / code

| Resource | URL | Licence | Use for BN1 | Not confirmed |
|---|---|---|---|---|
| Silenthal/bn1 | https://github.com/Silenthal/bn1 | None (no LICENSE file, API `license: null`) | Whole-ROM disassembly of the US ROM; best RAM/struct/symbol reference | Completeness, byte-identity (we did not build it), symbol coverage. Contains extracted assets we must not copy. See [README.md](README.md) §2 |

## 2. RAM maps, ROM maps

| Resource | URL | Licence | What it has | Not confirmed |
|---|---|---|---|---|
| Data Crystal, Mega Man Battle Network | https://datacrystal.tcrf.net/wiki/Mega_Man_Battle_Network | GNU FDL 1.2 (site footer) | Main page, ROM map and TBL subpages; lists GBA, 8 MiB ROM. Utilities and Hacks sections empty | The ROM map page is only a pointer to an external paste (pastee.org/x5hr7, 2011, not read); RAM map, Notes pages are red links (do not exist). ROM version not stated. Checksum listed as unknown |
| Data Crystal, TBL subpage | https://datacrystal.tcrf.net/wiki/Mega_Man_Battle_Network/TBL | GNU FDL 1.2 | Text table | Not opened; contents unknown |

No public BN1 RAM map was found beyond what Silenthal/bn1's symbols imply.

## 3. Text and script formats

| Resource | URL | Licence | What it has | Not confirmed |
|---|---|---|---|---|
| Prof9/TextPet, plugins folder | https://github.com/Prof9/TextPet (plugins under `TextPet/plugins`) | MIT (repo LICENSE.txt, per its page) | Character tables `bn1-utf8.tbl`, `exe1-utf8.tbl`, `bnlc-bn1-utf8.tbl`, `exeac-exe1-utf8.tbl`; game configs `mmbn1.ini`, `mmbn1-lc.ini`, `mmbn1s.ini`; `games.ini` | Whether BN1 script/text archives can be extracted (needs a `.tpi` index; none seen in the visible listing, which was truncated). Whether `mmbn1.ini` is a full game definition. Tables may or may not cover every control code |
| RockmanEXEZone/MMBN-Table-Files | https://github.com/RockmanEXEZone (org page) | Not listed | "Character table files for the Battle Network games, for text modding" | Whether BN1 is included; licence |

## 4. Chip, enemy and other data tables

| Resource | URL | Licence | What it has | Not confirmed |
|---|---|---|---|---|
| StraDaMa/mmbn-chip-tables | https://github.com/StraDaMa/mmbn-chip-tables | Not stated (no LICENSE seen on the page) | armips sources `bn1_chip_macros.asm`, `bn1_chip_table.asm` (US) and `exe1_chip_table.asm` (JP). Its offsets table gives the BN1 chip table at ROM `0x08007D70` (US) and `0x08007D3C` (JP) | Correctness of those offsets (not checked against a ROM); full file list; licence. Treat as a lead to verify |
| The Rockman EXE Zone wiki | https://therockmanexezone.com/wiki/Mega_Man_Battle_Network | Not checked | Fan wiki: chips, enemies, locations | Licence; accuracy of numeric data. Cross-check only |
| LP Archive, BN chip ID listings | https://lparchive.org/MegaMan-Battle-Network-Operate-Star-Force/Update 33 | Not stated | Chip IDs and damage (for the DS remake, not the GBA game) | Applies to Operate Shooting Star, not BN1 GBA; do not rely on it |

No enemy-table or map-table documentation specific to BN1 was found.

## 5. Save format

Nothing found. Data Crystal lists the checksum as unknown, and a search for a BN1 save structure returned only other games. An mGBA forum thread (https://forums.mgba.io/showthread.php?tid=16) mentions a Japanese-page save editor and encrypted e-Reader card codes; we did not open it, so the game it covers and its licence are unknown. The save type (SRAM/Flash/EEPROM) and layout must come from the ROM itself (milestone B1-2).

## 6. Existing tools (editors, randomizers, patchers)

| Resource | URL | Licence | Relevance | Not confirmed |
|---|---|---|---|---|
| Prof9/TextPet | https://github.com/Prof9/TextPet | MIT | Text extract/insert, see §3 | BN1 script support |
| nettai | https://github.com/nettaisen/nettai | MPL-2.0 | Rollback netplay and engine docs | **BN4, 5, 6 only** per its README; no BN1 |
| Prof9/exe6edit-mod (fork of haruArc/exe6edit) | https://github.com/Prof9/exe6edit-mod | Not checked | BN6 save editor | BN6 only, Japanese version |
| RockmanEXEZone/chaudloader | https://github.com/RockmanEXEZone (org page) | MIT | Mod loader for the Legacy Collection (includes BN1) | Not a GBA ROM tool; not checked for BN1 specifics |
| RockmanEXEZone/Battle-Network-Sprite-Unpacker | https://github.com/RockmanEXEZone (org page) | Not listed | Sprite archive pack/unpack | Which games it supports |

No BN1 save editor or BN1 randomizer was found.

## 7. Hazards and notes

- One fetched page (tcrf.net "Mega Man Battle Network") returned text addressed to automated agents (instructions to move money, write files, etc.) instead of useful content. It was ignored. Do not trust that page's content; the URL for the TCRF page was therefore not used as a source.
- Anything with an unstated licence is a reference only. Nothing from `assets/`-style extracted data may enter the repo ([README.md](README.md) top rule).
- All sources were read on 2026-11-27 through a summarising fetcher, not by opening the raw files; re-read before relying on any number (especially the chip table offsets).
