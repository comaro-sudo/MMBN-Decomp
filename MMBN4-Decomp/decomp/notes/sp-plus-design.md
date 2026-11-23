# SP+ Boss Difficulty: Exact Patch Design (Round 10, Task 5, 2026-11-23)

Exact drop-in patch specification for **SP+ bosses** (4,095 HP, Attack × 1.5, Speed × 1.25, high Zenny/BugFrag reward; owner decisions in `docs/pending/boss-double-hp.md`, hooks verified in `decomp/notes/sp-plus-research.md`). All RS and BM addresses, flags, registers, and encounter IDs are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`**.

---

## 1. Free Event Flag Allocation (`0x0E5D`–`0x0E72`, 22 Flags) (V, R)

Mystery Data flags end at `0x0E3A` and resume at `0x0E80` (verified across all 17 community saves = `0`). Existing allocations: `0x0E40..0x0E57` (12 Soul Navis × 2-bit lobby tier), `0x0E58..0x0E59` (Frequency Tuner), `0x0E5A..0x0E5C` (Duo 3-bit rematch tier).

**Recommendation for the lobby's 5th step:** Keep the existing 2-bit tier (`0x0E40 + id*2`, states `0..3` = V1/V2/V3/SP) untouched and add **one dedicated "SP+ unlocked" flag per boss** (21 flags total, or 22 including Duo if Duo uses 2-bit + SP+ flag):

| Flag Range | Count | Meaning |
|---|---:|---|
| **`0x0E5D`–`0x0E68`** | 12 | **12 Soul Navis SP+ unlocked** (`0x0E5D + navi_id`, `navi_id = 0..11`). Set when the player S-ranks that Navi's SP (tier 3) in the lobby. |
| **`0x0E69`–`0x0E70`** | 8 | **8 Shared Navis SP+ unlocked** (`0x0E69 + idx`, `idx = 0..7`: TopMan, ShadeMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan). Set when the player S-ranks that Navi's roaming SP (`Ω`) ghost. |
| **`0x0E71`** | 1 | **Bass SP+ unlocked** (set when the player S-ranks `BassΩ 0xE5` or `BassXX 0xE6` in Black Earth 2). |
| *(Duo: `0x0E5A..0x0E5C`)* | *(3 bits)* | Duo already has a 3-bit tier counter (`0..3` = Duo/α/β/Ω, **`4` = Duo SP+**), or use `0x0E72` if Duo's counter is 2-bit (`0x0E5A..0x0E5B`). |

**Why a separate "SP+ unlocked" flag beats widening the 2-bit lobby counter to 3 bits:**
- Zero migration needed for existing `0x0E40..0x0E57` saves.
- Identical unlock mechanism for all 22 bosses (lobby, roaming ghosts, Bass, Duo).
- Leaves `0x0E72..0x0E7F` (14 flags) still free in the gap.

---

## 2. How Each Boss Route Activates SP+ (`[0x020015EE] = 1`) (V, R)

### 2.1 Lobby (12 Soul Navis)
In `lobby_fc_rematch` (`wp_d3_lobby_rematches.asm`), after computing `r7 = tier (0..3)`:
- If `r7 == 3`, call `TEST_EVENT_FLAG(0x0E5D + navi_id)`.
- If non-zero, set **`[0x020015EE] = 1`** (SP+ active) and keep `r7 = 3` (spawns the SP enemy ID `0x7C + soul_idx*4 + 3`, which our `BattleSpawnEnemy` hook upgrades to 4,095 HP!).
- In `lobby_results_hook` (`RS 0x0802174C / BM 0x08021750`), when a lobby rematch ends with **Busting Level S (`BL >= 11`)**:
  - If `tier < 3`: increment 2-bit tier (existing logic).
  - If `tier == 3` and `[0x020015EE] == 0`: call `SET_EVENT_FLAG(0x0E5D + navi_id)` (unlocks SP+ for the next rematch!).

### 2.2 The 8 Shared Navis & Bass (Roaming SP Ghosts — Zero Encounter Table Edits!)
Every shared Navi's SP (`Ω`) ghost and Bass's rematches appear **exclusively** in `BN4_STORY_OFFSET` (`+0x10` / `+0x20`) wandering encounter tables (verified from `re06-encounters.md`):

| Idx | Boss | SP Enemy ID | Roaming Map (`BN4_STORY_OFFSET` `+0x10`) | SP+ Flag |
|---:|---|---:|---|---:|
| 0 | **TopMan** | `0xB3` (`TopMΩ`) | `(91, 00)` Town Area 1 | `0x0E69` |
| 1 | **ShadeMan** | `0xB7` (`ShadMΩ`) | `(94, 03)` Undernet 4 | `0x0E6A` |
| 2 | **BurnerMan** | `0xBB` (`BurnMΩ`) | `(92, 01)` Park Area 2 | `0x0E6B` |
| 3 | **ColdMan** | `0xBF` (`ColdMΩ`) | `(93, 03)` Sharo Area | `0x0E6C` |
| 4 | **SparkMan** | `0xC3` (`SprkMΩ`) | `(90, 02)` ACDC Area 3 | `0x0E6D` |
| 5 | **LaserMan** | `0xD3` (`LasrMΩ`) | `(94, 04)` Undernet 5 | `0x0E6E` |
| 6 | **KendoMan** | `0xD7` (`KendMΩ`) | `(93, 02)` NetFrica Area | `0x0E6F` |
| 7 | **VideoMan** | `0xDB` (`VideMΩ`) | `(92, 02)` Park Area 3 | `0x0E70` |
| 8 | **Bass** | `0xE5` (`BassΩ`) / `0xE6` (`BassXX`) | `(94, 07)` Black Earth 2 (`+0x10` / `+0x20`) | `0x0E71` |

**How roaming SP ghosts trigger SP+ cleanly (and work with LocEnemy too!):**
- Because none of these 10 enemy IDs (`0xB3, 0xB7, 0xBB, 0xBF, 0xC3, 0xD3, 0xD7, 0xDB, 0xE5, 0xE6`) ever appear in story battles, **we do not need to edit any encounter table or hook `BN4_STORY_OFFSET` at all!**
- Instead, in our **Hook 1 (`BattleSpawnEnemy` @ `0x08006802`, RS == BM)**, before checking `[0x020015EE]`:
  - Scan a 10-entry lookup table `{ u8 enemy_id, u8 flag_lo }` (`0xB3->0x69, ..., 0xE5->0x71, 0xE6->0x71`).
  - If `enemy_id` matches and `TestEventFlag(0x0E00 | flag_lo) != 0`, set **`[0x020015EE] = 1`** on the spot!
  - And record `enemy_id` in a 1-byte EWRAM scratch **`0x020015EC`** (`sp_roam_enemy`) so `BattleResultsGrantReward` knows which roaming SP ghost was just defeated!
- In `BattleResultsGrantReward` (`lobby_results_hook`): if `[0x020015EC]` is in the 10-entry table and `BL >= 11` (S rank) and `[0x020015EE] == 0`, call `SetEventFlag(0x0E00 | flag_lo)` (unlocks SP+ for all future encounters with that roaming ghost!). Clear `[0x020015EC] = 0` on battle exit.

### 2.3 Duo (Meteor Comp 1 Rematch)
- In `wp_d3_duo_rematch.asm`, when Duo's 3-bit tier (`0x0E5A..0x0E5C`) == `4` (or `tier == 3` and `0x0E72` set): launch `duo_rematch_rec_v3` (`DuoΩ 0xE3`) and set **`[0x020015EE] = 1`**.
- On S-rank against `DuoΩ` (`tier == 3`, `[0x020015EE] == 0`), advance Duo's tier to `4` (unlocking Duo SP+).

---

## 3. Concrete SP+ Reward Word & Grant Point (V)

Verified in `RewardDecode` (`RS 0x08021A98 / BM 0x08021A9C`):
- Bits 14–15 = kind (`0` chip, `1` Zenny `0x4000 | amt`, `2` HP, `3` BugFrags `0xC000 | amt`).
- Bits 0–13 = 14-bit unsigned amount (**max `16,383`**). Thus `30,000Z` does **not** fit in a single `BN4_BATTLE_REWARD` (`0x0203BD4C`) halfword.
- **Recommended concrete reward word: `0xC032` = 50 BugFrags** (fits natively in `RewardDecode`, displays `"50 BugFrags"` on the vanilla results screen, and matches the owner's `"e.g. 30,000Z or 50 BugFrags"`).
  - *(Alternative Zenny word: **`0x7A98` = 15,000Z**, the largest clean round Zenny figure under `16,383Z`; or `0x7A98` on screen + an extra `AddZenny(15000)` call inside `BattleResultsGrantReward` if 30,000Z is preferred.)*
- **Where it is set:** Hook `BN4_REWARD_PICK` exit (or right at the start of `BattleResultsGrantReward`, **`RS 0x08021684 / BM 0x08021688`**, before `BN4_BATTLE_REWARD` is decoded):
  - When `[0x020015EE] != 0`, write `0xC032` (50 BugFrags) into `[0x0203BD4C]` (`BN4_BATTLE_REWARD`), so the results screen displays AND grants 50 BugFrags natively!
  - *(Note: On SP+ victory in the lobby, keep `lobby_results_hook` still granting any missing Secret Chip if the player somehow didn't get it before, OR skip Secret Chip overwrite when `[0x020015EE] != 0` since the player already S-ranked SP to unlock SP+.)*

---

## 4. Exact Hook Points, Register Usage & Code (RS & BM) (V)

| Hook | RS Address | BM Address | Overwritten Instructions | Registers Live / Safe |
|---|---|---|---|---|
| **1. HP = 4,095 + Roaming SP+ check** | `0x08006802` | `0x08006802` | `ldrh r1,[r7]; adds r2,r1,#0; lsls r2,r2,#0x14; lsrs r2,r2,#0x14` (8 B) | Live: `r4` (actor), `r7` (`BN4_ENEMY_RECORD`), `r8` (`enemy_id`). Scratch: `r0-r3`. |
| **2. Attack × 1.5** | `0x08013A56` | `0x08013A56` | `bl sub_080133CE` (4 B) | Live: `r5` (MegaMan), `r6` (`[r5+0x54]` hit struct, `[r6+0x28]` u16 dmg). Scratch: `r0-r3`. |
| **3. Speed × 1.25** | `0x08003194` | `0x08003194` | `push {r7}; mov lr,pc; bx r0; pop {r7}` (8 B) | Live: `r0` (tick fn), `r5` (battle obj), `r7` (list node). Scratch: `r1-r3` (save `r0` around 1st tick). |
| **4. Reward override + Flag clear** | `0x08021684` / `0x0802174C` | `0x08021688` / `0x08021750` | `BattleResultsGrantReward` entry / `RESULTS_DONE_HOOK` (existing lobby veneer!) | Clear `[0x020015EE] = 0` and `[0x020015EC] = 0` in `lobby_results_hook` AND in battle-exit cleanup (`0x08005478` / `0x080082B0`) so flee/loss also clears SP+. |

### Drop-in Thumb code (RS == BM for Hooks 1–3; veneers in free space or dead padding)
- **Hook 1 (`sp_plus_hp_hook`, called via `bl` veneer from `0x08006802`):**
  ```arm
  sp_plus_hp_hook:
      push    r0, r1, r3, r14
      mov     r0, r8                  ; r0 = enemy_id
      ; Check 10-entry roaming SP table -> if matched, record in 0x020015EC & check SP+ flag -> set 0x020015EE
      ...
      ldrh    r1, [r7]                ; original hp_elem
      lsl     r2, r1, 20
      lsr     r2, r2, 20              ; r2 = 12-bit HP
      ldr     r3, =0x020015EE
      ldrb    r3, [r3]
      cmp     r3, 0
      beq     @@done
      mov     r0, r8
      cmp     r0, 0x80                ; bosses/Navis only
      blt     @@done
      ldr     r2, =0x0FFF             ; 4,095 HP
  @@done:
      pop     r0, r1, r3, r15
  ```
- **Hook 2 (`sp_plus_atk_hook`, called via `bl` veneer from `0x08013A56`):**
  ```arm
  sp_plus_atk_hook:
      push    r14
      ldr     r0, =0x020015EE
      ldrb    r0, [r0]
      cmp     r0, 0
      beq     @@normal
      ldrh    r0, [r6, 0x28]          ; incoming hit damage
      lsr     r1, r0, 1
      add     r0, r0, r1              ; dmg * 1.5
      strh    r0, [r6, 0x28]
  @@normal:
      ldr     r3, =0x080133CF         ; sub_080133CE | 1 (RS == BM)
      bl      @@call_r3
      pop     r15
  @@call_r3:
      bx      r3
  ```
- **Hook 3 (`sp_plus_spd_hook`, called via `bl` veneer from `0x08003194`):**
  - Increment `sp_phase` (`0x020015ED`) once per frame at `BN4_OBJECT_RUNNER` entry (`0x0800314C` RS == BM), or check `[0x020015ED] & 3 == 0`.
  - In `sp_plus_spd_hook`: save `{r0, r7, lr}`, call `bx r0` (1st tick). Then if `[0x020015EE] != 0`, `([0x020015ED] & 3) == 0`, `([r5, #2] & 0x0F) == 1`, `[r5, #0x16] == 1` (enemy side), `([r5, #0] & 1) != 0`, and `[r5, #0x24] != 0` (alive): call `bx r0` a **2nd time**. Pop `{r0, r7, pc}`.

---

## 5. Proposed Names (for `decomp/symbols/names.txt`)

*(All core routines in this design — `BattleSpawnEnemy`, `BattleMegaManApplyHit`, `BN4_OBJECT_RUNNER`, `RewardDecode`, `BattleResultsGrantReward`, `Overworld_TryRandomEncounter`, `BN4_STORY_OFFSET` — are already in `decomp/symbols/names.txt`.)*
