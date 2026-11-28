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
python patch/build.py --public   # leave out the local-only Duo / Grand Prix Power art (shareable patch)
```

Release package: `python patch/tools/release.py` builds `--public` for both versions, runs the `--check` tests and writes `build/release/` (the two `.bps`, `RELEASE-SHA1.txt` with the input-ROM, patch and patched-ROM SHA-1s, and a copy of `docs/player-readme-draft.md` as `README.md`), then zips it to `build/violet-eclipse-<date>.zip`. Options: `--no-zip`, `--name NAME`. It never includes a patched ROM.

Needs:
- **Both US ROMs** in `../roms/`, `bn4_rs_us.gba` and `bn4_bm_us.gba`, which are hash-checked. The random-drop package copies the other version's drop table from the player's other ROM at build time, so building either version needs both.
- `tools/armips/armips.exe` and `tools/flips/` ([`../docs/reference/toolchain.md`](../docs/reference/toolchain.md)).

The pipeline:
1. verify the ROM hashes;
2. run armips;
3. **patch text** with TextPet: our scripts in `src/scripts/text/*.tpl` (see `src/scripts/text/README.md`; `*.tpl` is gitignored everywhere else), merged into the version's existing archives with `--patch`;
4. fix the header checksum;
5. create the `.bps` with flips, and check that it round-trips.

Text (see [`../plan/text-insertion.md`](../plan/text-insertion.md)):
- the `.tpl` files hold **only our own scripts** (`@archive <NAME>` + `script` blocks, UTF-8 without BOM), never extracted game text; `build.py` maps the name to the version's offset and keeps every other script of the archive (Gemini's recipe, `decomp/notes/chip-text-design.md`);
- today: Duo and GP Power names and descriptions (chip archives bank 1, both moved to `0x087FC000`; checked script by script in both builds);
- TextPet rewrites only those archives: in place when they still fit, otherwise into `FS_TEXT` (`0x087FC000`–`0x08800000`, set in `defines.asm`);
- it needs `tools/textpet/` (TextPet with its BN4 plugins and US indexes).

## Smoke test

`patch/test/smoke/smoke.py` drives the PC port headlessly (`mmbn4.exe --test`, scripted input, EWRAM dump) and checks a few packages end to end in both versions, each against the vanilla ROM where that makes sense:

```powershell
python patch/build.py                      # the patched ROMs
python ports/pc/build_pc.py                # mmbn4.exe
python patch/test/smoke/smoke.py           # all checks; -k NAME for one, --list to list them
```

| Check | What it verifies |
|---|---|
| `shop-table` | The new-game shop stock table in the patched ROM: HP Memory 207,700Z → about 83,000Z (83,100Z with the per-line rounding), 34 of 34 WP-A3 prices at 40%, nothing else changed |
| `shop-migrate` | An existing save loaded in the patched ROM gets its HP Memory prices at 40% (the `SaveLoad` hook) |
| `status-pages` | On the MegaMan status screen R shows page 2, a second R page 3, L goes back, a third R wraps; vanilla ignores R |
| `tuner` | Talking to the Frequency Tuner in Higsby's sets flag `0x0E58` (Auto → Tier 1); vanilla sets nothing. Red Sun only: the Blue Moon save edit that moves Lan into Higsby's loads an empty room |

It reads the ROMs from `roms/` and `build/`, and the saves from the gitignored `ref/saves/*.sps`; a check whose save is missing is skipped. Everything it makes (converted and edited saves, dumps, screenshots) lives in a temporary folder that is deleted at the end. Nothing from a ROM is committed. Not covered yet: a Mystery Data pickup (it needs a scripted route through a net map). The inputs are frame-timed, so a check can fail on a very different boot speed.

## What's in it

Grouped by area. **Player sees** is the in-game effect; the technical column has the hooks and addresses; **Status** is the test status.

### Economy

Zenny and shop prices.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_a2_zenny_battle.asm` | WP-A2 | Battle Zenny rewards are 1.5 times bigger. | Battle Zenny × 1.5; the shown and granted amounts match | **PASS in-game** (Gemini, PC port: 50 Z → 75 Z, shown and granted) |
| `wp_a3_key_item_prices.asm` | WP-A3 | HP Memories, Zenny NaviCust programs, Unlockers and LocEnemies cost 40% of the vanilla price, on new and existing saves. | **Shop prices at 40% (Option 2) + existing-save load hook:** 14 HP Memories (207,700Z → 83,000Z), 12 Zenny NaviCust programs in Shops 14–15 (92,900Z → 36,900Z), 4 Unlockers (4,000Z → 1,600Z), and 4 LocEnemies (10,000Z → 4,000Z) in `BN4_SHOP_STOCK_INIT`, plus idempotent `SaveLoad` sync (`RS 0x08048948 / BM 0x08048950` via `VENEER_SAVE_LOAD` `0x08001794`) for pre-existing saves | **PASS in-game** (all 34 slots verified in RS & BM ROMs, New Game EWRAM, and existing vanilla save migration on load) |

### Souls (Double Souls, Duo)

The 12 Souls, the 13th Soul, and the status screen that lists them.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_d4_twelve_souls.asm` | WP-D4 | All 12 Double Souls can be used in both versions. | All 12 Double Souls can unite in both versions | Built; confirmed in-game |
| `wp_d4_star_twelve_souls.asm` | WP-D4 | The Souls completion star needs all 12 Souls. | The Souls completion star needs all 12 Souls (12-entry list, and the count compare raised to 12) | **PASS in-game after the fix** (`79398a4`): 6 Souls → no star, 12 → star, RS and BM |
| `wp_d4_status_two_pages.asm` | WP-D4 | The MegaMan status screen shows the Souls in pages of 6: R next page, L previous. | The status screen shows all 13 Souls (12 + Duo) as pages of 6: **R** next page, **L** previous; page 3 (Duo, drawn with MetalSoul's icon) appears only once a 13th Soul is owned | **PASS in-game** (Round 13: all 3 pages verified with 13 Souls owned via R cycling). Fixed 2026-11-27: page 3 also showed with 9 to 12 Souls because the game only fills the first 8 bytes of the Soul buffer with `0xFF` and the patch reads up to index 12; the fill is now 32 bytes (`movs r1, 8` of that `ByteFill` becomes `0x20`). Checked on the PC port with real saves (RS 11 Souls, BM 9 Souls: 2 pages) and 12 / 13 Souls via flag edits (3 pages only with the Duo Soul) |
| `wp_d4_duo_soul.asm` | WP-D4 | Duo Soul, a 13th Soul, with its own colours, charge shot and passive abilities. | **Duo Soul**, the 13th Soul: unite tables moved to free space with entry 13 (class `0x0D`, flag `0x0023`); form 13 drawn with MetalSoul's sprite + custom Duo palette (`0x080107E2` via `VENEER_DUO_PAL` `0x0800A164`); charge shot = MetalFist at 180; passive = AirShoes + FloatShoes + B+Left Reflect Shield (`BN4_FORM_ENTERS[13]`); emotion-window OOB face fix (`BN4_FACE_GET` `0x08016A20`); ten class-`0x0C` chips → `0x0D`; MetalSoul's icon for `0x16D`; `FC 0C 0C` grants it | **PASS in-game** (transform, custom Duo palette, clean emotion window, Air/Float `0x30` + Reflect `0x6A` passive verified in PC port) |
| `wp_d2_duo_story_grant.asm` | WP-D2/D4 | Beating Duo gives the Duo Soul and the Duo D chip. | **Beating Duo gives the Duo Soul and Duo D.** Replaces Duo's exclusive deletion-state pointer `dat_080C04F8[2]` (RS `0x080C0500` / BM `0x080C050C`, originally `sub_08010DF8`) with `duo_defeat_hook` at `0x087FB8D8` (restoring Bass's `sub_080C4780` which sets `0x006F`). On any Duo version (story or rematch): grants Duo Soul (`0x0015`, `0x0023`, `WriteChipKey(0x16D)`) and Duo D once (`0x0E72`), then tail-calls `sub_08010DF8` | **PASS in-game** (PC port: killing Duo V1 fires `duo_defeat_hook`, sets `0x0023=True` and `0x0E72=True`, restores Bass `0x080C4780`) |
| `wp_d4_grant_soul.asm` | WP-D4 | Nothing by itself: the script command the friendly matches use to give a Soul. | Script command **`FC 0C <idx>`** grants any of the 12 Souls in either version (flag `0x15` + flag `0x17 + idx` + `WriteChipKey(0x161 + idx)`); for the friendly matches | Built, disassembly-checked; not yet called by any script |

### Tournaments

Fixed brackets, the Free Tournament and the friendly matches.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_b1_tournaments.asm` | WP-B1 | Tournament opponents are the same every time (canon Soul Navi pairs); the Den tutorial gives AquaSoul in Red Sun. | Fixed tournaments: canon Soul Navi pairs (AquaMan, Roll, ProtoMan), no pair coin flip or playthrough-2 swap, deterministic bracket; RS's Den tutorial gives AquaSoul | Bytes verified in RS and BM, smoke-tested (Gemini, `bac8ce9`) |
| `wp_d4_free_tournament.asm` | WP-D4 | The Free Tournament fields the 12 Soul Navis, so the 36 secret chips can be earned. | The Free Tournament fields the 12 Soul Navis without trades (missing chips first; type-0x01 battles), so the 36 secret chips are obtainable | **PASS in-game** (7 Soul Navis with no traded Navis; Gemini, `c9d36cf`) |
| `wp_b1_friendly_matches.asm` | WP-B1 | Nine friendly matches in the three tournament halls: win one and you get that Navi's Soul; a loss can be retried. | **9 friendly tournament matches** (3 per tournament in DenDome Hall, Castillo WaitRm, Colosseum Lobby; talk indices `0xE0`–`0xE8`, `FC 0E <eid> <sidx>`): normal story tier (`b4 = 0x3D`), retryable on loss, grants that Navi's Soul on win (`0x0017`+idx + `WriteChipKey`), hides once owned or after the tournament round window | **PASS in-game** (all 3 venues, all 9 sprites incl. Higsby & Sal, round gating, loss retry, win grants FireSoul `0x001B` & hides Match) |

### Rematches and SP+

Operators, boss rematches and the harder SP+ fights.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_d3_lobby_rematches.asm` | WP-D3 | Nine operators in Higsby's and two on the ACDC street, plus a trash can that warps to JunkMan; S ranks climb V1, V2, V3, SP, SP+. | 9 operators in Higsby's, 2 + JunkMan's trash can (`list[7] 0x4A` debris pile) on the ACDC street; rematches climb V1 → V2 → V3 → SP → SP+ on S ranks (`b4 = 0x3D` shows results screen; tier flags `0x0E40`–`0x0E57`); the trash can warps to JunkMan in `WtrGodComp1 (8E, 00)` | **PASS in-game** (debris-pile sprite & warp to `8E,00`, FireMan SP S-rank unlocks `0x0E5E`, SP+ 4,095 HP + `0x7A98` 15,000Z reward, Sal sprite fixed via `wp_d0_sprite_lazy.asm`) |
| `wp_d3_duo_rematch.asm` | WP-D3 | After the game, a "Duo's signal" in Meteor Comp 1 lets you fight Duo again at higher tiers. | **Duo rematches** (post-game): "Duo's signal" (`list[7] 0x07` Duo hexagonal crest) in Meteor Comp 1 `(82,00)` at (212, 112), shown after Game Clear (`0x0500`). "Fight Duo?" → `FC 0D` replays Duo (`00 00 64 16 33 17 9F 00`) at current tier (V1/α/β/Ω via `0x0E5A`–`0x0E5B`, SP+ via `0x0E5C`). Hooks `BattleCalcBustingLevel` (`0x08007692` via `VENEER_DUO_RANK` `0x08001784`) so S-rank wins (`r0 >= 11`) climb V1 → α → β → Ω → SP+ (`duo-rematch-rank.md`) | **PASS in-game** (Round 13: crest, Duo V1 S-rank climbs `0x0E5A=True`, DuoΩ S-rank unlocks `0x0E5C=True`, DuoΩ SP+ 4,095 HP) |
| `wp_d3_sp_plus.asm` | WP-D3 | SP+ bosses: 4,095 HP, stronger and faster, with a big Zenny or BugFrag reward. | **SP+ boss difficulty** (after S-ranking SP/Ω): 4,095 HP (`0x08006806`), ATK ×1.5 (`0x08013A06`), Speed ×1.25 (`0x08003196`), random reward 80% 15,000Z (`0x7A98`) / 20% 50 BugFrags (`0xC032`) (`RS 0x080F5804` / `BM 0x080F5810`). Covers 12 lobby Navis (`0x0E5D`–`0x0E68`), Duo (`0x0E5C`), and 10 roaming SP bosses (`0x0E69`–`0x0E71`) | **PASS in-game** (FireMan SP S-rank unlocks `0x0E5E`, SP+ 4,095 HP + ATK ×1.5 + Speed ×1.25 + `0x7A98` 15,000Z reward; DuoΩ SP+ 4,095 HP; roaming BassΩ `0xE5` S-rank unlocks `0x0E71`) |
| `wp_d3_battle_exit_clear.asm` | WP-D3 | Nothing visible: a lost or fled rematch no longer leaves its tier marker behind. | Zeroes `0x020015EC`–`0x020015EF` (rematch marker, SP+ bytes) on every battle exit, via `BattleExitCleanup`'s first call at `0x08006E20` through `VENEER_BATTLE_EXIT` (`0x0800A144`, `sub_0800A144` veneer pool); fixes the stale marker after a lost/fled rematch | **PASS in-game** (Round 12: lobby loss & Duo exit both cleared `0x020015EC`–`0x020015EF` to 0); veneer moved to `0x0800A144` |
| `wp_b2_frequency_tuner.asm` | WP-B2 | An NPC in Higsby's sets the virus tier: Auto, V1, V2 or V3. | Frequency Tuner: an NPC in Higsby's cycles the virus tier Auto / V1 / V2 / V3 (flags `0x0E58`–`0x0E59`), read by `BN4_ID_RAISE` | **PASS in-game** (RS, PC port): talking and answering Yes cycles Auto, Tier 1, 2, 3, Auto (flags `0x0E58`/`0x0E59` = 00, 01, 10, 11, 00), and the text box closes each time. Fixed a soft-lock: after "Yes" the box stayed open and empty, because the handler only advanced the script pointer to the lone `E5`; it now jumps to entry `0x0F` with the game's own script-jump routine (like the "No" answer). Blue Moon not yet checked in-game (the smoke-test save edit loads an empty room there) |

### Chips and Mystery Data

New chip sources, chip drops and chip art.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_d3_bass_both_chips.asm` | WP-D3 | A Bass battle gives both versions' chips. | A Bass battle also grants the other version's chip (Bass ↔ BassAnly, BlakBarr ↔ DeltaRay) | **PASS in-game** (Bass V1 and BassΩ: both Library flags set, both pack counts +1) |
| `wp_d2_random_version_drops.asm` | WP-D2 | Drops that differ between Red Sun and Blue Moon can come from either version. | Drops that differ by version come from either version's table, 50/50 | **PASS in-game** (Billy on BM, 30 seeds: vanilla 6 P / 0 L; patched 4 P / 4 L) |
| `wp_d2_zsaver_md.asm` | WP-D2 | A Blue Mystery Data in an Undernet 1 dead end holds Z-Saver Z. | Z-Saver Z in a Blue Mystery Data in an Undernet 1 dead end (flag `0x0E04`) | **PASS in-game** (Gemini, `7c0f9b8`) |
| `wp_d2_gpp_md.asm` | WP-D2 | A Blue Mystery Data in ACDC Area 2 holds Grand Prix Power G. | Grand Prix Power G in a Blue Mystery Data in ACDC Area 2 `(90,01)` at (106, 326), flag `0x0D0B` (Gemini's `special-chip-md-design.md`, the Z-Saver recipe); the map's NPC list goes to 14 of 16 | **PASS in-game** (opens cleanly, displays `"GP Power G"`, sets `0x0D0B` & Library `0x16C0+0x138`) |
| `wp_b4_gmd_tier.asm` | WP-B4 | Green Mystery Data follow the Frequency Tuner, so every reward is reachable on any playthrough. | **Green Mystery Data follow the Frequency Tuner:** the GMD content block (Normal / Hard / Super Hard) comes from the Tuner setting (Auto = playthrough as before, Tier 1/2/3 = block 0/1/2), so every GMD reward is reachable on any playthrough. `MysteryDataGetContentTable` (RS `0x080ADF5C` / BM `0x080ADF68`) keeps its `push` and jumps to `0x087FB968` | **PASS in-game** (PT1 ACDC Area 2 GMD `0x0D09`, identical RNG seed: Auto → `300 Zennys`, Tier 3 → `700 Zennys`) |
| `wp_d2_ereader_chips.asm` | WP-D2 | Duo and GP Power show their names and descriptions in the chip screens. | Duo and GP Power names/descriptions come from the text archives: the two chip text routines' e-Reader threshold (`0x137`; RS `0x0802F1BC`/`0x08106C28`, BM `0x0802F1C0`/`0x08106C34`) is raised to `0x1000` (Gemini found the bypass) | **PASS in-game** (`"GP Power G"` verified on screen via BMD) |
| `wp_d2_special_chip_art.asm` | WP-D2 | Duo and GP Power show their own chip art (private builds only; blank in the shareable patch). | Chip art for **Duo** (`0x137`) and **Grand Prix Power** (`0x138`), which no ROM has: `build.py` converts the owner's local `ref/chip-art/` PNGs with `tools/chipart.py` (picture, palette, and a 16×16 icon made from the picture on the Z-Saver icon frame), places them at `0x087FAA70` (0xBC0 bytes) and repoints both records' icon, picture and palette pointers (EWRAM today). **Private builds only:** skipped by `--public` or when the PNGs are missing | Built, bytes and pointers checked in both builds; not yet seen in-game (the names are still blank) |

### Infrastructure and fixes

Nothing new to see; these make the other packages possible or fix a bug.

| File (`src/common/`) | Package | Player sees | Technical detail | Status |
|---|---|---|---|---|
| `wp_a1_woodman_fix.asm` | WP-A1 | WoodMan's scenario works (Prof. 9's fix). | WoodMan scenario fix (Prof. 9's), byte-identical to his IPS | Built; matches the official fix |
| `wp_b6_waiting_room_guard.asm` | WP-B6 | A bad traded-Navi slot no longer corrupts the save. | **Waiting Room Guard A:** bounds-checks the incoming traded-Navi slot `r6` (`1 <= r6 <= 7`) at `WaitingRoomReceiveTradedNavi` (RS `0x0803FA90` / BM `0x0803FA98`), returning `r0 = 1` (error path without saving) on out-of-range slots | Built; RS & BM disassembly verified |
| `wp_d0_npc_lists.asm` | infrastructure | Nothing visible: Higsby's and ACDC Town can hold more NPCs. | Higsby's and ACDC Town's NPC lists move to free space, with room for 16 | Built; byte-identical copies; boots |
| `wp_d0_sprite_lazy.asm` | infrastructure | Nothing visible: Higsby and Sal draw correctly without white circles. | On-demand LZ77 sprite decompression (`0x08002894` via `VENEER_SPRITE_LAZY` `0x0800178C`) for Higsby (`0x08`) & Sal (`0x2C`) into overworld `BN4_SPRITE_CACHE` (`0x0200F720`) via BIOS `swi 0x11` | **PASS in-game** (Higsby in DenDome Hall; Sal in Higsby's & Castillo WaitRm — zero white circles) |

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
