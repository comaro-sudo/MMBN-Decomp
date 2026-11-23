# Special-Chip Blue Mystery Data — Exact Patch Design (2026-11-23)

Exact addresses, table formats, spawn-script bytes, and pickup behavior for adding two Blue Mystery Data (BMD):
1. **Grand Prix Power** (`0x138`, code `G` = `6`) at **ACDC Area 2 `(90, 01)`**, `(106, 326, 0)`, flag **`0x0D0B`**;
2. **Z-Saver** (`0x0DA`, code `Z` = `25`) at **Undernet 1 `(94, 00)`**, `(-282, 56, 0)`, flag **`0x0E04`**.

> [!WARNING]
> **E-Reader RAM-chip caveat (`patch-test-results-2.md` §2):** **`Z-Saver` (`0xDA`)** is a normal ROM-backed Mega chip (`flags = 0x42`, ROM art/icon/name) and works immediately. **`Grand Prix Power` (`0x138`)**, like `Duo` (`0x137`), has its `BN4_CHIP_TABLE` art/icon/palette pointers pointed at empty EWRAM (`0x020013E0 / 0x02000E80 / 0x020013C0`, `flags = 0x07`) unless an e-Reader card is loaded — so its chip data/graphics will need to be populated separately for it to display properly when picked up.

**V** = verified in disassembly & ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. Per-Area Mystery Data Tables & Relocation (V)

Both maps' Mystery Data lists are tightly packed in `code_06` (`0x080A9110`–`0x080ADD8C` RS, `+0x0C` BM) and **must be relocated to free space (`0x087FA0A0+`)** by copying the existing entries, appending the new 12-byte BMD record + 4-byte `0` terminator, and patching the 4-byte map pointer slot:

| Map | Master table (`0x080ADD34` RS / `+0xC` BM) | Pointer slot to patch (RS / BM) | Existing list addr (RS / BM) | Existing entries (× 12 B) |
|---|---|---|---|---|
| **ACDC Area 2 `(90, 01)`** | Group `0x90`, map `1` | **`0x080AA434`** / **`0x080AA440`** | `0x080AA6B4` / `0x080AA6C0` | 3 (36 B: PMD `0x0D08`, GMDs `0x0D09`–`0x0D0A`) |
| **Undernet 1 `(94, 00)`** | Group `0x94`, map `0` | **`0x080AC6D0`** / **`0x080AC6DC`** | `0x080AC6F4` / `0x080AC700` | 4 (48 B: BMD `0x0E00`, PMD `0x0E01`, GMDs `0x0E02`–`0x0E03`) |

*(Tip: use `.incbin ROM_IN, OLD_LIST - 0x08000000, N * 12` to copy the existing entries without committing ROM bytes, just like `wp_d0_npc_lists.asm`!)*

### 1.1 12-byte Mystery Data entry record
```arm
; { u8 type (1=BMD), u8 obj_flags (0), u16 flag, u32 ptr_placements, u32 ptr_contents }
; Grand Prix Power (ACDC Area 2):
    .byte 1, 0
    .halfword 0x0D0B
    .word FS_MD_GPP_PLACE
    .word FS_MD_GPP_CONT

; Z-Saver (Undernet 1):
    .byte 1, 0
    .halfword 0x0E04
    .word FS_MD_ZSAV_PLACE
    .word FS_MD_ZSAV_CONT

; Terminator after appended entry:
    .word 0x00000000
```

### 1.2 8-byte Placement record (terminated by `0`)
```arm
; { u8 valid (1), u8 weight (0x20), s16 x, s16 y, s16 z }, then .word 0
FS_MD_GPP_PLACE:
    .byte 1, 0x20
    .halfword 106, 326, 0           ; ACDC Area 2 (106, 326, 0)
    .word 0

FS_MD_ZSAV_PLACE:
    .byte 1, 0x20
    .halfword -282, 56, 0           ; Undernet 1 (-282, 56, 0)
    .word 0
```

### 1.3 8-byte Content records (chip with code; all slots identical for NG+ safety)
Format (`re07-mystery-data.md` §3): `{ u8 kind (1 = Battle Chip), u8 weight (0), u8 pad (0xFF), u8 code (0=A..25=Z, 26=*), u32 chip_id }`, terminated by `.word 0`.
Because `MysteryDataNextIndices` (`0x080ADEB2`) clamps `content_idx` to the last slot on New Game+, putting **2 identical slots** (or 4) guarantees the chip never changes across playthroughs:
```arm
FS_MD_GPP_CONT:                     ; Grand Prix Power (0x138), code G (6)
    .byte 1, 0, 0xFF, 6  : .word 0x00000138
    .byte 1, 0, 0xFF, 6  : .word 0x00000138
    .word 0

FS_MD_ZSAV_CONT:                    ; Z-Saver (0x0DA), code Z (25)
    .byte 1, 0, 0xFF, 25 : .word 0x000000DA
    .byte 1, 0, 0xFF, 25 : .word 0x000000DA
    .word 0
```

---

## 2. Map-Object Spawn Scripts & CW NPC Lists (V)

Each Mystery Data also needs one 4-byte pointer in the map's **CW NPC/object spawn list** (`dat_08029E14` RS / `0x08029E18` BM) pointing to an **11-byte** `MapObj_SpawnMysteryData` bytecode script (verified directly from the existing MD scripts in `(90, 01)` at `0x080784A8` and `(94, 00)` at `0x0808351A`):

| Map | CW NPC pointer slot (RS / BM) | Existing NPC list (RS / BM) | Existing count (cap 16) | After adding 1 BMD |
|---|---|---|---|---|
| **ACDC Area 2 `(90, 01)`** | **`0x0807748C`** / **`0x08077498`** | `0x080780F4` / `0x08078100` | 13 | **14 / 16** (2 free) |
| **Undernet 1 `(94, 00)`** | **`0x08082A00`** / **`0x08082A0C`** | `0x080834B4` / `0x080834C0` | 6 | **7 / 16** (9 free) |

Relocate each CW NPC list to free space (`.incbin ROM_IN, OLD_LIST - 0x08000000, COUNT * 4`, append `.word FS_SPAWN_xxx`, `.word 0xFF`) and place the 11-byte spawn scripts:

```arm
; Exact 11-byte Mystery Data object spawn script (verified from 0x080784A8 & 0x0808351A):
;   08          = init object
;   25 08 1C    = set object flags / Mystery Data behavior
;   29 <fl_lo> <fl_hi> = skip/hide if event flag already set
;   45 <fl_lo> <fl_hi> = MapObj_SpawnMysteryData(flag) -> sets Blue palette, coords, pickup script
;   03          = end / show
FS_SPAWN_GPP:
    .byte 0x08, 0x25, 0x08, 0x1C, 0x29, 0x0B, 0x0D, 0x45, 0x0B, 0x0D, 0x03

FS_SPAWN_ZSAV:
    .byte 0x08, 0x25, 0x08, 0x1C, 0x29, 0x04, 0x0E, 0x45, 0x04, 0x0E, 0x03
```

---

## 3. Pickup Text (V)

**Zero custom text is needed.** When MegaMan interacts with a BMD (`MapObj_InteractMysteryData`, `0x080A84E8`), `MapObj_SpawnMysteryData` (`0x080A89BC`) automatically assigns script index `(content_kind - 1) * 10 + (type - 1)` = **`0`** in the shared Mystery Data text archive (`RS 0x087585A0` / `BM 0x08758030`, byte-identical). Script `0` reads `[text_state+0x54]` (chip ID) and `[text_state+0x58]` (code), prints *"MegaMan got a BattleChip: `<Name> <Code>`!"*, calls `CHIP_GIVE`, and sets the taken flag (`0x0D0B` or `0x0E04`).

---

## 4. Proposed symbol names (`names.txt` format)

```text
0807748C 08077498 data   GEMINI       BN4_CW_NPCS_ACDC2_SLOT           Pointer slot in dat_08029E14[0x10] for ACDC Area 2 (90,01) NPC list (13 entries @ 0x080780F4 RS)
08082A00 08082A0C data   GEMINI       BN4_CW_NPCS_UNDERNET1_SLOT       Pointer slot in dat_08029E14[0x14] for Undernet 1 (94,00) NPC list (6 entries @ 0x080834B4 RS)
```
