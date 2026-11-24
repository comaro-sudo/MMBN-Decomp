# Better Props for the Trash Can and Duo's Signal (Round 12, Task 5)

**Date:** 2026-11-23  
**Contact sheets (outside repo):**
- `C:\Users\test\.gemini\antigravity\brain\7e09fd7a-7f26-455f-919d-1f878852d7c9\scratch\r12\contact_list7.png` (all 14 uncompressed `list[7]` sprites)
- `C:\Users\test\.gemini\antigravity\brain\7e09fd7a-7f26-455f-919d-1f878852d7c9\scratch\r12\contact_list6.png` (all 91 uncompressed `list[6]` sprites)

---

## 1. Complete Inventory of Uncompressed Sprites (RS & BM)

Sprite archive pointer tables (`dat_0802793C` RS / `dat_08027940` BM):
- **`list[6]`** (`list_byte = 0x18`, characters/Navis): `RS 0x08027CE8` / `BM 0x08027CEC`, `0x66` entries — **91 uncompressed** (identical ID set in RS and BM).
- **`list[7]`** (`list_byte = 0x1C`, overworld props/effects): `RS 0x08027E80` / `BM 0x08027E84`, `0x6E` entries — **14 uncompressed** (identical ID set in RS and BM):

| `list[7]` ID | RS Sprite Ptr | BM Sprite Ptr | Anims | Visual Description (from contact sheet) |
|---|---|---|---|---|
| `0x06` | `0x08494954` | `0x08494954` | 1+ | Cyan vertical laser beam |
| **`0x07`** | **`0x08494C54`** | **`0x08494C54`** | **2 (`0x00`–`0x01`)** | **Duo's hexagonal green/silver crest with magenta X's and cross (Meteor Comp emblem)** |
| `0x08` | `0x08495D48` | `0x08495D48` | 1 (`0x00`) | Floating cyan Mystery Data crystal (current placeholder) |
| `0x09` | `0x084969E4` | `0x084969E4` | 1+ | Pink horizontal laser security barrier |
| `0x12`, `0x5C`, `0x6B`, `0x6D` | `0x084B5478` | `0x084B5478` | 1 | White circle fallback placeholder (`dat_084B5478`) |
| `0x20` | — | — | 1+ | Tiny off-center green spark/dot |
| `0x45` | `0x084A96FC` | `0x084A9774` | 4 (`0x00`–`0x03`) | Blue double-arrow floor warp panel |
| **`0x4A`** | **`0x084AB1FC`** | **`0x084AB274`** | **2 (`0x00`–`0x01`)** | **Crumpled purple/pink cyber-debris / wreckage pile** |
| `0x53` | — | — | 1+ | Tiny blue spark |
| `0x5D` | — | — | 1+ | Tiny grey star/sparkle |
| `0x67` | — | — | 1+ | Pink/teal angled security gate |

In `list[6]` (91 uncompressed): `0x00`–`0x3F` are human NPCs (plus `0x36` yellow flower), and `0x40`–`0x65` are Navis (including `0x5A` JunkMan and `0x53` tiny grey box/Mettaur-like object; `0x5B`/`0x5D` are the white circle placeholder).

---

## 2. Recommended Picks (RS & BM Identical Opcode Bytes)

### A. Duo's Signal (Meteor Comp 1) — **`list[7] 0x07`** (Duo's Hexagonal Crest)
- **Why:** It is literally Duo's Meteor Comp emblem/console (green-rimmed silver hexagon with magenta X's and central cross), unmistakable as Duo's signal and far more fitting than the generic crystal `0x08`.
- **Sprite address:** `RS 0x08494C54` / `BM 0x08494C54` (uncompressed in both ROMs).
- **Exact NPC script opcode bytes:** `25 07 1C` (`NpcCmd_SetPropSprite(id=0x07, list=0x1C)`).
- **Valid animation:** `0x00` (static/idle crest; `0x01` is also valid — sprite has 2 animations).

### B. ACDC Town Trash Can (JunkMan's Trash Pile) — **`list[7] 0x4A`** (Debris / Wreckage Pile)
- **Why:** Among all 14 uncompressed `list[7]` props, `0x4A` is the only debris/junk-pile graphic (low, crumpled purple/grey wreckage sitting on the ground).
- **Sprite address:** `RS 0x084AB1FC` / `BM 0x084AB274` (uncompressed in both ROMs).
- **Exact NPC script opcode bytes:** `25 4A 1C` (`NpcCmd_SetPropSprite(id=0x4A, list=0x1C)`).
- **Valid animation:** `0x00` (also has `0x01` — 2 animations total).
- **Optional fallback if a larger/taller real-world object is preferred:** `list[6] 0x5A` (JunkMan himself, `RS/BM 0x08486FC8`, 9 anims, opcode `17 5A` or `25 5A 18`, anim `0x00`), or keep `list[7] 0x08` (`25 08 1C`, anim `0x00`).

---

## 3. Can an NPC Script Draw a Compressed Sprite?

**No NPC script opcode can decompress a sprite on the fly.**

Verified in `code_06.s` (`NpcRunScript`, RS `0x080A8598`, opcode table `dat_080A85D0`):
1. The NPC script interpreter has only two sprite-selection opcodes:
   - `0x17 <id>` (`NpcCmd_SetCharSprite`, RS `0x080A8880`): sets `r5[+0x3C] = id`, `r5[+0x78] = 0x18` (`list[6]`).
   - `0x25 <id> <list_byte>` (`NpcCmd_SetPropSprite`, RS `0x080A8980`): sets `r5[+0x3C] = id`, `r5[+0x78] = list_byte`.
2. Neither opcode (nor any other opcode in `dat_080A85D0`) calls `sub_08002A4A` (`SpritePreloadList`) or `sub_08002AEC` (`BN4_SPRITE_PRELOAD`).
3. When the NPC later calls `BN4_SPRITE_LOAD` (`sub_0800287C`, RS == BM), it checks `BN4_SPRITE_CACHED` (`sub_08002ACA`) for `(list_byte << 8) | id` in the 8-slot decompressed cache at `0x02038800`. If not preloaded, any ROM pointer with bit 31 set (`>= 0x80000000`, LZ77-compressed) falls back to the white circle placeholder `dat_084B5478`.
4. **How compressed NPC sprites work in vanilla:** map-init code (`code_04` / `code_05`) calls `sub_08002A4A` with a per-map table of 2-byte `(list_byte, id)` pairs (terminated by `0xFF`) to decompress up to 8 sprites into `0x02038800` before NPC scripts run.
5. **Consequence:** Using an uncompressed sprite (`25 07 1C` / `25 4A 1C`) is strictly safer and requires zero map-preload table edits or EWRAM cache budget.
