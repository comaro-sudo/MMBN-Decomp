# WP-B1: Deterministic Tournament & Friendly-Match Design (2026-11-23)

Implementation design for *Violet Eclipse*'s unified story tournaments (`canon-navis.md` §6):
- **1st: Den Battle (`Tournament 0`):** canon story fight **AquaMan** (Shuko); friendlies **GutsMan** (Dex, *after* AquaMan), **FireMan** (Mr. Match), **NumberMan** (Higsby).
- **2nd: Eagle / Hawk (`Tournament 1`):** canon story fight **Roll** (Mayl); friendlies **WindMan** (Lily), **MetalMan** (Tamako), **WoodMan** (Sal).
- **3rd: Red Sun / Blue Moon (`Tournament 2`):** canon story fight **ProtoMan** (Chaud); friendlies **SearchMan** (Raika, *before* Chaud), **ThunderMan** (Raoul), **JunkMan**.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/code_02.s`, `code_03.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. Fixing the Bracket — Exact Patch Points (V, R)

Every story tournament runs `BN4_START_TOURNAMENT(t)` (`RS 0x08043FA0` / `BM 0x08043FA8`), which calls:
1. `TournamentPickSoulPair` (`RS 0x08043FE4` / `BM 0x08043FEC`)
2. `TournamentFillEntrants` (`RS 0x08044058` / `BM 0x08044060`)
3. `TournamentFillGenericPoolA` (`RS 0x0804422C` / `BM 0x08044234`) & `TournamentFillGenericPoolB` (`RS 0x0804428C` / `BM 0x08044294`)
4. `BN4_DRAW_BRACKET` (`RS 0x080442EC` / `BM 0x080442F4`)
5. `BN4_NEXT_OPPONENT` (`RS 0x08044744` / `BM 0x0804474C`)

### 1.1 Lock the Soul Navi Pair (`TournamentPickSoulPair` + `TournamentCheckSoulPairSwap`)

- **Pair tables (`BN4_TOURNAMENT_SOUL_PAIRS`):**
  - RS `0x08044050`: `{00, 01}` (Guts/Fire), `{04, 05}` (Roll/Wind), `{08, 09}` (Thun/Srch)
  - BM `0x08044058`: `{03, 02}` (Aqua/Num), `{06, 07}` (Metl/Wood), `{0A, 0B}` (Proto/Junk)
- **Unified canon pair table (both RS & BM):**
  - Tournament `0` (Den Battle): **`03 00`** (slot `+0` = **AquaMan `0x03`**)
  - Tournament `1` (Eagle/Hawk): **`04 06`** (slot `+0` = **Roll `0x04`**)
  - Tournament `2` (RS/BM): **`0A 09`** (slot `+0` = **ProtoMan `0x0A`**)
- **Disable the pair coin flip (`TournamentPickSoulPair`, `RS 0x0804400C..0x08044038` / `BM +0x08`):**
  - After testing flag `0x15` (`TestEventFlagHL`), vanilla calls `GetTertiaryRNG() & 1` and swaps `pair[0]`/`pair[1]`.
  - **Patch:** Replace the `beq _08044032` / `GetTertiaryRNG` block with an unconditional `b _08044032` (with `r0 = 0`) so `pool[0]` is **always** `pair[0]` (the canon Navi) on every playthrough!
- **Disable the Playthrough-2 pair swap (`TournamentCheckSoulPairSwap`, `RS 0x0804480C` / `BM 0x08044814`):**
  - Vanilla swaps `pool[0]` ↔ `pool[1]` on Playthrough 2 (`[progress+0xD] == 1`) when flag `0x9B` is clear.
  - **Patch:** Return `0` (`movs r0, #0; bx lr` — 4 bytes at `RS 0x0804480C` / `BM 0x08044814`) so New Game+ never swaps the canon Navi out of `pool[0]`.

### 1.2 Make `BN4_DRAW_BRACKET` (`RS 0x080442EC` / `BM 0x080442F4`) Deterministic

In `BN4_DRAW_BRACKET`:
- `r5` points to the tournament pool (`0x020073A0 + t * 0xC`): `pool[0]` = canon Soul Navi, `pool[2]` = non-Soul scenario Navi, `pool[7]`/`pool[9]` = generic/shared round Navis.
- **Vanilla randomness:**
  - When flag `0x15` is clear (1st tournament, PT1): calls `GetTertiaryRNG() & 1` at `RS 0x08044332` to choose whether `pool[0]` goes to **left bracket slot 2** (semifinal vs MegaMan) or **right bracket slot 4** (final vs MegaMan).
  - When flag `0x15` is set (Tournaments 2 & 3, and all of PT2+): calls `GetTertiaryRNG() & 3` at `RS 0x08044340` + a second `GetTertiaryRNG() & 1` to scatter `pool[0]` across Round 1 (slot 1), Semifinal (slot 2), or Final (slot 4).
- **Patch:**
  - Force the deterministic branch at `RS 0x08044330` / `BM 0x08044338`: always branch directly to the layout that puts **`pool[0]` (the canon Soul Navi) in MegaMan's path** (specifically: **Round 1 or Semifinal** for Tournament 0 so Dex/GutsMan can challenge Lan *after* AquaMan and *before* the final, and **Final (right slot 4)** for Tournament 2 so Raika/SearchMan can challenge Lan *before* Chaud/ProtoMan!).
  - Removing the `GetTertiaryRNG` shuffle in `TournamentFillEntrants` (`RS 0x08044058` / `BM 0x08044060`) makes the remaining 6 non-Soul bracket slots deterministic too.

---

## 2. Inserting the 9 Friendly Matches in the Tournament Day Flow (V, R)

### 2.1 How tournament rounds & battles are scripted
In the cutscene/event script engine (`BN4_CUTSCENE_SCRIPT_CMDS`, `RS 0x0802C140` / `BM 0x0802C144`):
- **Opcode `0x51`** (`CutsceneCmd_StoryBattle`, `RS 0x0802CDEE` / `BM 0x0802CDF2`): takes `u8 story_battle_id`, `u8 mode`, calls **`BN4_STORY_BATTLE`** (`sub_08007BEC`, using `BN4_STORY_BATTLES` records — all 12 Soul Navis' V1/V2/V3/SP records exist identically in both ROMs!).
- **Opcode `0x52`** (`CutsceneCmd_StartTournament`, `RS 0x0802CE1E` / `BM 0x0802CE22`): calls `BN4_START_TOURNAMENT(t)`.
- **Opcode `0x53`** (`CutsceneCmd_AdvanceRound`, `RS 0x0802CE32` / `BM 0x0802CE36`): calls `BN4_ADVANCE_ROUND` + `BN4_NEXT_OPPONENT`.
- **Opcodes `0x26` + `0x66`** (`SetEventFlag` + `WriteChipKey`): already grant any Soul inline in 11 bytes (`soul-grant.md` §2.2, `soul-grant-patch-design.md` §1)!

### 2.2 Where to insert each friendly match
Between tournament rounds, Lan returns to the venue's waiting room (`(02, 02)` DomeRoom, `(03, 02)` WaitingRoom, `(04, 03)` ColosseumRm) before walking through the arena door to the next match:
- **Insertion mechanism (zero-risk, repeatable on loss):** Place each tournament's 3 friendly-match operators as **flag-gated NPCs** in the venue's waiting room / lobby during that tournament's story phases:
  - **2–3 friendlies active before Round 1 / Canon fight** (e.g. Mr. Match & Higsby in DenDome before AquaMan; Lily, Tamako & Sal in Castillo before Roll/Final; **Raika/SearchMan**, Raoul & JunkMan signal in Colosseum before **Chaud/ProtoMan**).
  - **Dex / GutsMan** gated on `AquaMan defeated` (canon fight done) and `Den Battle Final not yet finished`, standing right outside/inside DomeRoom `(02, 02)` so he challenges Lan right after AquaMan with the approved dialogue (`canon-navis.md` §7).
- **Friendly-match script sequence (per operator):**
  1. Pre-battle dialogue (`canon-navis.md` §7).
  2. `0x51 <story_battle_id> <no_game_over_mode>` — launches that Navi's existing `BN4_STORY_BATTLES` record (with normal playthrough tier, no game-over on loss so the player can retry immediately).
  3. Set the Navi's **"met"** flag (win or lose, so its operator appears in Higsby's / ACDC Town for post-tournament rematches).
  4. On win: run the **Soul grant** (`0x26 FF 15 00`, `0x26 FF <soul_flag> 00`, `0x66 <chip_key_lo> 01`) + set `friendly_done` flag.

---

## 3. Version Note — Fielding Cross-Version Canon Navis in Both ROMs (V, R)

Because the unified canon lineup mixes **Blue Moon's** (`AquaMan`, `ProtoMan`) and **Red Sun's** (`Roll`):
1. **What BOTH ROMs already have identically (no porting needed):**
   - All 12 Soul Navis' `BN4_STORY_BATTLES` records, battle AI/sprites, overworld sprites (`list[6]`), operator sprites (`list[7]`), and 10 of 12 Soul-grant cutscene scripts (`soul-grant.md` §2.2).
2. **What differs and must be patched per ROM:**
   - **Den Battle starter Soul tutorial (`code_04` map script):**
     - **Red Sun (`0x08059EB6` / `0x08059EBA`):** currently hardcodes `GutsSoul` (`0x18`, key `0x162`). Must be patched to **`AquaSoul` (`0x21`, key `0x16B`)** so Den Battle's canon AquaMan tutorial grants AquaSoul!
     - **Blue Moon (`0x08059EC2` / `0x08059EC6`):** already grants `AquaSoul` (`0x21`, key `0x16B`) — untouched!
   - **`BN4_TOURNAMENT_SOUL_PAIRS` (`RS 0x08044050` / `BM 0x08044058`):**
     - **Red Sun:** change `{00,01}, {04,05}, {08,09}` → **`{03,00}, {04,06}, {0A,09}`** (imports AquaMan & ProtoMan as canon).
     - **Blue Moon:** change `{03,02}, {06,07}, {0A,0B}` → **`{03,00}, {04,06}, {0A,09}`** (imports Roll as canon).
   - **Scenario scripts & text for cross-version canon Navis:**
     - **Red Sun** needs AquaMan's & ProtoMan's pre-match tournament scenarios/text (if running full scenarios; or simplified tournament-day cutscenes calling their existing `BN4_STORY_BATTLES` records + existing Soul-grant scripts).
     - **Blue Moon** needs Roll's Castillo tournament scenario/text (same note — Roll's battle record, sprites, and Soul-grant cutscene at `BM 0x0809347D` are already in the Blue Moon ROM!).

---

## 4. Proposed symbol names (`names.txt` format)

```text
0802CDEE 0802CDF2 code   GEMINI       CutsceneCmd_StoryBattle          Cutscene opcode 0x51: launches BN4_STORY_BATTLES record via BN4_STORY_BATTLE (sub_08007BEC)
0802CE1E 0802CE22 code   GEMINI       CutsceneCmd_StartTournament      Cutscene opcode 0x52: calls BN4_START_TOURNAMENT(t)
0802CE32 0802CE36 code   GEMINI       CutsceneCmd_AdvanceRound         Cutscene opcode 0x53: calls BN4_ADVANCE_ROUND + BN4_NEXT_OPPONENT
```
