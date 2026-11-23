# Friendly Matches: Updated Patch Design for `FC 0E` (Round 11, Task 6, 2026-11-23)

Updated drop-in patch specification for *Violet Eclipse*'s **9 friendly tournament matches**, re-issued for **`FC 0E`** (`FC 0D` is now used by `wp_d3_duo_rematch.asm`), with verified `0xE0..0xEF` talk-index scan, exact `OpenMapDialogue` redirect, and TextPet `patch/src/scripts/new/` archive wiring.

---

## 1. Venue Maps, Placements, Windows & Sprites (V, R)

Unchanged from `friendly-matches-design.md` §1–§3 (all RS & BM counts and addresses verified):
- **T0 Den Battle — DenDome Hall `(02, 01)`:** 6 → 9/16 NPCs (`RS slot 0x080593D8 / list 0x0805B024`; `BM slot 0x080593E4 / list 0x0805B030`).
- **T1 Eagle / Hawk — Castillo WaitRm `(03, 02)`:** 5 → 8/16 NPCs (`RS slot 0x0805D794 / list 0x0805FE0C`; `BM slot 0x0805D7AC / list 0x0805FE24`).
- **T2 Red Sun / Blue Moon — Colosseum Lobby `(04, 06)`:** 8 → 11/16 NPCs (`RS slot 0x08062B50 / list 0x08066770`; `BM slot 0x08062B64 / list 0x0806677C`).
- **Window gating:** Native NPC opcodes `0x42`/`0x43` (`JumpIfTournamentRound`) for `r = 0..1` (before Final `r = 2`), plus `0x04` (`JumpIfFlagSet`) on each Navi's own Soul flag (`0x0017..0x0022`) — **no extra "friendly done" flags needed**.
- **JunkMan sprite (Match 9, Colosseum Lobby):** Use uncompressed **`list[7][0x08]`** (`.byte 0x25, 0x08, 0x1C`, glowing blue Mystery Data / signal crystal — confirmed working in-game, never a white circle!) with `.byte 0x16, 0`.

| # | Operator & Navi | Venue Map | Window | Hide Flag | Talk Idx | `FC 0E` Args (`<enemy_id> <soul_idx>`) |
|---:|---|---|---|---|---|---|
| 1 | **Dex & GutsMan** | DenDome Hall `(02,01)` | `t=0, r=1` *(after AquaMan)* | `0x0018` | `0xE0` | `FC 0E 84 01` |
| 2 | **Mr. Match & FireMan** | DenDome Hall `(02,01)` | `t=0, r∈{0,1}` | `0x001A` | `0xE1` | `FC 0E 90 03` |
| 3 | **Higsby & NumberMan** | DenDome Hall `(02,01)` | `t=0, r∈{0,1}` | `0x001E` | `0xE2` | `FC 0E 9C 07` |
| 4 | **Lily & WindMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x0019` | `0xE3` | `FC 0E 88 02` |
| 5 | **Tamako & MetalMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x001F` | `0xE4` | `FC 0E A0 08` |
| 6 | **Sal & WoodMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x0022` | `0xE5` | `FC 0E AC 0B` |
| 7 | **Raika & SearchMan** | Colosseum Lobby `(04,06)` | `t=2, r=0` *(before Chaud)* | `0x001C` | `0xE6` | `FC 0E 8C 05` |
| 8 | **Raoul & ThunderMan** | Colosseum Lobby `(04,06)` | `t=2, r∈{0,1}` | `0x001B` | `0xE7` | `FC 0E 94 04` |
| 9 | **JunkMan** *(signal)* | Colosseum Lobby `(04,06)` | `t=2, r∈{0,1}` | `0x0020` | `0xE8` | `FC 0E A4 09` |

---

## 2. Talk Indices `0xE0`–`0xEF` Collision Scan & Archive Choice (V, R)

- **Full-ROM scan (`BN4_RW_MAP_NPCS` + `BN4_CW_MAP_NPCS`, all 6 RW + 21 CW groups):**
  - **0 vanilla NPCs** across the entire game use any talk index in **`0xE0..0xEF`** (`Any in 0xE0..0xEF: []`).
- **Recommendation: Use a separate second archive (`friendly_dialogue_archive`) rather than growing `lobby_dialogue_archive` to 32 entries.**
  - Why: `lobby_dialogue_archive` is hand-assembled short Yes/No prompts in `wp_d3_lobby_rematches.asm`, whereas the 9 friendly-match dialogues are full multi-page TextPet `.tpl` scripts compiled by `build.py` from `patch/src/scripts/new/friendly_matches.tpl` → `build/text/friendly_matches.bin` (`.incbin FRIENDLY_MATCHES_BIN`). Keeping them separate keeps the lobby package self-contained and lets `friendly_matches.tpl` be edited and recompiled independently.
- **Exact change to `lobby_open_dialogue` (`wp_d3_lobby_rematches.asm:43-56`, RS `0x0804E0A0` / BM `0x0804E0A8`):**
  ```arm
  lobby_open_dialogue:
      push    r4, r5, r14
      mov     r1, r0
      cmp     r1, 0xF0
      bge     @@lobby
      cmp     r1, 0xE0
      blt     @@vanilla
      ldr     r0, =friendly_dialogue_archive  ; compiled from patch/src/scripts/new/friendly_matches.tpl
      sub     r1, 0xE0                        ; 0xE0..0xE8 -> scripts 0..8
      b       @@open
  @@lobby:
      ldr     r0, =lobby_dialogue_archive
      sub     r1, 0xF0                        ; 0xF0..0xFF -> lobby scripts 0..15
      b       @@open
  @@vanilla:
      ldr     r0, =0x02030000
  @@open:
      ldr     r3, =OPEN_CUSTOM_DIALOGUE | 1
      bl      lobby_call_r3
      pop     r4, r5, r15
  ```

---

## 3. Reaching Each Friendly Match's `.tpl` & `FC 0E` Battle/Soul Flow (V)

1. **Source file (committed):** `patch/src/scripts/new/friendly_matches.tpl` (`@archive 0`, `@size 10`: scripts `0..8` for the 9 matches, script `9` = `end` for the `"No"` jump target).
2. **Build step (`build.py`):** Compiles `patch/src/scripts/new/friendly_matches.tpl` → `build/text/friendly_matches.bin` (uncompressed BN4 archive) and passes `-definelabel FRIENDLY_MATCHES_BIN` (or `.incbin "build/text/friendly_matches.bin"`) to armips at `friendly_dialogue_archive`.
3. **Inside each script `0..8`:**
   - Displays the approved pre-battle dialogue from `canon-navis.md` §7, followed by a Yes/No prompt (`No` → jump to script `9` `end`).
   - On `Yes`, emits the raw command **`.byte 0xFC, 0x0E, <enemy_id>, <soul_idx>`** (TextPet `jump` / raw hex command or custom command plugin entry) + `end`.
4. **Handler `friendly_fc_battle` (`ScriptGroup_FC` entry `0x0E` in `lobby_fc_table`):**
   - Reads `r4 = [script+2]` (`enemy_id`) and `r6 = [script+3]` (`soul_idx`).
   - Writes **`0x40 | soul_idx`** into `0x020015EF` (bit 6 = friendly-match marker, distinct from bit 7 `0x80` = lobby/Duo rematch).
   - Calls `INIT_BATTLE_REC`, `SET_BATTLE_PARAMS` with **`r5 = 0x5F`** (bit 7 = **0**, so `BN4_ID_RAISE` runs at the normal story playthrough tier!) and `r4 = 3` (retryable on loss, no Game Over), `BATTLE_ADD_ACTOR(1, 0x01020500 | enemy_id)`, `BATTLE_ADD_ACTOR(0, 0x00020200)`, and `START_OW_BATTLE`.
   - On **victory**, `lobby_results_hook` (`BattleResultsGrantReward`) sees `([0x020015EF] & 0x40) != 0`, grants that Navi's Soul (`SetEventFlag(0x0017 + soul_idx)` + `WriteChipKey(0x0161 + soul_idx)`), and returns.
   - On **loss or flee**, `BattleResultsGrantReward` is skipped and Task 2's `BattleExitCleanup` (`0x08006E20`) clears `0x020015EF = 0`, so the operator stays on the map and can be retried immediately.
