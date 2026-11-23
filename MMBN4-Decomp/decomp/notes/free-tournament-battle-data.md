# Free Tournament battle data: traded Navis vs. story battles (2026-11-23)

Static trace from `FreeTournamentPickMatch` (`0x0804498C`) to battle launch and in-battle stat/AI reads, answering the open question in `docs/pending/free-tournament-secret-chips.md`.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. Bottom-line answer (V)

- **Yes — the vanilla Free Tournament battle against a traded Soul Navi DOES read save slots `+0x78` (stats) and `+0x7C` (AI/folder).** It spawns the opponent as **object type `0x04`** (traded/AI-controlled Navi), NOT as a standard `BN4_STORY_BATTLES` enemy (`0x01`).
- **Story tournament battles (`BN4_STORY_BATTLES`, RE-06) do NOT touch `+0x78` or `+0x7C`.** They spawn the opponent as **object type `0x01`** with a standard enemy ID (`0x80` Roll, `0x84` GutsMan, ..., `0xAC` WoodMan), using `BN4_ENEMY_STATS` and the ROM's built-in Navi AI.
- **Consequence for the two patches:**
  1. **Lobby rematches (owner's primary design):** Need **ONLY a standard `BN4_STORY_BATTLES` record** with the **"no automatic tier raise" bit (`+6` bit 7 = `0x80`)** set (so the rematch tier V1/V2/V3/SP is controlled directly), followed by `FreeTournamentSelectPrize` (`0x08045458`). **No `+0x78` / `+0x7C` data is needed at all.**
  2. **Free Tournament bracket itself (`FreeTournamentSetup`, `0x080448C0`):** If the Free Tournament bracket is also patched to field all 12 Soul Navis without link trades, `sub_08044F80` (MegaMan vs. opponent) must **either** be changed to spawn type-`0x01` standard enemy IDs (just like its NormalNavi/HealNavi branch at `_08044FD8` already does!) **or** `+0x78`/`+0x7C` must be populated with synthetic stats/AI blocks. Changing `sub_08044F80` to type-`0x01` enemy IDs is much smaller and cleaner.

---

## 2. Full call chain: `FreeTournamentPickMatch` → Battle → `+0x78` / `+0x7C` (V)

1. **`FreeTournamentPickMatch` (`sub_0804498C` RS / `0x08044994` BM):**
   - Called from `_080451F8` (RS `0x080451F8`). Picks the two bracket slots for the next match (`[0x020015E0 + 2]` and `[0x020015E0 + 3]`) from `0x020014C0` and transitions to state `4` (`sub_08045208`).
2. **`sub_08045208` (RS `0x08045208` / BM `0x08045210`):**
   - Calls **`sub_08044A8C`** (builds the dynamic battle record in RAM), then launches the battle via **`sub_0800554E`** (`sub_080054EE` with RAM battle record `_08005914`).
3. **`sub_08044A8C` (RS `0x08044A8C` / BM `0x08044A94`):**
   - Initializes the dynamic RAM battle record via `sub_08007C90` + `sub_08007C74(0, 0x64, 0x14, 0x3F, 3, 0xDF, 0)`.
   - Classifies side-0 entrant `[0x020015E0 + 2]` via `FreeTournamentClassifyEntrant` (`sub_08044AFC` → `0` = traded Soul Navi, `1` = Normal/HealNavi, `2` = AI-vs-AI, `3` = MegaMan).
   - Dispatches via **`sub_08044B58`** → `dat_08044B70[r0]`:
     - `[0]` `sub_08044B80` (traded Soul Navi on side 0)
     - `[1]` `sub_08044CF0` (Normal/HealNavi on side 0)
     - `[2]` `sub_08044E38` (other AI side 0)
     - `[3]` **`sub_08044F80`** (**MegaMan on side 0** — the player's match!).
4. **`sub_08044F80` (RS `0x08044F80` / BM `0x08044F88`) — MegaMan's match:**
   - Classifies side-1 opponent `[0x020015E0 + 3]` via `FreeTournamentClassifyEntrant`:
   - **Case `r0 == 0` (traded Soul Navi):**
     - Calls `sub_08044B40` (maps tournament Navi ID `0..11` → Soul index `1..12` via `dat_08044B4C`).
     - Calls **`sub_0800EB98(side=1, r1=waiting_room_slot, r2=soul_index, r3=0)`** — stores `[side*4 + 0] = soul_index`, `[side*4 + 1] = waiting_room_slot` (`1..7`), `[side*4 + 2] = 0` in the battle-side Navi table (`_0800EC3C`).
     - Calls **`sub_08007CB0(slot=1, 0x04020501)`** — spawns **object type `0x04`** (traded AI Navi) at col `5`, row `2`, side `1` (and MegaMan as `0x00020200` at col `2`, row `2`, side `0`).
   - **Case `r0 == 1` (NormalNavi / HealNavi, `_08044FD8`):**
     - Computes `enemy_id = 0xC8 + tier` (`sub_08045068`).
     - Calls **`sub_08007CB0(slot=1, 0x01020500 | enemy_id)`** — spawns **object type `0x01`** (standard enemy ID). **Never calls `sub_0800EB98` and never reads `+0x78` or `+0x7C`.**

---

## 3. Exact routines that read save slots `+0x78` and `+0x7C` (V)

| Save slot | Size per entry | Readers during battle & setup | What it holds |
|---|---:|---|---|
| **`[sl + 0x78]`** | `0x40` bytes × 8 slots (0 = MegaMan, 1..7 = Waiting Room) | `sub_08008708` (`_0800872E`, copies `0x40` bytes of slot stats at battle init), `sub_0800D670` / `sub_0800D772` (`BN4_NAVI_BYTE_SIDE`) / `sub_0800D78A`–`0800D7A2` (`GetNaviStat16`) | Traded Navi stats (HP, attack, rapid, charge, rank, flags). Indexed by `sub_0800EBAA(side)` (`waiting_room_slot`). |
| **`[sl + 0x7C]`** | `0xE0` bytes × 8 slots (0 = MegaMan, 1..7 = Waiting Room) | `sub_080088AE` (battle init), **`_080F5C8E`** (`code_09`: calls `sub_0800EBAA(side ^ 1)`, multiplies by `0xE0`, adds `[sl+0x7C]`, reads the 29-chip folder & AI behavior weights) | Traded Navi's custom folder & learned AI/chip-use profile from the other player's save. |

Both slots are written by `WaitingRoomReceiveTradedNavi` (`0x0803FA90`, `code_03`) during link-cable trades, and cleared/initialized at New Game (`code_00` L12659 / L12672).

---

## 4. Comparison with story tournament battles (`BN4_STORY_BATTLES`) (V, R)

| Feature | Free Tournament traded Navi (`sub_08044F80` case 0) | Story tournament / Lobby rematch (`BN4_STORY_BATTLES`) |
|---|---|---|
| Enemy spawn entry | `0x04020501` (type `0x04` = traded AI Navi) | `0x010205<eid>` (type `0x01` = standard enemy ID `0x80..0xAF`) |
| Reads `[sl + 0x78]` / `[sl + 0x7C]`? | **Yes** (indexed by Waiting Room slot `1..7` via `sub_0800EBAA`) | **No** (reads `BN4_ENEMY_STATS` and ROM Navi AI directly) |
| Tier control | Determined by traded Navi's saved stats in `+0x78` | Controlled by enemy ID (V1/V2/V3/SP) + record `+6` bit 7 (`0x80` = suppress playthrough auto-raise) |
| Works without link-cable trades? | **No** (unless `+0x78`/`+0x7C` are filled or `sub_08044F80` is patched to type `0x01`) | **Yes, out of the box** (all 12 Soul Navis' V1/V2/V3/SP records are identical in both RS and BM) |

---

## 5. Proposed symbol names (`names.txt` format)

```text
08044A8C 08044A94 code   GEMINI       FreeTournamentBuildBattle        Initializes RAM battle record and dispatches to sub_08044B58 for the current Free Tournament match
08044B58 08044B60 code   GEMINI       FreeTournamentSetupSides         Dispatches on side-0 entrant class (dat_08044B70: 0 traded, 1 filler, 2 AI, 3 MegaMan -> sub_08044F80)
08044F80 08044F88 code   GEMINI       FreeTournamentSetupPlayerMatch   Sets up MegaMan's match: case 0 spawns type-0x04 traded Navi (reads +0x78/+0x7C); case 1 spawns type-0x01 filler
0800EB98 0800EB98 code   GEMINI       BattleSetSideTradedNavi          r0 side, r1 waiting_room_slot, r2 soul_idx, r3 flags: writes side's traded Navi descriptor for type-0x04 AI
0800EBAA 0800EBAA code   GEMINI       BattleGetSideWaitingRoomSlot     r0 side: returns waiting_room_slot (0..7) used to index save slots +0x78 (x0x40) and +0x7C (x0xE0)
```
