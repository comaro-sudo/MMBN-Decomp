# Duo Palette: MegaMan Form Palette Load Hook (Round 11, Task 3, 2026-11-23)

Exact drop-in hook to substitute the 32-byte Duo palette (`duo_soul_pal` from `duo-soul-design.md` §1) **only for form 13 (DuoSoul)** while keeping MetalSoul's sprite (`list[0][9]`), hit-flashes, and normal palette restoration intact. Verified in `decomp/asm/code_00.s` and `code_01.s` (**100% identical in RS and BM**).

---

## 1. How MegaMan's Battle Palette Is Loaded (V)

1. **`sub_0801079C`** (`code_01.s:993`, **RS == BM**) is MegaMan's form-load routine (runs at battle start and on every Double Soul transform / revert):
   ```arm
   080107B6: bl sub_0800B90A   @ BattleGetFormSprite (hooked by wp_d4_duo_soul.asm: form 13 -> list[0][9])
   080107BA: movs r0, #0x80
   080107BC: bl sub_0800287C   @ BN4_SPRITE_LOAD (zeroes sprite_hdr + 0x34!)
   ...
   080107E0: bl sub_0800283C   @ sprite frame init
   080107E4: bl sub_0800BFFE   @ returns palette index r0 (0 for Souls, 1 for Dark)
   080107E8: bl sub_08002DEC   @ BN4_SPRITE_PALETTE_SET: stores r0 -> [sprite_hdr + 4], leaves r3 = sprite_hdr!
   ```
2. **Built-in custom palette override in `sub_0800295C`** (`code_00.s:5290`, **RS == BM** — the sprite OBJ palette copier):
   ```arm
   0800295C: push {lr}
   0800295E: ldr  r0, [r5, #0x34]      ; <--- CUSTOM PALETTE BASE POINTER!
   08002960: tst  r0, r0
   08002962: bne  _0800296E            ; if [sprite_hdr + 0x34] != 0, use it instead of sprite archive!
   08002964: ldr  r0, [r5, #0x1c]      ; otherwise load from sprite archive...
   ...
   _0800296E:
   0800296E: ldrb r1, [r5, #4]         ; palette index (0 = normal, 1 = hit-flash, 2 = shadow)
   08002970: ldrb r2, [r5, #5]
   08002972: adds r1, r1, r2
   08002974: lsls r1, r1, #5           ; + pal_idx * 0x20
   08002976: adds r0, r0, r1
   ...                                 ; copies 32 bytes from r0 -> 0x03002640[slot] (master OBJ PAL shadow)
   ```
   Every VBlank, `sub_08002590` copies `0x03002640` → `0x03002840` (applying fades/flashes) and `sub_080027FC` DMAs `0x03002840` → `0x05000200` (OBJ PAL RAM).
3. **Why `[sprite_hdr + 0x34]` is the ideal mechanism:**
   - Persists cleanly across every animation frame and hit-flash (supply 3 × 32 B = 96 B: normal Duo palette, hit-flash palette copied from MetalSoul `0x0826D91C`, shadow palette).
   - Automatically resets to `0` when `BN4_SPRITE_LOAD` (`sub_0800287C`) reloads Normal MegaMan or another Soul, so no cleanup hook is needed!
   - Affects **only** MegaMan's sprite when `BN4_NAVI_BYTE(0x24) == 13` (MetalSoul form 9 stays 100% untouched).

---

## 2. Exact Hook Point & Drop-In Code (RS == BM) (V)

| Field | Value (RS & BM) |
|---|---|
| **Hook address** | **`0x080107E8`** (`bl sub_08002DEC` inside `sub_0801079C`, RS == BM) |
| **Original 4 bytes** | `F2 F7 00 FB` (`bl sub_08002DEC`) |
| **Live registers** | `r0` = palette index from `sub_0800BFFE`, `r4` = `[r5+0x58]`, `r5` = MegaMan actor struct. `r1–r3` scratch. |
| **Veneer spot (in `bl` range)** | **RS `0x080C4790` / BM `0x080C479C`** (2nd 8-byte slot in the 20 dead bytes after `DUO_DEFEAT_STATE`; distance `+0xB3FA8`, within ±4 MB Thumb `bl` range!). |

### Drop-in Thumb code
```arm
; At 0x080107E8 (RS == BM):
.org 0x080107E8
    bl      VENEER_FORM_PALETTE     ; RS 0x080C4790 / BM 0x080C479C

; At VENEER_FORM_PALETTE (8 bytes, word-aligned):
.org VENEER_FORM_PALETTE
    ldr     r3, [r15, 0]
    bx      r3
    .word   duo_form_palette_hook | 1

; In free space:
duo_form_palette_hook:
    push    r4, r14
    ; 1. Original BN4_SPRITE_PALETTE_SET (0x08002DEC): sets [sprite_hdr + 4] = r0, returns sprite_hdr in r3
    ldr     r2, =0x08002DEC | 1
    bl      @@call_r2
    mov     r4, r3                  ; r4 = sprite_hdr
    ; 2. Check active form: BN4_NAVI_BYTE(0x24) (0x0800D7CA, RS == BM)
    mov     r1, 0x24
    ldr     r2, =0x0800D7CA | 1
    bl      @@call_r2
    mov     r1, 0                   ; default: no custom palette override
    cmp     r0, 13                  ; form 13 = DuoSoul?
    bne     @@store
    ldr     r1, =duo_soul_pal_set
@@store:
    str     r1, [r4, 0x34]          ; [sprite_hdr + 0x34] = duo_soul_pal_set (or 0)
    pop     r4, r15
@@call_r2:
    bx      r2
    .pool

.align 4
duo_soul_pal_set:
    ; Palette 0 (normal DuoSoul — from duo-soul-design.md §1)
    .halfword 0x0020, 0x0000, 0x0000, 0x1043, 0x149F, 0x0C36, 0x0810, 0x7EB0
    .halfword 0x5CEB, 0x4488, 0x3045, 0x2024, 0x7F73, 0x037F, 0x48C9, 0x3466
    ; Palette 1 (hit-flash — same 32B as MetalSoul @ 0x0826D91C, RS == BM)
    .incbin ROM_IN, 0x0826D91C - 0x08000000, 0x20
    ; Palette 2 (shadow/dark — dimmed DuoSoul)
    .halfword 0x0842, 0x0842, 0x0000, 0x0842, 0x0834, 0x0410, 0x0408, 0x4A30
    .halfword 0x3466, 0x2444, 0x1822, 0x1002, 0x4A90, 0x0258, 0x2865, 0x1C23
```

---

## 3. Proposed Names (for `decomp/symbols/names.txt`)

```
0800295C 0800295C code   GEMINI       SpriteCopyPaletteToShadow        Copies 32B OBJ palette from [sprite_hdr+0x34] (if non-zero) or sprite archive into 0x03002640[slot]
0801079C 0801079C code   GEMINI       BattleMegaManLoadFormSprite      Loads MegaMan's form sprite (BattleGetFormSprite), OBJ palette (BN4_SPRITE_PALETTE_SET), and BN4_FORM_STATE
```
