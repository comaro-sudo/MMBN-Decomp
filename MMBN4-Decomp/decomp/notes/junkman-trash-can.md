# Trash can & jack-in target for JunkMan (2026-11-23)

Static analysis of all jack-in-able objects in Higsby's `(00, 06)` and surrounding ACDC maps, existing real-world trash cans across BN4, all object Comps in groups `0x80`–`0x8F`, and what it takes to place JunkMan behind a trash can.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`) and all 794 text archives. **T** = tentative.

---

## 1. Summary & recommendation

- **No Trash-Can Comp exists anywhere in BN4** (groups `0x80`–`0x8F` contain 0 trash-can Comps). **R**
- **Higsby's `(00, 06)` has zero jack-in-able objects and no existing trash can.** **R**
- **Nearest existing trash cans (non-jack-in-able examine objects):**
  1. **`(00, 04)` Dex's Room** (`0x0876CA84`): *"It's a trash can"* — right in ACDC Town, one door from Higsby's.
  2. **`(01, 00)` Elec Town 1** (`0x0876D910`): *"The recyclable and regular trash is properly separated"* — one metro stop away.
  3. **`(02, 00)` DenDome** (`0x0876E3CC`) & **`(02, 02)` DomeRoom** (`0x0876EB0C`): trash/burnable garbage cans.
- **Recommended implementation:**
  - **Option A (zero map-tile edits, inside Higsby's):** Add a **talkable trash-can NPC/object** directly into Higsby's relocated NPC list (`lobby-npcs.md`, slot `RS 0x0805444C` / `BM 0x08054458` → free space `0x087F7420+`), using an object sprite from `list[7]` (`0x08027E80`), whose script asks *"Jack in to the trash can?"* and warps MegaMan (`where_bn4_plays.warp`, `sub_08005572`) into **`(8E, 00)`** (1-room Comp, 1 NPC) or **`(88, 00)` Lan's HP** (0 NPCs), where JunkMan waits.
  - **Option B (use existing trash can in Dex's Room `(00, 04)`):** Hook the existing trash-can examine script in `(00, 04)` (`0x0876CA84`) to offer a jack-in warp into JunkMan's cyber map.

---

## 2. Jack-in-able objects in Higsby's and surrounding maps (V, R)

Jack-in slots are checked by `MapCheckJackIn` (`sub_0802A2E8` RS / `0x0802A2EC` BM): facing tile returns slot `1..15`, gated by event flag `0x1150 + slot` and warp descriptor at `0x0200B6D0 + slot × 4`.

| Map `(group, map)` | Area name | Examine archive (RS) | Jack-in unlock flags in map init | Jack-in-able objects & targets | Existing trash can? |
|---|---|---|---|---|---|
| **`(00, 06)`** | **Higsby's** | `0x0876D368` | *(none)* | **None** (Free Tournament terminal, Chip Order, Number Trader, chip displays are examine-only) | **No** |
| **`(00, 00)`** | **ACDC Town** | `0x0876BC3C` | `0x1152`–`0x1156` | Doghouse (`DogHousComp` `(8C, 0D)`), town cyber-maintenance panels | No |
| **`(00, 01)`** | **Lan's House** | `0x0876BF90` | `0x1151`, `0x1152` | Kitchen oven / microwave (`MicrowvComp` `(8C, 00)`) | No |
| **`(00, 02)`** | **Lan's Room** | `0x0876C388` | `0x1151` | Lan's PC (`Lan's HP` `(88, 00)`) | No |
| **`(00, 03)`** | **Mayl's Room** | `0x0876C7F4` | `0x1151` | Electric piano (`Mayl's HP` `(88, 01)`) | No |
| **`(00, 04)`** | **Dex's Room** | `0x0876CA84` | *(always active)* | Dex's PC (`Dex's HP` `(88, 02)`) | **Yes** (*"It's a trash can"*, examine-only) |
| **`(00, 05)`** | **Yai's Room** | `0x0876CE74` | *(always active)* | Yai's PC / portrait (`Yai's HP` `(88, 03)`) | No |
| **`(01, 00)`** | **Elec Town 1** | `0x0876D910` | *(story-gated)* | Electronics / vending displays (`VendMchComp` `(8C, 0F)`, etc.) | **Yes** (separated recycling/trash bin, examine-only) |

---

## 3. All object Comps in groups `0x80`–`0x8F` (R)

Decoded from the map-name table (`RS 0x086E8966`, 14-byte stride):
- **Group `0x80`–`0x82`:** `ElTwrComp1–2`, `ToyRoboComp1–4`, `MeteorComp1–4`, `ControlArea`.
- **Group `0x88` (8 Homepages):** `Lan's HP` `(88,00)` (0 NPCs), `Mayl's HP` `(88,01)`, `Dex's HP` `(88,02)`, `Yai's HP` `(88,03)`, `Hotel HP` `(88,04)`, `Castillo HP` `(88,05)`, `JomonElec HP` `(88,06)`, `SpaceCtr HP` `(88,07)`.
- **Group `0x8C` (16 Comps):** `MicrowvComp`, `StereoComp`, `HotdogComp`, `DomeNBMComp`, `CyberTopComp`, `LCD Comp`, `CastNBMComp`, `StatueComp`, `NupopoComp`, `ComputerComp`, `ToyComp`, `ColoNBMComp`, `LionComp`, `DogHousComp`, `GameComp`, `VendMchComp`.
- **Group `0x8D`–`0x8E`:** `CardComp`, `WaterComp`, `Ticket Comp`, `StandComp`, `AntComp1–4`, `BuddhaComp`, `GoddessComp`, `HeroComp`, `CookComp`, `WtrGodComp1–16` (`(8E, 00)` and `(8E, 02)` have only 1 NPC and 1 exit).

None is a trash-can Comp.

---

## 4. What adding a trash can to Higsby's `(00, 06)` takes

1. **Avoiding BG tilemap & collision edits (recommended):** Spawn the trash can as an **overworld object/NPC** inside Higsby's relocated NPC list (`RS 0x0805444C` / `BM 0x08054458` → free space `0x087F7420+`). Because NPCs carry their own sprite (`op 0x17`, `list[7]`), solid collision footprint, position (`op 0x14`), and A-button interaction (`op 0x18` + `op 0x36 0x080A8EE8`), **no map tileset or collision-map editing is required**.
2. **Jack-in / warp hook:** Talking to (or pressing R at) the trash-can NPC runs a short dialogue (*"A trash can in the corner of Higsby's... Jack in?"*) and calls `where_bn4_plays.warp` (`sub_08005572`) into the target cyber map (or registers a jack-in slot in `0x0200B6D0` + flag `0x1150 + slot` if R-button jack-in is preferred).
3. **Target cyber map for JunkMan:**
   - **`(8E, 00)`** (`WtrGodComp5`, 1 existing NPC, 1 exit) or **`(8E, 02)`** (`WtrGodComp7`, 1 NPC, 1 exit) — tiny single-room cyber maps rarely visited outside AquaMan's scenario.
   - **`(88, 00)` Lan's HP** (0 NPCs, 1 exit) — smallest/cleanest room, if JunkMan is gated by a temporary "entered from trash can" flag so he doesn't appear during normal Lan's HP visits.

---

## 5. Proposed symbol names (`names.txt` format)

```text
080523F8 08052400 data   GEMINI       BN4_RW_EXAMINE_ARCHIVES          6 RW group pointers -> per-map examine-object text archives (Higsby's (00,06) @ 0x08052540 -> 0x0876D368)
08054540 0805454C data   GEMINI       MapInit_ACDCTown                 ACDC Town (00,00) map-init script; unlocks jack-in flags 0x1152..0x1156
08054E3C 08054E48 data   GEMINI       MapInit_Higsbys                  Higsby's (00,06) map-init script (no jack-in flags)
```
