# RE-04 Remainder: Free Tournament, Bracket Draw & Traded-Navi List

Analysis by Gemini (2026-11-22), continuing [`docs/re/tournaments.md`](../../docs/re/tournaments.md).
Status: **V** = verified in disassembly and both US ROMs (`code_03` BM delta = `+0x8`).

## 1. Exact Tournament Soul Navi ID Table (V)

Proven from the Free Tournament secret-chip tables (`dat_08045528` / `dat_080455A8`) and `dat_08044B4C` (Soul index `1..12`):

| Tournament ID | Navi | Version | Story Tournament | Soul Index (`dat_08044B4C`) | Secret Chips (V1–V3) |
|---|---|---|---|---|---|
| `0` (`0x00`) | GutsMan | RS | Den Battle (`0`) | `2` | `GutPnch1`–`3` (`154`–`156`) |
| `1` (`0x01`) | FireMan | RS | Den Battle (`0`) | `5` | `Meteors1`–`3` (`163`–`165`) |
| `2` (`0x02`) | NumberMan | BM | Den Battle (`0`) | `8` | `NumbrBl1`–`3` (`172`–`174`) |
| `3` (`0x03`) | AquaMan | BM | Den Battle (`0`) | `11` | `AquaUp1`–`3` (`181`–`183`) |
| `4` (`0x04`) | Roll | RS | Eagle (`1`) | `1` | `RollAro1`–`3` (`151`–`153`) |
| `5` (`0x05`) | WindMan | RS | Eagle (`1`) | `3` | `PropBom1`–`3` (`157`–`159`) |
| `6` (`0x06`) | MetalMan | BM | Hawk (`1`) | `9` | `MetlGer1`–`3` (`175`–`177`) |
| `7` (`0x07`) | WoodMan | BM | Hawk (`1`) | `12` | `GreenWd1`–`3` (`184`–`186`) |
| `8` (`0x08`) | ThunderMan | RS | Red Sun (`2`) | `6` | `Ligtnin1`–`3` (`166`–`168`) |
| `9` (`0x09`) | SearchMan | RS | Red Sun (`2`) | `4` | `SeekBom1`–`3` (`160`–`162`) |
| `10` (`0x0A`) | ProtoMan | BM | Blue Moon (`2`) | `7` | `HawkCut1`–`3` (`169`–`171`) |
| `11` (`0x0B`) | JunkMan | BM | Blue Moon (`2`) | `10` | `PanlSht1`–`3` (`178`–`180`) |

Generic filler entrants used in the Free Tournament: `0x14` = NormalNavi, `0x15` = HealNavi, `0x1E` = MegaMan.

## 2. Free Tournament (`sub_080448C0` RS / `0x080448C8` BM) (V)

Reached from `ScriptCmd_callFreeTournament` (RS `0x08050500` / BM `0x0805050C`) via `BN4_START_TOURNAMENT(3)`.

### 2.1 Entrant Selection & Seeding (`FreeTournamentSetup`, `0x080448C0`)
1. Clears Free Tournament state at `0x020015E0` (`sub_080448A8`).
2. Builds a temporary 8-entry list at `0x0201A000` (2 bytes per entry `{u8 slot_idx, u8 navi_id}`):
   - Entry 0 is always MegaMan (`{0x00, 0x1E}`).
   - Scans the 7 two-byte entries of the Waiting Room list **`0x020014F0`** (`0xFFFF` = empty) and appends every present traded Navi. Let `N` (`1..8`) be the resulting count.
3. **Shuffling:** If `N > 1`, runs an in-place Fisher-Yates shuffle over indices `N-1..0` using `GetPositiveSignedRNG() % N` (`sub_08001B16`, `svc #6`).
4. **Padding:** Fills the remaining `8 - N` slots alternately with `{0x00, 0x14}` (NormalNavi) and `{0x00, 0x15}` (HealNavi).
5. **Bracket placement (`FreeTournamentSetSlot`, `0x08044974`):** Writes the 8 entrants into **`0x020014C0`** (8 × 4 bytes `{u8 navi_id, u8 status=0x40, u8 waiting_room_slot, u8 pad}`), filling even bracket slots (`0, 2, 4, 6`) first and then odd bracket slots (`1, 3, 5, 7`).

### 2.2 Match Pairing & Advancement (`0x0804498C`, `0x080449E0`, `0x08044AFC`, `0x08044E38`)
- `FreeTournamentPickMatch` (`0x0804498C`): Scans `0x020014C0` for two active (`status & 0x40`) entrants matching the current round's win-mask (`dat_080449DC` = `{0x00, 0x01, 0x03, 0x07}` for rounds 0–3) and writes their indices to `[0x020015E0+2]` and `[0x020015E0+3]`.
- `FreeTournamentRecordWinner` (`0x080449E0`): Sets the current round's win bit (`1 << round`) on the winning entrant's status byte.
- `FreeTournamentClassifyEntrant` (`0x08044AFC`): Returns `0` for traded Soul Navi (`+2` = waiting-room slot), `1` for NormalNavi (`0x14`), `2` for HealNavi (`0x15`), `3` for MegaMan (`0x1E`).

### 2.3 Prize Selection (`FreeTournamentSelectPrize`, `0x08045458`)
Reads MegaMan's defeated opponent `r7` from `0x020014C0` and the battle Busting Level `lv = [0x020015E0 + 0xD]` (`1..11`, where `10`/`11` = Busting 10/S):

| Opponent | Prize Kind (`+0x16`) | Routine / Table (RS / BM) | Rule |
|---|---|---|---|
| NormalNavi (`0x14`) | `1` (Zenny) | `dat_080454C0` / `+8` (`11 × u16`) | `0, 500, 500, 500, 1000, 1000, 1000, 2000, 2000, 2000, 3000` Z by Busting Level `0..10` |
| HealNavi (`0x15`) | `2` (BugFrags) | `dat_080454D6` / `+8` (`11 × u8`) | `0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4` BugFrags by Busting Level `0..10` |
| Traded Soul Navi (`0..11`), Busting 1–9 | `0` (Chip) | `FreeTournamentPickSecretChip` (`0x080454E4`) + `dat_08045528` (`12 × u16` V1 base) | `tier = (lv - 1) / 3` (`0`=V1, `1`=V2, `2`=V3); chip ID = `base[navi] + tier`. Random valid code from chip record `+0..+3` via `GetPositiveSignedRNG() & 3` (5 retries, fallback `+0`). |
| Traded Soul Navi (`0..11`), Busting 10 / S | `0` (Chip) | `FreeTournamentPickMissingSecretChip` (`0x08045540`) + `dat_080455A8` (`12 × u16` V3 top) | Checks Library flags `0x16C0 + chip_id` from **V3 down to V1** (`3` tries), awarding the **highest-tier secret chip not yet in the Library** (falls back to V3 if all 3 are owned). Same random valid code roll. |

All 4 prize tables (`dat_080454C0`, `dat_080454D6`, `dat_08045528`, `dat_080455A8`) are **100% identical** between US Red Sun and US Blue Moon.

## 3. Story Tournament Bracket Draw (`BN4_DRAW_BRACKET`, `0x080442EC` RS / `0x080442F4` BM) (V)

Before `BN4_DRAW_BRACKET`, `BN4_START_TOURNAMENT` calls:
- `TournamentFillGenericPoolA` (`0x0804422C`, table `dat_08044278`): writes the tournament's first 2 generic Navis into `pool[7..8]` (`{0x14, 0x16}` / `{0x18, 0x1A}` / `{0x1C}`).
- `TournamentFillGenericPoolB` (`0x0804428C`, table `dat_080442D8`): writes the next 2 generic Navis into `pool[9..10]` (`{0x15, 0x17}` / `{0x19, 0x1B}` / `{0x1D}`).
- `TournamentCheckSoulPairSwap` (`0x0804480C`): on playthrough 2 (`[progress+0xD] == 1`) when tournament `!= 0` and flag `0x9B` is clear, forces a swap on tournament 2 or flips `GetPositiveSignedRNG() & 1` on tournament 1 and sets flag `0x9B`.

Then `BN4_DRAW_BRACKET` builds the 8-entrant bracket at `0x0200ACC0 + 0xC` (`r7`), maintaining two cursors: left half (`[r6+2]`, starts at 0) and right half (`[r6+3]`, starts at 4):
1. Places MegaMan (`0x1E`) at slot `0` (`left = 1`).
2. **Main seeding (all uses `GetTertiaryRNG`, `sub_08001B60`):**
   - If **flag `0x15` is clear** (first tournament of playthrough 1): flips `GetTertiaryRNG() & 1` to choose whether `pool[0]` (Soul Navi 0) and `pool[2]` (first non-Soul entrant) go to left slot `2` + right slot `4` or vice versa, with `pool[7]` in left slot `1` (MegaMan's round-1 opponent).
   - If **flag `0x15` is set**: rolls `GetTertiaryRNG() & 3` (4 layout cases) plus a second `GetTertiaryRNG() & 1` coin flip to distribute `pool[0]`, `pool[2]`, `pool[7]`, and `pool[9]` across MegaMan's round-1 opponent (slot `1`), left slot `2`, and right slots `4..5`.
3. **Remaining slots:** Gathers unused candidates from `pool[1]` (Soul Navi 1), `pool[3..6]`, `pool[7..8]`, `pool[9..10]` into `0x0200ACC0 + 0x14`, picks one for left slot `3` via `GetTertiaryRNG() & 7` reduced modulo candidate count, and fills the remaining right-half slots from the rest of the pool.

## 4. Who Writes the Traded-Navi List (`0x02006F40`) and Waiting Room (`0x020014F0`) (V)

Every reference to `0x02006F40` (`BN4_TRADED_NAVIS`, 7 × 2 bytes) and `0x020014F0` (Waiting Room list, 7 × 2 bytes):

| RS Address | BM Address | Proposed Name | Role |
|---|---|---|---|
| `0x0803FA90` | `0x0803FA98` | `WaitingRoomReceiveTradedNavi` | **Primary writer (link-trade receive):** receives traded Navi into Waiting Room slot `r6` (`1..7`), maps enemy ID through `dat_0803FBB8`→`dat_0803FBBC` into `0x020014F0[(r6-1)*2 + 1]`, copies `0x40` B stats to `[sl+0x78] + r6*0x40`, `0xE0` B AI/pattern data to `[sl+0x7C] + r6*0xE0`, `0x2C` B names to `0x02001610`/`0x02007230`, and calls `SyncTradedNavisToPool`. |
| `0x0803F948` | `0x0803F950` | `SyncTradedNavisToPool` | Copies all valid (`!= 0xFF`) Navi IDs from `0x020014F0` into `0x02006F40` (`BN4_TRADED_NAVIS`). |
| `0x0803F9B0` | `0x0803F9B8` | `WaitingRoomValidateIncomingNavi` | Validates an incoming link-trade Navi (must be one of the 12 Soul Navis, max 2 copies in `0x020014F0`, no duplicate trainer ID). |
| `0x0803F15C` | `0x0803F164` | `WaitingRoomDeleteEntry` | Deletes Waiting Room slot `idx`: writes `0xFFFF` to both `0x020014F0[idx]` and `0x02006F40[idx]`, clears `[progress+0x70 + (idx+1)*4]`, and saves. |
| `0x0803F450` | `0x0803F458` | `WaitingRoomClearSlotForSend` | Clears `0x02006F40[idx]+1 = 0xFF` when preparing to overwrite/send a Waiting Room slot. |
| `0x0803F980` | `0x0803F988` | `WaitingRoomFillDebugPreset` | Leftover/debug helper that writes 5 preset Navi IDs from `dat_0803F9A8` into `0x020014F0`. |

## 5. Proposed Symbol Names (Merged into `decomp/symbols/names.txt`)

```
0803F15C 0803F164 code   GEMINI       WaitingRoomDeleteEntry           Deletes Waiting Room entry from 0x020014F0 and 0x02006F40 (BN4_TRADED_NAVIS)
0803F450 0803F458 code   GEMINI       WaitingRoomClearSlotForSend      Clears 0x02006F40[idx]+1 to 0xFF before overwriting a Waiting Room slot
0803F948 0803F950 code   GEMINI       SyncTradedNavisToPool            Copies valid Navi IDs from Waiting Room (0x020014F0) to BN4_TRADED_NAVIS (0x02006F40)
0803F980 0803F988 code   GEMINI       WaitingRoomFillDebugPreset       Debug helper: fills 0x020014F0 with 5 Navi IDs from dat_0803F9A8
0803F9B0 0803F9B8 code   GEMINI       WaitingRoomValidateIncomingNavi  Validates incoming link-trade Navi (12 Soul Navis, max 2 copies, unique ID)
0803FA90 0803FA98 code   GEMINI       WaitingRoomReceiveTradedNavi     Link-trade receive: writes 0x020014F0, slots +0x78/+0x7C, names, and syncs 0x02006F40
0804422C 08044234 code   GEMINI       TournamentFillGenericPoolA       Fills tournament pool +7/+8 from dat_08044278
0804428C 08044294 code   GEMINI       TournamentFillGenericPoolB       Fills tournament pool +9/+10 from dat_080442D8
0804480C 08044814 code   GEMINI       TournamentCheckSoulPairSwap      Playthrough 2 (flag 0x9B): swaps Soul Navi pair order on tournaments 1/2
080448C0 080448C8 code   GEMINI       FreeTournamentSetup              Builds/shuffles Free Tournament 8-entrant bracket from 0x020014F0 into 0x020014C0
08044974 0804497C code   GEMINI       FreeTournamentSetSlot            Writes one 4-byte Free Tournament bracket slot in 0x020014C0
0804498C 08044994 code   GEMINI       FreeTournamentPickMatch          Picks next round pairing in 0x020014C0 -> 0x020015E0+2/+3
080449E0 080449E8 code   GEMINI       FreeTournamentRecordWinner       Sets round win bit on the winning Free Tournament entrant
08044AFC 08044B04 code   GEMINI       FreeTournamentClassifyEntrant    Classifies 0x020014C0 entrant: 0=TradedNavi, 1=NormalNavi, 2=HealNavi, 3=MegaMan
08045458 08045460 code   GEMINI       FreeTournamentSelectPrize        Selects Free Tournament prize (Zenny, BugFrags, or Secret Chip by Busting Level)
080454E4 080454EC code   GEMINI       FreeTournamentPickSecretChip     Awards V1/V2/V3 secret chip (dat_08045528 + tier) with random valid code
08045540 08045548 code   GEMINI       FreeTournamentPickMissingSecretChip Busting 10/S: awards highest unowned secret chip (V3->V1) checking Library 0x16C0+id
```
