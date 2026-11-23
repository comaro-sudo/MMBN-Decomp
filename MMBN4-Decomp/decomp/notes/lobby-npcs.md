# Free Tournament lobby, NPC placement format & jack-in PC (2026-11-23)

Static analysis of the Free Tournament lobby map, its NPC list and bytecode format, sprite lists for human operators vs. Navis, jack-in PC registration, and candidate small cyber maps for JunkMan's rematch spot.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`). **T** = tentative.

---

## 1. The Free Tournament lobby map (V, R)

- **Trigger opcode:** `ScriptCmd_callFreeTournament` (RS `0x08050500` / BM `0x08050508`) is **`FC 07`** (`ScriptGroup_FC`, entry 7 of `dat_08050434`), calling `BN4_OPEN_FREE_BRACKET` (`0x080057C8`).
- **Lobby map:** **`(00, 06)` — Higsby's** (RW group `0x00`, map `0x06`).
  - Examine-object dialogue pointer: `dat_080523F8` (`sub_0804E524` RW examine table) → group `0x00` (`0x08052528`) + `6 × 4` (`0x08052540`) → RS archive **`0x0876D368`** (contains `FC 07` at `+0x030B`: the Free Tournament terminal in *"Higsby's free space"*).
  - Standard map dialogue (Higsby's shop & NPCs): `dat_080523E0` → `(00, 06)` `phase_arr @ 0x0805251C` (`0x0877D30C`, `0x0877D944`, `0x0877E0B0`).

---

## 2. NPC spawning & bytecode format (V, R)

### 2.1 How a map loads its NPCs

- On map load, `sub_08029EBC` calls **`sub_0802A3D2`** (`MapSpawnNPCs`, RS `0x0802A3D2` / BM `0x0802A3D6`):
  - Looks up `(group, map)` in the RW NPC table **`dat_08029C94`** (RS `0x08029C94` / BM `0x08029C98`) or CW NPC table **`dat_08029E14`** (RS `0x08029E14` / BM `0x08029E18`).
  - Passes the resulting map NPC pointer list to **`sub_08026748`** (`NpcSpawnList`, RS `0x08026748` / BM `0x0802674C`).
  - `sub_08026748` iterates `u32` script pointers until `0x000000FF`, spawning an overworld object (type `0` via `sub_08004224`) for each pointer and storing the script pointer at `[r5 + 0x60]` (active PC at `[r5 + 0x5C]`).
- **Higsby's `(00, 06)` current NPC list:**
  - Table slot: `dat_08054434 + 6 × 4` = **RS `0x0805444C`** / **BM `0x08054458`**.
  - Points to **RS `0x08056550`** / **BM `0x0805655C`**: **4 NPC pointers** (`0x08056564`, `0x08056658`, `0x080567A8`, `0x08056820` in RS; `+0x0C` in BM) followed immediately by `0x000000FF`, with `NPC[0]`'s bytecode starting right at `0x08056564`.
  - **Must relocate:** The 20-byte list is tightly packed before `NPC[0]`, so it **cannot be extended in place**. To add 11 operator NPCs, build a new 16-word list (4 existing pointers + 11 new operator script pointers + `0x000000FF`) in free space (`0x087F7420+`) and patch the single 4-byte map pointer at **RS `0x0805444C` / BM `0x08054458`**.

### 2.2 NPC bytecode opcodes (`sub_080A8598` → `dat_080A85D0`, `code_06`)

Each NPC is driven by a small bytecode script (interpreted by `sub_080A8598`, RS `0x080A8598` / BM `0x080A85A4`, jump table **`dat_080A85D0`**):

| Opcode | Handler (RS) | Bytes | Meaning |
|---|---|---|---|
| `0x03` | `sub_080A8714` | 1 | Show/enable NPC (clear hidden state) |
| `0x08` | `sub_080A8792` | 1 | Init/enable object (`[r5] = 3`) |
| `0x14` | `sub_080A8836` | 7 | **Set position:** `s16 x, s16 y, s16 z` (shifted `<< 16` into `[r5+0x24..0x2C]`) |
| `0x16` | `sub_080A8878` | 2 | **Set facing:** `u8 dir` (`[r5+0x14]`, `0..7` 8-way isometric) |
| `0x17` | `sub_080A8880` | 2 | **Set sprite index:** `u8 idx` (`[r5+0x3C]`; selects sprite list via `_080A88BC`) |
| `0x18` | `sub_080A88C0` | 2 | **Set talk script index:** `u8 msg_idx` (`[r5+0x1C]`, index in the map's dialogue archive) |
| `0x36` | `sub_080A8B30` | 5 | **Call subscript:** `u32 ptr` (e.g. `0x080A8EE8` = standard idle/talkable NPC handler) |
| `0x3B` | `sub_080A8BDA` | 7 | **Story-phase branch:** `u8 min, u8 max, u32 target` (jump if `story_phase` in `[min, max]`) |
| `0x42` | `sub_080A8C80` | — | **Flag-gated branch** (checks event flag to hide/show NPC; also `sub_080A8468` checks `[r5+0x80]` via `TestEventFlag`) |

Example (Higsby's `NPC[3]` at RS `0x08056820`):
`08` (init) → `17 0C` (sprite `0x0C`) → `3B 12 16 <ptr>` (story phase gate) → `03` (show) → `18 15` (talk script `0x15`) → `14 90FF 4E00 0000` (pos `(-112, 78, 0)`) → `16 07` (facing `7`) → `36 E88E0A08` (idle talkable loop).

### 2.3 Sprite lists (`dat_0802793C` RS / `0x08027940` BM) (V, R)

`BN4_SPRITE_LOAD` (`sub_0800287C`) takes `r1 = list_id × 4` and indexes `dat_0802793C`:
- **`list[6]`** (`0x08027CE8`, **102 entries**, `r1 = 0x18`): overworld **Navis** (MegaMan, Soul Navis including JunkMan, generic Navis — CE's `BN4_OW_*`).
- **`list[7]`** (`0x08027E80`, **110 entries**, `r1 = 0x1C`): overworld **human characters / operators & map objects** (Dex, Mayl, Chaud, Higsby, Mr. Match, Shuko, Lily, Tamako, Sal, Raoul, Raika, etc. — also used by `sub_080A8468` with `r1 = 0x1C`).
- **`list[8]`** (`0x08028038`, 102 entries) & **`list[9]`** (`0x080281D0`, 76 entries): additional overworld/effect sprites.

All 11 human operators' sprites are already in `list[7]` and JunkMan's overworld sprite is already in `list[6]` in **both** RS and BM.

---

## 3. Jack-in PC object & JunkMan's small cyber map (V, R)

### 3.1 How a map gets a jack-in PC

- On map load, `sub_08029EBC` zeroes the 16-entry jack-in target table at **`0x0200B6D0`** (`0x40` bytes = 16 × `u32` warp targets) and runs the map's init script (`dat_08029C4C` for RW maps).
- When Lan presses **R** facing an interactive map tile (`sub_0802A2E8`, RS `0x0802A2E8` / BM `0x0802A2EC`):
  1. `sub_080276F2` returns the facing tile's jack-in slot `r4` (`1..15`).
  2. Tests event flag **`0x1150 + r4`** (`TestEventFlag`).
  3. Reads the warp descriptor from **`0x0200B6D0 + r4 × 4`**: if non-zero, launches the jack-in cutscene **`dat_0809CDA8`** (`sub_0802BD10`), which warps MegaMan into the target cyber map.
- **Alternative (zero map-collision edits needed):** Because Higsby's Shop `(00, 06)` already has the **Free Tournament terminal** (an examine-object tile in `dat_08029C7C[0][6]`, archive `0x0876D368`), a patch can either:
  - **(Simplest)** Add a menu option on the Free Tournament terminal (*"Jack in to Lobby Net"*) or on a talkable PC NPC in Higsby's that calls `where_bn4_plays.warp` (`sub_08005572`) directly into JunkMan's cyber map — no tile-collision editing required; **or**
  - Register a jack-in slot in `0x0200B6D0` + set flag `0x1150 + slot` in Higsby's map-init script (`0x08054E3C`) if an existing R-button tile is present.

### 3.2 Candidate small cyber maps for JunkMan's spot (R)

From `dat_08029E14` (CW NPC table):
1. **`(88, 00)` — Lan's HP** (`NPC list @ 0x0807053C`, **0 NPCs**): tiny 1-room cyber map with a single exit warp and zero existing NPCs — cleanest possible target if we don't mind sharing Lan's HP (or gating JunkMan on a flag so he only appears when entered from Higsby's).
2. **`(8E, 00)`** (`NPC list @ 0x08076518`, **1 NPC**) or **`(8E, 02)`** (`NPC list @ 0x080765D8`, **1 NPC**): tiny single-room Comp maps with only 1 existing NPC and 1 exit.
3. **`(88, 06)` — JomonElec HP** (`0x08070BCC`, 3 NPCs) or **`(88, 07)` — SpaceCtr HP** (`0x08070D54`, 3 NPCs): small single-room HP maps.

*(Best choice: **`(8E, 00)`** or **`(88, 00)`** with its CW NPC list pointer in `dat_08029E14` relocated to free space `0x087F7420+` to add JunkMan's 1 NPC script gated by his "met" flag.)*

---

## 4. What a patch must add for all 12 lobby rematch NPCs

1. **Relocate Higsby's `(00, 06)` NPC list** from RS `0x08056550` / BM `0x0805655C` to free space (`0x087F7420+`): 4 existing pointers + **11 new operator NPC scripts** (each ~32 bytes: flag-gate on "Navi met", sprite index from `list[7]`, `(x, y, z)` in Higsby's free space, facing, talk script index, `0x36 0x080A8EE8`) + `0x000000FF`. Patch the 4-byte pointer at **RS `0x0805444C` / BM `0x08054458`**.
2. **Add JunkMan's 1 NPC script** (using `list[6]` JunkMan overworld sprite, gated on "JunkMan met" flag) by relocating the chosen small cyber map's NPC list pointer in `dat_08029E14` to free space.
3. **Add jack-in entry** from Higsby's `(00, 06)` to JunkMan's cyber map (either via a dialogue option on the Free Tournament terminal / new PC NPC calling `sub_08005572`, or via `0x0200B6D0` + flag `0x1150 + slot`).

---

## 5. Proposed symbol names (`names.txt` format)

```text
0802A3D2 0802A3D6 code   GEMINI       MapSpawnNPCs                     Looks up (group, map) in dat_08029C94 (RW) / dat_08029E14 (CW) and spawns NPCs via sub_08026748
08026748 0802674C code   GEMINI       NpcSpawnList                     r0 ptr to 0xFF-terminated u32 NPC script list: spawns an overworld object for each entry
08029C94 08029C98 data   GEMINI       BN4_RW_MAP_NPCS                  6 RW group pointers -> per-map NPC script lists (Higsby's (00,06) at 0x0805444C -> 0x08056550 RS)
08029E14 08029E18 data   GEMINI       BN4_CW_MAP_NPCS                  21 CW group pointers -> per-map NPC script lists
0802A2E8 0802A2EC code   GEMINI       MapCheckJackIn                   Checks facing jack-in tile (1..15), flag 0x1150+slot, and warp target at 0x0200B6D0+slot*4; runs dat_0809CDA8
080A8598 080A85A4 code   GEMINI       NpcRunScript                     Overworld NPC bytecode interpreter (dispatch table dat_080A85D0: 0x14 pos, 0x16 dir, 0x17 sprite, 0x18 msg, 0x36 call)
```
