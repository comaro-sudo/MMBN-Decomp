# Friendly Matches: Exact Patch Design (Round 10, Task 4, 2026-11-23)

Exact drop-in patch specification for *Violet Eclipse*'s **9 friendly tournament matches** (3 per tournament; owner decisions in `decomp/notes/canon-navis.md` §6–§7 and `docs/pending/free-tournament-secret-chips.md`). All RS and BM addresses, NPC list counts, opcodes, sprites, and battle records are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`**.

---

## 1. Venue Maps & Relocated NPC Lists (V, R)

All three chosen tournament-hub maps have **identical vanilla NPC counts in RS and BM** and sit directly on the player's mandatory tournament path (waiting rooms / main halls). Like `wp_d0_npc_lists.asm`, each map's tightly packed ROM list is copied at build time (`.incbin ROM_IN, <LIST> - 0x08000000, <COUNT> * 4`), followed by our 3 friendly-match NPC pointers and `.word 0xFF`.

| Tournament | Venue Map | Map ID | Vanilla NPCs (RS / BM) | After +3 | RS Slot / List | BM Slot / List | Relocate? |
|---|---|---|---:|---:|---|---|---|
| **T0: Den Battle** | **DenDome Hall** | `(02, 01)` | 6 / 6 | **9 / 16** | `0x080593D8` / `0x0805B024` | `0x080593E4` / `0x0805B030` | **Yes** (`FS_NPC_DENDOME_HALL`) |
| **T1: Eagle / Hawk** | **Castillo Waiting Room** | `(03, 02)` | 5 / 5 | **8 / 16** | `0x0805D794` / `0x0805FE0C` | `0x0805D7AC` / `0x0805FE24` | **Yes** (`FS_NPC_CASTILLO_WAIT`) |
| **T2: Red Sun / Blue Moon** | **Colosseum Lobby** | `(04, 06)` | 8 / 8 | **11 / 16** | `0x08062B50` / `0x08066770` | `0x08062B64` / `0x0806677C` | **Yes** (`FS_NPC_COLOSSEUM_LOBBY`) |

*(Why `(03,02)` instead of `(03,00)` Castillo Entrance: `(03,00)` has 10 NPCs in RS vs 8 in BM, whereas `(03,02)` has 5/5 in both versions and is the exact tournament waiting room. Why `(02,01)` instead of `(02,00)` DenDome: `(02,00)` already uses 13/16 slots, which would hit 16/16 with zero headroom, whereas `(02,01)` has 6/16.)*

---

## 2. Opening & Closing the Window: Native NPC Bytecode Opcodes (V)

No custom code hook is needed to open or close the friendly-match window. BN4's NPC script interpreter (`NpcRunScript`, `RS 0x080A8598 / BM 0x080A85A4`) already has native opcodes for exact tournament round and event-flag gating:

| Opcode | Handler (RS / BM) | Format | Semantics |
|---|---|---|---|
| `0x04` | `0x080A8732` / `0x080A873E` | `.byte 0x04; .hword flag; .word target` | **`JumpIfFlagSet`**: jump to `target` if `TestEventFlag(flag) != 0` |
| `0x05` | `0x080A8750` / `0x080A875C` | `.byte 0x05; .hword flag; .word target` | **`JumpIfFlagClear`**: jump to `target` if `TestEventFlag(flag) == 0` |
| `0x42` | `0x080A8C80` / `0x080A8C8C` | `.byte 0x42, t, r; .word target` | **`JumpIfTournamentRound`**: calls `0x08043F4C(t)`; jumps if active tournament `t` (`0..2`) is at round `r` (`0` = R1, `1` = Semifinal, `2` = Final; returns `0xFF` when tournament `t` is not active) |
| `0x43` | `0x080A8C9C` / `0x080A8CA8` | `.byte 0x43, t, r; .word target` | **`JumpIfTournamentRoundNotEqual`**: jumps if tournament `t` is NOT at round `r` |
| `0x03` | `0x080A8724` / `0x080A8730` | `.byte 0x03` | **Hide / End**: terminates NPC script without spawning |

### Window rules per tournament (all before the Final `r = 2`)
In `wp_b1_tournaments.asm`, the canon Soul Navi is always **Round 0 (`r = 0`)**, the semifinal is **Round 1 (`r = 1`)**, and the Final is **Round 2 (`r = 2`)**:
- **Before the canon fight (Round 0 only):** `.byte 0x43, t, 0; .word @@hide` — open during Round 0, closes as soon as Round 0 ends (or stays open into Round 1 if we allow `r = 0` or `r = 1`, see below).
- **After the canon fight, before the Final (Round 1 only):** `.byte 0x43, t, 1; .word @@hide` — opens right after the canon Soul fight (`AquaMan` / `Roll` / `ProtoMan`) finishes and closes when the Final (`r = 2`) begins.
- **Anytime before the Final (`r = 0` or `r = 1`):**
  ```arm
  .byte 0x42, t, 0; .word @@check_done   ; open in Round 0
  .byte 0x42, t, 1; .word @@check_done   ; open in Round 1
  .byte 0x03                             ; hidden outside R0..R1 (incl. Final & post-tournament)
  ```
- **Dex / GutsMan (owner beat):** *after* AquaMan → gate on **`(t = 0, r = 1)`** (OR `0x42 0 1` + `0x05 0x0021 @@hide` confirming AquaSoul `0x0021` is already obtained).
- **Raika / SearchMan (owner beat):** *before* Chaud / ProtoMan → gate on **`(t = 2, r = 0)`** + **`0x04 0x001D @@hide`** (hidden once ProtoSoul `0x001D` is obtained, ensuring he appears strictly before the Chaud fight).

### Does a win need a separate "friendly done" flag?
**No — gating on the Navi's own Soul flag (`0x0017`–`0x0022`) is sufficient and cleaner!**
- Each friendly match grants that Navi's Soul (`FC 0C <idx>`) on victory and does **not** set the flag on loss (so losses are automatically retryable).
- Adding `.byte 0x04; .hword <soul_flag>; .word @@hide` at the top of the NPC script makes the operator vanish immediately after you win (and stay vanished on New Game+, since Soul flags carry over).
- Zero new event flags are consumed! *(If the owner ever wanted friendly matches to be replayable on NG+ even when the Soul is already owned, 9 flags could be allocated in `0x0E69..0x0E71`, cleared on NG+; for Violet Eclipse's single-playthrough design, gating on `!<soul_flag>` is exact.)*

---

## 3. All 9 Friendly Matches: Exact Placement, Sprites, Flags & Battles (V, R)

All coordinates below sit in clear walkable space verified against each venue map's existing NPC coordinate ranges (from `BN4_RW_MAP_NPCS`), and all `list[6]` character sprite IDs are the exact verified IDs from `lobby-fixes.md` (JunkMan uses `list[7]` prop `0x08` glowing signal crystal or `list[6]` `0x53` Navi sprite — since JunkMan has no operator, a floating glowing blue signal `list[7] 0x08` via `.byte 0x25, 0x08, 0x1C` in Colosseum Lobby matches his dialogue *"I'm picking up a strange Navi signal right next to us..."*!).

| # | Tournament | Operator & Navi | Venue Map | Window | Hide If Set | Pos `(x, y, z)` & Dir | Sprite Opcode | Soul `FC 0C` idx | Enemy ID (V1) | Talk Idx |
|---:|---|---|---|---|---|---|---|---|---:|---|
| 1 | T0 Den Battle | **Dex & GutsMan** | DenDome Hall `(02,01)` | `t=0, r=1` *(after AquaMan)* | `0x0018` (GutsSoul) | `(-120, 126, 0)`, dir `3` | `.byte 0x17, 0x02` | `1` | `0x84` | `0xE0` |
| 2 | T0 Den Battle | **Mr. Match & FireMan** | DenDome Hall `(02,01)` | `t=0, r∈{0,1}` | `0x001A` (FireSoul) | `(16, 152, 0)`, dir `5` | `.byte 0x17, 0x1F` | `3` | `0x90` | `0xE1` |
| 3 | T0 Den Battle | **Higsby & NumberMan** | DenDome Hall `(02,01)` | `t=0, r∈{0,1}` | `0x001E` (NumberSoul) | `(76, 196, 0)`, dir `7` | `.byte 0x17, 0x06` | `7` | `0x9C` | `0xE2` |
| 4 | T1 Eagle/Hawk | **Lily & WindMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x0019` (WindSoul) | `(36, -16, 0)`, dir `5` | `.byte 0x17, 0x23` | `2` | `0x88` | `0xE3` |
| 5 | T1 Eagle/Hawk | **Tamako & MetalMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x001F` (MetalSoul) | `(20, 24, 0)`, dir `7` | `.byte 0x17, 0x21` | `8` | `0xA0` | `0xE4` |
| 6 | T1 Eagle/Hawk | **Sal & WoodMan** | Castillo WaitRm `(03,02)` | `t=1, r∈{0,1}` | `0x0022` (WoodSoul) | `(-40, 20, 0)`, dir `3` | `.byte 0x17, 0x20` | `11` | `0xAC` | `0xE5` |
| 7 | T2 RS/BM | **Raika & SearchMan** | Colosseum Lobby `(04,06)` | `t=2, r=0` *(before Chaud)* | `0x001C` (SearchSoul) | `(-12, -108, 0)`, dir `3` | `.byte 0x17, 0x28` | `5` | `0x8C` | `0xE6` |
| 8 | T2 RS/BM | **Raoul & ThunderMan** | Colosseum Lobby `(04,06)` | `t=2, r∈{0,1}` | `0x001B` (ThunderSoul) | `(-64, 16, 0)`, dir `3` | `.byte 0x17, 0x22` | `4` | `0x94` | `0xE7` |
| 9 | T2 RS/BM | **JunkMan** *(signal)* | Colosseum Lobby `(04,06)` | `t=2, r∈{0,1}` | `0x0020` (JunkSoul) | `(56, 36, 0)`, dir `5` | `.byte 0x25, 0x08, 0x1C` | `9` | `0xA4` | `0xE8` |

*(Coordinates should be given a quick visual check on `mmbn4.exe` after building; each was placed inside the verified walkable bounding box of existing NPCs on that map.)*

---

## 4. Battle Record & Story-Tier Scaling (V)

- **Plain battle launch:** Add one new `ScriptGroup_FC` sub-command **`FC 0D <enemy_id>`** (`friendly_fc_battle`, ~30 bytes in free space, reuses `lobby_fc_rematch`'s exact `INIT_BATTLE_REC` + `SET_BATTLE_PARAMS` + `BATTLE_ADD_ACTOR` + `START_OW_BATTLE` sequence) — **with two key differences from the lobby:**
  1. **Normal story tier (`BN4_ID_RAISE` enabled):** Pass `r5 = 0x5F` (bit 7 = **0**, instead of the lobby's `0xDF`) to `SET_BATTLE_PARAMS`. Then `BN4_ID_RAISE` (`0x0800B036`) automatically scales the V1 enemy ID to V1 on Playthrough 1, α on Playthrough 2, and β on Playthrough 3 — matching the normal story tournament tier!
  2. **Do NOT set `0x020015EF`** (the lobby rematch marker) and **do NOT set `0x020015EE`** (SP+). That way `lobby_results_hook` does not touch lobby tiers or secret chips.
- **Retry on loss, grant Soul on win:** Standard BN4 dialogue battle-result branch (`checkBattleResult` / `FC 0D` returns outcome in script flow, or use `SET_BATTLE_PARAMS` with `r4 = 3` (retry/no game-over on loss, same as lobby) and test the battle result flag `[0x020097F8]` / `jumpIfBattleLost` before executing `FC 0C <soul_idx>`!).
  - Specifically, in BN4 text scripts, **`jumpIfFlagSet`** or **`FC 0D <enemy_id> <soul_idx>`** can handle the post-battle Soul grant directly: have `friendly_fc_battle` record `0x40 | soul_idx` in `0x020015EF` (bit 6 = friendly-match marker, distinct from bit 7 = lobby rematch). When `BattleResultsGrantReward` (`lobby_results_hook`, `RS 0x0802174C / BM 0x08021750`) runs on **victory**, if `([0x020015EF] & 0x40) != 0`, it clears `0x020015EF` and calls `GrantSoul(soul_idx)` (`wp_d4_grant_soul.asm`'s helper: sets `0x0017 + soul_idx` + `WriteChipKey(0x0161 + soul_idx)`). On **loss or flee**, `BattleResultsGrantReward` is skipped and `0x020015EF` is cleared on battle exit — so the player keeps retrying until they win, and on victory they immediately receive the Soul!

---

## 5. Dialogue Archive Pipeline Hook (V)

- **Source file (committed):** `patch/src/text/new/friendly_matches.tpl` (`@archive 0`, `@size 9`, scripts `0..8` containing the 9 approved dialogues from `canon-navis.md` §7, each ending with a Yes/No choice and `FC 0D <enemy_id> <soul_idx>`).
- **Compiled binary (gitignored, built by `build.py`):** `build/text/friendly_matches.bin`, included via `.incbin FRIENDLY_MATCHES_BIN` at `friendly_dialogue_archive`.
- **Extending `lobby_open_dialogue` (`wp_d3_lobby_rematches.asm`):**
  - Currently `talk_idx >= 0xF0` → `lobby_dialogue_archive` (`idx - 0xF0`).
  - Add 4 instructions before `@@vanilla`:
    ```arm
    cmp     r1, 0xE0
    blt     @@vanilla
    ldr     r0, =friendly_dialogue_archive
    sub     r1, 0xE0
    b       @@open
    ```
  - Talk indices `0xE0..0xE8` (indices `0..8` of `friendly_dialogue_archive`) are 100% unused by any vanilla RW map archive (vanilla map dialogue archives peak well below `0xD0`). Zero vanilla text archives are touched, and it is 100% version-neutral across RS and BM!

---

## 6. Proposed Names (for `decomp/symbols/names.txt`)

```
080A8BDA 080A8BE6 code   GEMINI       NpcCmd_JumpIfStoryPhaseInRange   NPC script opcode 0x3B: jumps if story_phase ([sl+0x40]+6) is within [lo, hi]
080A8C80 080A8C8C code   GEMINI       NpcCmd_JumpIfTournamentRound     NPC script opcode 0x42: jumps if tournament t (0..2) is currently at round r (0..2)
080A8C9C 080A8CA8 code   GEMINI       NpcCmd_JumpIfTournamentRoundNe   NPC script opcode 0x43: jumps if tournament t (0..2) is NOT at round r
08043F4C 08043F54 code   GEMINI       TournamentGetRound               r0 tournament (0..2): returns current round (0..2), or 0xFF if not active
```
