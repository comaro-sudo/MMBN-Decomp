# Handoff — Gemini Patch Sprint (Items 1–6) (2026-11-23)

## Goal & Current Status
Owner approved Claude's area override (`patch/`) and recommendations to implement Items 1–6 directly in `MMBN4-Decomp/patch/`. **All 6 items are built, verified on both RS & BM (`patch/build.py` + `--check`), tested live on `mmbn4.exe`, and committed:**

1. **`d0c64ea` — Status screen OAM fix (`wp_d4_status_two_pages.asm`):** Moved 6-byte `dat_0810DABF` (`00 10 20 30 40 50`) to `FS_STATUS_OAM` (`0x087FA0A0`) and repointed `ICON_OAM_PTR` (`RS 0x0810DA7C / BM 0x0810DA90`). Verified 6 icons on Page 1 & 5 on Page 2 (`22565.sav`).
2. **`7c0f9b8` — Z-Saver (`0x0DA`, code Z) BMD (`wp_d2_zsaver_md.asm`):** Added Undernet 1 `(94, 00)` Blue Mystery Data at `(-282, 56, 0)` flag `0x0E04` (`FS_ZSAVER_MD` `0x087FA0A8..0x087FA140`). Verified pickup (`"MegaMan got a BattleChip: 'Z Saver Z'!!"`, pack `0 -> 1`). Duo (`0x137`) and Grand Prix Power (`0x138`) deferred per Claude/owner recommendation.
3. **`bac8ce9` — WP-B1 deterministic tournaments (`wp_b1_tournaments.asm`):** Unified canon pairs `{03,00}, {04,06}, {0A,09}`, disabled pair RNG (`TournamentPickSoulPair`) & PT2 swap (`TournamentCheckSoulPairSwap`), deterministic `BN4_DRAW_BRACKET`, and RS Den Battle tutorial `0x08059EB6` -> AquaSoul (`0x21`/`0x16B`).
4. **`c9d36cf` — Free Tournament 36-secret-chip fix (`wp_d4_free_tournament.asm`):** Bypassed empty Waiting Room gate (`0x080057CE`), added missing-chips-first 7-Soul-Navi selector (`FS_FREE_TOURN_FILL`), and changed `FreeTournamentSetupPlayerMatch` case 0 to spawn standard type-`0x01` ROM enemies (`0x7C + soul_index*4 + tier`). Verified 7 Soul Navis populated with 0 traded Navis and MegaMan vs Roll V3 (1000 HP) in battle.
5. **`ce9d481` — Lobby rematches + JunkMan Trash Can (`wp_d3_lobby_rematches.asm`):** Added 9 operators in Higsby's `(00, 06)`, 2 operators + Trash Can on ACDC Town `(00, 00)`, and JunkMan (`list[6] 0x5A`) in `WtrGodComp1 (8E, 00)`. Hooked `OpenMapDialogue` (`RS 0x0804E0A0 / BM 0x0804E0A8`) for `talk_idx >= 0xF0`, extended `ScriptGroup_FC` with `FC 09 <navi_id>` (rematch launcher via `sub_08005544` + `0x0200B480`) and `FC 0A 00` (Trash Can warp via `sub_08005664`), and hooked `BattleResultsGrantReward` exit (`RS 0x0802174C / BM 0x08021750` via `BASS_PAIR_SPACE + 0x48`) for S-rank tier climb (`0x0E40..0x0E57`) and Secret Chip award. Verified Higsby's spawn, Dex -> GutsMan V1 battle, and Trash Can -> `WtrGodComp1` with JunkMan.
6. **WP-B2 Frequency Tuner (`wp_b2_frequency_tuner.asm`):** Hooked `BN4_ID_RAISE` (`0x0800B050`, identical in RS & BM) to override tier from flags `0x0E58..0x0E59` (`00` Auto, `01` V1, `10` V2, `11` V3), added `FC 0B 00` cycler + always-visible Tuner NPC in Higsby's `(-100, 20, 0)` (slot 14/16). Verified flag `0x0E58` toggles `0 -> 1` in `mmbn4.exe`.

## Key Decisions & Constraints
- **Free space map (`patch/src/common/defines.asm`):** Used `0x087F7400..0x087FAA30` (next free slot: **`0x087FAA30`**).
- **Event flags allocated (verified 0 across all 13 community saves):**
  - `0x0E04`: Undernet 1 Z-Saver BMD taken flag.
  - `0x0E40..0x0E57`: 12 Soul Navis × 2-bit lobby rematch tier (`00` V1, `01` V2, `10` V3, `11` SP).
  - `0x0E58..0x0E59`: WP-B2 Frequency Tuner mode (`00` Auto, `01` V1, `10` V2, `11` V3).
- **EWRAM scratch byte:** `0x020015EF` (1-byte lobby rematch marker `navi_id | 0x80`, cleared on victory in `lobby_results_hook`).
- **Build & verification:** Both `bn4_rs_overhaul.gba` and `bn4_bm_overhaul.gba` build cleanly (`13,738` / `13,736` bytes differ) and pass `python patch/build.py --check`.

## Modified & Relevant Files
- `MMBN4-Decomp/patch/src/common/wp_d4_status_two_pages.asm` (`d0c64ea`)
- `MMBN4-Decomp/patch/src/common/wp_d2_zsaver_md.asm` (`7c0f9b8`)
- `MMBN4-Decomp/patch/src/common/wp_b1_tournaments.asm` (`bac8ce9`)
- `MMBN4-Decomp/patch/src/common/wp_d4_free_tournament.asm` (`c9d36cf`)
- `MMBN4-Decomp/patch/src/common/wp_d3_lobby_rematches.asm` (`ce9d481`)
- `MMBN4-Decomp/patch/src/common/wp_b2_frequency_tuner.asm` (Item 6)
- `MMBN4-Decomp/patch/src/common/wp_d0_npc_lists.asm`, `defines.asm`, `main.asm`, `rs/version.asm`, `bm/version.asm`
- `MMBN4-Decomp/TODO.md`

## Open Issues / Follow-ups
- **Operator sprites (cosmetic polish):** Chaud (`0x04`), Mayl (`0x01`), Dex (`0x02`), Higsby (`0x08`), and JunkMan (`0x5A`) are verified exact sprites; the other 7 operator NPCs currently use placeholder `list[7]` indices (e.g. `0x09`, `0x14`, `0x11`, `0x39`, `0x2B` for Trash Can) and can be swapped to exact character sprites / custom trash-can art in a 1-table edit.
- **SP+ tier (`4,095 HP`, 1.5× atk, 1.25× spd):** Lobby rematches currently climb `V1 -> V2 -> V3 -> SP`; extending to SP+ and adding roaming SP+ ghosts for non-operator bosses is next.
- **Duo (`0x137`) & Grand Prix Power (`0x138`):** Deferred until JP ROM art copy + English names approach is settled.
- **9 friendly tournament matches + Soul grant scripts** (`canon-navis.md` §6–§7) and **WP-B4 GMD Frequency Tuner hook** remain open.

## Immediate Next Steps
1. Swap the 7 placeholder `list[7]` sprite indices in `wp_d3_lobby_rematches.asm` to exact operator sprites + Trash Can prop.
2. Add the 9 friendly tournament matches & Soul grant scripts (`soul-grant-patch-design.md`).
3. Decide Duo & Grand Prix Power JP ROM art/name copy approach, and implement SP+ tier + roaming ghosts.
