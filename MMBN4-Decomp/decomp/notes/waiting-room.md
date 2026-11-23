# Free Tournament waiting room (Option A) & outside trash can (2026-11-23)

Static + runtime analysis (`mmbn4.exe` headless captures outside the repo) of BN4's real-world map table, door/warp mechanics, story warps into Higsby's, the per-map NPC cap, the best candidate waiting room for the 11 Soul Navi operators, and the outside trash can for JunkMan on the ACDC Town sidewalk.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`) and runtime screenshots.

---

## Part A: The Waiting Room (Owner's Option A)

### 1. Real-world map table layout & unused maps (V, R)

Real-world maps are organized into **6 groups (`0x00`–`0x05`)** across all map tables (`dat_08028304` graphics, `dat_08028904` collision, `dat_08029C94` NPCs, `dat_08026604` group map loaders, `dat_080523F8` examine archives). **There are 34 real-world maps total and zero unused/hidden/debug maps** — the graphics and collision tables (`sub_08026144`, 12 bytes/map) have exact group sizes `7, 3, 4, 6, 10, 4` matching the active maps:

| Group | Maps | NPC counts | Notable interiors with 2 doors |
|---|---|---|---|
| `00` ACDC (7) | `(00,00)` ACDC Town, `(00,01)` Lan's House, `(00,02)` Lan's Room, `(00,03)` Mayl's Room, `(00,04)` Dex's Room, `(00,05)` Yai's Room, `(00,06)` Higsby's | 13, 3, 3, 1, 2, 1, 4 | *(none; Higsby's has 1 door)* |
| `01` Elec Town (3) | `(01,00)` Elec Town1, `(01,01)` Elec Town2, `(01,02)` Square | 12, 12, 13 | *(all outdoor)* |
| `02` DenDome (4) | `(02,00)` DenDome, `(02,01)` Alleyway, **`(02,02)` DomeRoom**, `(02,03)` Stadium | 13, 6, **2**, 4 | **`(02,02)` DomeRoom** (2 NPCs, 2 doors) |
| `03` Castillo (6) | `(03,00)` Entrance, `(03,01)` CenterSquare, **`(03,02)` WaitingRoom**, `(03,03)` AirStadium, `(03,04)` MelSquar, `(03,05)` VampirManor | 10, 14, **5**, 5, 11, 4 | **`(03,02)` WaitingRoom** (5 NPCs, 2 doors) |
| `04` Foreign (10) | `(04,00)` Netopia, `(04,01)` ColosseumAve, `(04,02)` Guest Rm, **`(04,03)` ColosseumRm**, `(04,04)` Colosseum, `(04,05)` YumLand, `(04,06)` YumRuins, `(04,07)` NetFrica, `(04,08)` Sharo, `(04,09)` SpaceCtr | 8, 12, 4, **0**, 1, 5, 8, 12, 3, 3 | **`(04,03)` ColosseumRm** (0 NPCs, 2 doors); `(04,02)` Guest Rm has only 1 door |
| `05` NAXA (4) | `(05,00)` ObservRm, `(05,01)` Roof, `(05,02)` NAXA, `(05,03)` NAXALobby | 6, 4, 0, 3 | `(05,02)` is exterior; `(05,00)`/`(05,01)` have 1 door |

### 2. BN4's real per-map NPC limit (V)

- **`sub_08004224`** (`AllocOverworldNPC`, `code_00`) allocates overworld NPC objects from **`0x02006140`..`0x02006EC0`** with stride **`0xD8`** (bitmask at `0x020015D0`):
  $$(0\text{x}02006\text{EC}0 - 0\text{x}02006140) / 0\text{xD}8 = \mathbf{16\text{ NPC slots per map}}$$
- Same 16-NPC hard cap as BN6. Higsby's `(00, 06)` already uses 4 NPCs (4 + 11 = 15/16, very tight), whereas a dedicated waiting room gives plenty of headroom.

### 3. Best candidate room: **`(02, 02)` DomeRoom** (V, R)

All three 2-door waiting rooms were inspected at runtime via `mmbn4.exe` (screenshots saved outside the repo):

| Candidate | Existing NPCs (free / 16) | Visual description (from runtime captures) | Verdict |
|---|---|---|---|
| **`(02, 02)` DomeRoom** | **2** (**14 free**) | **Bright, clean tournament waiting room**: white square-tiled floor, beige walls with dark-red trim, double glass doors at the top-left, a **framed tournament bracket chart on the wall**, U-shaped wooden tables with PETs/cups/BattleChips, folding chairs stacked by the wall, and sinks in the upper-right corner. Wide open floor space all around the tables. | **Best candidate (#1).** Looks immediately like a Free Tournament lounge attached to Higsby's, has 2 doors, ample floor space, and 14 free NPC slots (11 operators fit with 3 slots to spare). |
| `(04, 03)` ColosseumRm | 0 (16 free) | Outdoor-looking weathered stone Roman staircase on a green grass void. | **No** — does not look like an indoor shop waiting room. |
| `(03, 02)` WaitingRoom | 5 (11 free) | Ornate medieval castle hall with blue/gold carpet runner, marble trophy pedestals, wooden throne chairs. | **No** — clashes with Higsby's theme and leaves 0 spare NPC slots (5 + 11 = 16). |

### 4. How doors & warps work, and how to redirect behind a flag (V, R)

- **Warp descriptor (16 bytes):** `{ u8 dest_group, u8 dest_map, u8 warp_type, u8 facing, s32 x_fp16, s32 y_fp16, s32 z_fp16 }`.
  - `warp_type = 0x00` is a normal door fade (`sub_080052E0`); `0x14` is Metroline.
- **Map load:** Each RW group has a loader in `dat_08026604` (RS) / `0x08026608` (BM) that writes the current map's warp-table pointer into `[warp_state + 0x14]`:
  - **Group `00` loader:** RS `0x08054284` / BM `0x08054290` → warp pointer table RS `0x08054180` / BM `0x0805418C`.
    - **`(00, 00)` ACDC Town** (RS `0x0805419C` / BM `0x080541A8`, 6 warps): **Warp 5** (RS `0x080541EC` / BM `0x080541F8`) is the Higsby's street door → `dest=(00, 06), dir=7, pos=(-20, 116, 0)`.
    - **`(00, 06)` Higsby's** (RS `0x0805425C` / BM `0x08054268`, 1 warp): **Warp 0** → `dest=(00, 00), dir=3, pos=(-176, -90, 0)`.
  - **Group `02` loader:** RS `0x08059234` / BM `0x08059240` → warp pointer table RS `0x08059124` / BM `0x08059130`.
    - **`(02, 02)` DomeRoom** (RS `0x08059194` / BM `0x080591A0`, 2 warps):
      - **Warp 0** (bottom-left door): `dest=(02, 01), pos=(-130, -184, 0)`
      - **Warp 1** (top-left double doors): `dest=(02, 03), pos=(4, 150, 0)`
- **Step-on-door trigger (`sub_0800524C`, RS & BM `0x0800524C`):** Reads the collision warp index `r4 = 1..15` via `sub_080276F2`, checks lock flag `0x1150 + r4`, stores `r4` at `[warp_state + 0x12]`, indexes `[warp_state + 0x14] + (r4 - 1) * 16`, and dispatches `dat_080052C0[warp_type]`. When the fade finishes, `where_bn4_plays.warp` (`sub_08005572`) copies that 16-byte entry and loads the destination map.
- **Clean flag-gated redirect (zero tilemap edits):**
  1. Reserve 1 unused event flag `FLAG_IN_HIGSBY_LOUNGE` (cleared on leaving the lounge; set only when entering `(02, 02)` from Higsby's street door).
  2. Hook **Group `00` loader** (`RS 0x08054284` / `BM 0x08054290`) where `[warp_state + 0x14]` is stored: if `map_id == 0x00` (ACDC Town) **and** the Free Tournament / lounge unlock flag is set, store a relocated 6-entry warp table in free space where **Warp 5** points to `(02, 02)` DomeRoom at `(-58, 160, 0)` (its normal entrance coords). Set `FLAG_IN_HIGSBY_LOUNGE` when Warp 5 triggers (or in `sub_0800524C`).
  3. Hook **Group `02` loader** (`RS 0x08059234` / `BM 0x08059240`): if `map_id == 0x02` (DomeRoom) **and** `FLAG_IN_HIGSBY_LOUNGE` is set, store a relocated 2-entry warp table where **Warp 0** → `(00, 00)` ACDC Town `(-176, -90, 0)` (exit to street, clears flag) and **Warp 1** (the double doors) → `(00, 06)` Higsby's `(-20, 116, 0)` (enter Higsby's, clears flag).
  4. Relocate `(02, 02)` DomeRoom's NPC list (`dat_08029C94[2][2]`, RS `0x080593DC` → `0x0805B484`) to free space: keep the 2 vanilla NPCs (gated out when `FLAG_IN_HIGSBY_LOUNGE` is set) and append the **11 operator NPCs** (gated on `FLAG_IN_HIGSBY_LOUNGE` **and** each Navi's "met" flag). Normal DenDome story visits see the original room unchanged!

### 5. Story scripts that warp Lan into Higsby's `(00, 06)` (R)

Full-ROM scan for every 16-byte warp descriptor targeting `(00, 06)` finds **only two** in the entire ROM:
1. **RS `0x080541EC` / BM `0x080541F8`** — ACDC Town `(00, 00)` Warp 5 (the street door, `dir=7, pos=(-20, 116, 0)`).
2. **RS `0x080438B4`** (entry 3 of `dat_08043884`, used by `sub_08043BEE` in `code_03`) — **Free Tournament return warp** (`dir=4, pos=(-64, -56, 0)` right in front of the Free Tournament terminal inside Higsby's).

Because neither `0x080438B4` nor any cutscene script uses ACDC Town's Warp 5 table, redirecting Warp 5 behind the unlock flag leaves **every story cutscene and Free Tournament return warp 100% intact**.

---

## Part B: The Outside Trash Can for JunkMan (Owner's Placement)

- **Map:** **`(00, 00)` ACDC Town** (RW group `0x00`, map `0x00`).
  - Current NPC list: `dat_08029C94[0][0]` = **RS `0x08054434` / BM `0x08054440`** → RS `0x08054F58` (**13 NPCs**, leaving **3 free slots** out of 16 — enough for 1 trash-can NPC with 2 slots to spare).
- **Exact free spot (verified on `mmbn4.exe` captures of `(00, 00)`):**
  - Higsby's door exit lands Lan at **`(-176, -90, 0)`** (`dir = 3`, facing SE toward the squirrel statue).
  - The boy NPC stands on the front-left side of Higsby's (`(-198, -2, 0)` / `(-172, 0, 0)`).
  - On `(00, 00)`'s isometric grid (`-Y` = up-left, `+Y` = down-right, `+X` = up-right along Higsby's storefront), the sidewalk right beside Higsby's **front-right corner under the awning** (between the right glass window and the first bush) is **`(-150, -90, 0)`** (or `(-152, -92, 0)`), completely clear of the door trigger and all 13 existing ACDC Town NPCs.
- **Trash-can sprite check (V, R):**
  - Checked both existing trash cans: **Dex's Room `(00, 04)`** (only 2 NPCs, both human `spr=2, 33`) and **Elec Town 1 `(01, 00)`** (all 12 NPCs are humans/Navi NPCs). Both existing trash cans are **BG map tiles** (examine-only via `dat_08029C7C`), **not** overworld NPC sprites.
  - Therefore, spawning a talkable trash-can NPC outside Higsby's has two clean options:
    1. **Reuse an existing object/prop sprite** from `list[7]` (`0x08027E80`) or `list[8]` (`0x08028038`), **or**
    2. **Append 1 small 16×16 trash-can sprite** (recoloring/reusing the BG trash-can tiles from Dex's Room `(00, 04)`) as a custom sprite entry in free space (`0x087F7420+`), and point the NPC's `op 0x17` at it (zero BG tilemap or collision edits required).
- **Warping into JunkMan's cyber spot:**
  - The trash-can NPC runs a standard talkable NPC loop (`op 0x18 <msg_idx>` + `op 0x36 0x080A8EE8`), gated on the "JunkMan met" flag (or always visible once unlocked).
  - Talking to it opens a short Yes/No dialogue (*"A trash can under Higsby's awning... Jack in?"*) that calls `where_bn4_plays.warp` (`sub_08005572`) — or `sub_0802BD10(dat_0809CDA8, warp_ptr)` for the full jack-in animation — targeting **`(8E, 00)`** (`WtrGodComp5`, 1-room Comp, 1 existing NPC) or **`(88, 00)` Lan's HP** (0 NPCs, JunkMan gated on a temporary "entered from trash can" flag), where JunkMan's `list[6]` (`0x5A`) NPC waits.
  - Because `sub_08005572` automatically saves Lan's return map `(00, 00)` and exact standing position into `[game_state + 0x48..0x58]` when crossing from RW (`< 0x80`) to CW (`>= 0x80`), jacking out returns Lan cleanly to the ACDC Town sidewalk right in front of the trash can!
