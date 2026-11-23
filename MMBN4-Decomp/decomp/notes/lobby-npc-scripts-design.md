# Lobby NPCs, Rematch Scripts, Trash Can & JunkMan — Patch Design (2026-11-23)

Implementation design for the 11 Soul Navi operators (9 inside Higsby's `(00, 06)`, 2 on ACDC Town `(00, 00)`), the JunkMan trash can outside Higsby's, and JunkMan's rematch NPC in a small cyber map.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`) & community saves. **T** = tentative (needs 1-frame visual check when placing).

---

## 1. Sprites (`list[7]` humans & `list[6]` JunkMan)

All 11 human operators exist in `list[7]` (`RS 0x08027E80` / `BM 0x08027E84`, 110 entries, identical indices in RS & BM) and JunkMan exists in `list[6]` (`RS 0x08027CE8`):

| Operator | Navi (Tourney ID) | Sprite List & Index | Placement | Status |
|---|---|---|---|---|
| **Mayl** | Roll (`0x04`) | `list[7]` **`0x01`** | Higsby's `(00,06)` | **V** (`Mayl Room (00,03)`) |
| **Dex** | GutsMan (`0x00`) | `list[7]` **`0x02`** | Higsby's `(00,06)` | **V** (`Dex Room (00,04)`) |
| **Higsby** | NumberMan (`0x02`) | `list[7]` **`0x08`** | **Already `NPC[0]` behind counter!** | **V** (`Higsby (00,06)`) — see note below |
| **Chaud** | ProtoMan (`0x0A`) | `list[7]` **`0x04`** | Higsby's `(00,06)` | **T** (main cast `0..4`: Lan, Mayl, Dex, Yai, Chaud) |
| **Mr. Match** | FireMan (`0x01`) | `list[7]` **`0x09`** / **`0x1D`** | Higsby's `(00,06)` | **T** (present in DenDome `(02,00..03)`) |
| **Shuko** | AquaMan (`0x03`) | `list[7]` **`0x14`** / **`0x0A`** | Higsby's `(00,06)` | **T** (present in DomeRoom `(02,02)` / ElecTown) |
| **Lily** | WindMan (`0x05`) | `list[7]` **`0x2E`** / **`0x2F`** | Higsby's `(00,06)` | **T** (Castillo `(03,01..02)`) |
| **Tamako** | MetalMan (`0x06`) | `list[7]` **`0x13`** / **`0x2A`** | Higsby's `(00,06)` | **T** (YumLand `(04,05)` / Castillo) |
| **Sal** | WoodMan (`0x07`) | `list[7]` **`0x17`** / **`0x18`** | Higsby's `(00,06)` | **T** (Castillo `(03,01..03)`) |
| **Raika** | SearchMan (`0x09`) | `list[7]` **`0x11`** / **`0x16`** | ACDC Town `(00,00)` | **T** (Sharo `(04,08)` / SpaceCtr `(04,09)`) |
| **Raoul** | ThunderMan (`0x08`) | `list[7]` **`0x39`** / **`0x12`** | ACDC Town `(00,00)` | **T** (NetFrica `(04,07)`) |
| **JunkMan** | JunkMan (`0x0B`) | `list[6]` **`0x5A`** | Cyber map `(8E,00)` or `(88,00)` | **V** (`0x08486FC8` in both RS & BM) |
| **Trash Can** | *(jack-in object)* | Custom 16×16 or `list[7]` prop | ACDC Town `(-150,-90,0)` | **R** (existing trash cans in `(00,04)` & `(01,00)` are BG tiles, not sprites; append 1 custom sprite or reuse a `list[7]` crate/bin prop) |

- **Higsby's rematch:** Higsby is already `NPC[0]` behind the counter (`RS 0x08056564`), so **do not spawn a duplicate Higsby** — instead, add a **"Battle NumberMan!"** option to Higsby's existing shop/talk dialogue (or place all 9 *other* non-Higsby operators in Higsby's 9 floor spots from `placements.md`, leaving 2 operators outside on ACDC Town = 10 floor/street NPCs + Higsby at the counter = all 11!).

---

## 2. NPC Bytecode & "Met That Navi" Flag Gates (V, R)

Each added operator NPC is a **~26-byte** script appended to the relocated NPC lists (`wp_d0_npc_lists.asm`: `FS_NPC_HIGSBY` and `FS_NPC_ACDC`):

```arm
; Operator NPC bytecode (NpcRunScript 0x080A8598, dispatch dat_080A85D0):
    .byte 0x08                      ; init/enable object
    .byte 0x17, <sprite_idx>        ; sprite index in list[7]
    .byte 0x42, <met_lo>, <met_hi>  ; hide if "met that Navi" flag is CLEAR (jump to end)
    .word @@hidden
    .byte 0x03                      ; show/enable NPC
    .byte 0x18, <talk_script_idx>   ; talk script index in map's dialogue archive
    .byte 0x14                      ; position (s16 x, s16 y, s16 z)
    .halfword <x>, <y>, 0
    .byte 0x16, <facing>            ; 0..7 isometric facing
    .byte 0x36                      ; call standard idle/talkable loop
    .word 0x080A8EE8                ; (BM: 0x080A8EF4)
@@hidden:
    .byte 0x00                      ; end (hidden)
```

- **"Met that Navi" flags (`docs/re/tournaments.md` §4.4):**
  - Vanilla BN4 already tracks **"met in tournament"** flags at **`0x0001`–`0x0012`** (`flag = tournament_id + 1`, so `0x0001..0x000C` correspond to the 12 Soul Navis `0x00..0x0B`: GutsMan=`0x01`, FireMan=`0x02`, NumberMan=`0x03`, AquaMan=`0x04`, Roll=`0x05`, WindMan=`0x06`, MetalMan=`0x07`, WoodMan=`0x08`, ThunderMan=`0x09`, SearchMan=`0x0A`, ProtoMan=`0x0B`, JunkMan=`0x0C`)!
  - Friendly-match scripts (WP-B1) simply set `0x0001 + tournament_id` (win or lose), and the lobby NPCs gate directly on that exact flag!

---

## 3. Rematch Talk Script, Tier Counter, & S-Rank Climb (V, R)

### 3.1 Free event flags for the 24-bit rematch tier counter
- Verified **0 across all 13 community saves** (`check_duo_sprites_flags.py`): **`0x0E40`–`0x0E5F`** (32 contiguous free flags between Black Earth 2 MD flags `0x0E3B` and Elec Tower MD flags `0x0E80`).
- Allocate **2 bits per Soul Navi** (`0x0E40 + navi_id * 2` and `0x0E40 + navi_id * 2 + 1`, total `0x0E40`–`0x0E57`):
  - `00` = **V1**, `01` = **V2**, `10` = **V3**, `11` = **SP**.

### 3.2 Starting the battle, checking S-rank, and awarding the Secret Chip
- **Starting the battle (`BN4_STORY_BATTLES`):**
  - Every Soul Navi has V1/V2/V3/SP records in `BN4_STORY_BATTLES` (`RS 0x080FA900` / `BM 0x080FA90C`, identical in both ROMs).
  - Launch with the **"no automatic tier raise" bit (`+6` bit 7 = `0x80`)** set (using a tiny 12-byte wrapper record in free space or a dedicated ASM helper called by a new `ScriptGroup_FC` sub-command!) so the playthrough count does not override the chosen V1/V2/V3/SP tier.
- **Checking battle result & Busting Level:**
  - `ScriptCmd_checkBattleResult` (`RS 0x0804F3FC` / `BM 0x0804F408`) checks `[game_state + 0x10]` (win / lose / run) — it does **not** expose Busting Level directly.
  - **Best design (1 small `ScriptGroup_FC` command, ~90 bytes total for all 12 operators!):**
    - Add one custom text-script command **`FC <new_idx> <navi_id>`** (extending `dat_08050434`, `code_04`) that handles the entire rematch flow in Thumb:
      1. Reads the 2-bit tier (`0..3`) from flags `0x0E40 + navi_id * 2` (`TestEventFlag`).
      2. Looks up the Navi's 4 `BN4_STORY_BATTLES` IDs `(V1, V2, V3, SP)` from a 48-byte table in free space, sets the no-tier-raise bit, and launches `BN4_START_BATTLE` (`0x08005490`).
      3. On return, checks win (`[game_state + 0x10] == 0`) and reads the battle's **Busting Level** (`lv`, `1..11` where `11 = S-rank`, stored at `[0x020015E0 + 0xD]` / battle results struct).
      4. If **`lv == 11` (S-rank)** and `tier < 3`, increments the 2-bit tier in `0x0E40 + navi_id * 2` (`SetEventFlag` / `ClearEventFlag`).
      5. Calls **`FreeTournamentSelectPrize`** (`RS 0x08045458` / `BM 0x08045460`) with `navi_id` and `lv` so Busting 1–3 gives V1, 4–6 gives V2, 7–9 gives V3, and 10/S gives the **highest missing Secret Chip** (`FreeTournamentPickMissingSecretChip`, `0x08045540`)!
    - This avoids writing 12 complex branching text scripts by hand — each operator's talk script is just **1 short Yes/No prompt + `FC <new_idx> <navi_id>`**!

---

## 4. Minimum Text & Encoding (V, R)

- **Minimum text needed:** Only **1 shared prompt** (*"Want a rematch? Yes / No"*) + optional 1-line greeting per operator (or 1 shared template with operator/Navi name), plus 1 prompt for the trash can (*"A trash can... Jack in? Yes / No"*).
- **Format & insertion:** BN4 text archives (`RS 0x0877D30C` for Higsby's `(00,06)`, `0x0877C568` for ACDC Town `(00,00)`) are **uncompressed** `u16` offset tables followed by custom-encoded byte strings (`0x00=' '`, `0x01..0x0A='0'..'9'`, `0x0B..0x24='A'..'Z'`, `0x26..0x3F='a'..'z'`, `0xE5` newline, `0xE7` end, `0xEE`/`0xFC` script opcodes).
- **No full TextPet rebuild required:** Because each map's dialogue archive is referenced by a single `u32` pointer in `dat_080523E0`, a patch can relocate just Higsby's & ACDC Town's archives to free space (`.incbin` existing bytes + append new `u16` offsets and encoded strings at the end), or assemble them with TextPet (`tools/TextPet`) using the existing BN4 plugin!

---

## 5. Trash Can & JunkMan Cyber Spot (V, R)

- **Trash-can NPC on ACDC Town `(00, 00)`:**
  - Appended to `FS_NPC_ACDC` (`wp_d0_npc_lists.asm`) at **`(-150, -90, 0)`** (`facing = 3`), gated on JunkMan's met flag **`0x000C`** (or always visible post-Den Battle).
  - Talk script asks *"Jack in to the trash can? Yes / No"*; on Yes, executes a warp into **`(8E, 00)`** (`WtrGodComp1`, 1-room Comp, 1 existing NPC) or **`(88, 00)` Lan's HP** (0 NPCs, setting a temporary flag so JunkMan appears only when entered from the trash can) via `sub_0802BD10(dat_0809CDA8, warp_ptr)` (full jack-in animation) or `where_bn4_plays.warp` (`sub_08005572`).
  - Because `sub_08005572` automatically saves Lan's return map `(00, 00)` and exact sidewalk position into `[game_state + 0x48..0x58]` when crossing RW → CW, jacking out returns Lan cleanly in front of the trash can.
- **JunkMan NPC in `(8E, 00)` / `(88, 00)`:**
  - Relocate the target cyber map's CW NPC list (`dat_08029E14`) to free space, append 1 NPC script with `list[6]` sprite **`0x5A`** (JunkMan, `0x08486FC8` in both RS & BM), gated on flag `0x000C` (and temp flag if sharing `(88, 00)`), whose talk script calls the exact same **`FC <new_idx> 0x0B`** rematch handler!
