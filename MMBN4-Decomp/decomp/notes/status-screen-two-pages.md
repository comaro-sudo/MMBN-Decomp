# Status screen two-page Soul view — patch design (2026-11-23)

Implementation-ready patch design for displaying up to all 12 Double Souls on the MegaMan Status screen using **two pages of 6** in the existing 2 × 3 layout (since `status-screen-2x6.md` proved 2 × 6 collides with the `NAVIGATOR` box at `Y = 94`).

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = verified against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. Input handling & free buttons on the Status screen (V, R)

All input reads in the Status screen (`0x08033290`–`0x08035200`, `code_02`):

| Routine (RS / BM) | Input field (`[sl+4]`) | Buttons used | Action |
|---|---|---|---|
| `StatusScreenMain` (`0x08033590` / `0x08033594`) | `+2` (newly pressed) | `0x000A` (**B**, **START**) | Close Status screen |
| `StatusScreenInput` (`0x08033EE0` / `0x08033EE4`) | `+4` (repeat) | `0x0030` (**LEFT**, **RIGHT**) | Switch right-hand panel (`MEGA BUSTER` ↔ `PROGRAMS`/`RECORDS`) |
| `StatusScreenInput` → `sub_080348F0` | `+4` (repeat) | `0x00C0` (**UP**, **DOWN**) | Move `NAVIGATOR` menu cursor |
| `sub_08034170` / `sub_08053918` | `+2` (newly pressed) | `0x0001` (**A**) | Confirm NaviCust / Records |

- **Free buttons:** **`L` (`0x0200`)**, **`R` (`0x0100`)**, and **`SELECT` (`0x0004`)** are **completely unused** anywhere on the Status screen.
- **Recommended page-toggle button:** **`L` or `R` (`0x0300`)** on newly-pressed input (`ldrh r0, [[sl + 4] + 2]`; optionally `0x0304` to also allow `SELECT`).
- **Cleanest hook point (zero displaced instructions!):**
  - `StatusScreenState0` (`RS 0x08033634` / `BM 0x08033638`) calls **`sub_08034EB0`** at **RS `0x0803367A` / BM `0x0803367E`** every frame when the Status screen is idle (`[r5, #2] == 0`) and not fading.
  - `sub_08034EB0` (`RS 0x08034EB0` / `BM 0x08034EB4`) is an **empty no-op stub** (`push {r4-r7, lr}; pop {r4-r7, pc}`). Replacing that single `bl` (or patching `sub_08034EB0` to jump to free space) requires **zero displaced instructions**.

---

## 2. Re-running the 6-icon fill for Page 1 vs Page 2 (V, R)

### 2.1 Existing pipeline & zero-cost page state

1. `StatusCollectOwnedSouls` (`RS 0x08034A30` / `BM 0x08034A34`):
   - Calls `sub_080351A8`, which fills **`0x0201D000..0x0201D01F`** (32 bytes) with `0xFF`.
   - Walks `BN4_STATUS_SOUL_FLAGS` (`0xFF`-terminated), writing owned Soul event flags into `0x0201D000[0..count-1]` (at most 12 bytes: `0x0201D000..0x0201D00B`, leaving `0x0201D00C = 0xFF`).
   - Calls `sub_08053F8C(dat_0803325C)` to spawn the 6 UI objects of type **`0x45`** (`slot = 0..5`).
2. **Page offset byte:** Store the active page offset (`0` for Page 1, `6` for Page 2) at **`0x0201D018`** — inside the same 32-byte scratch buffer!
   - Initialize `[0x0201D018] = 0` inside `StatusCollectOwnedSouls` (before spawning `dat_0803325C`) so opening the Status screen always starts on Page 1.
   - Page 2 is enabled iff **`[0x0201D006] != 0xFF`** (player owns ≥ 7 Souls). When ≤ 6 Souls are owned, `L`/`R` does nothing.

### 2.2 Expanding the 4 Soul tables to all 12 Souls

Relocate these 4 tables to free space (`0x087F7420+`) and repoint their 4 pool literals (sprite `0x37` already contains all 12 Soul animations `0..11` in both RS and BM):

| Table | Pool literal (RS) | Pool literal (BM) | New 12-Soul bytes |
|---|---|---|---|
| `BN4_STATUS_SOUL_FLAGS` | `0x08034A6C` | `0x08034A70` | `17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 FF` (13 B) |
| `BN4_STATUS_ICON_SOUL_FLAGS` | `0x0810DA78` | `0x0810DA8C` | `17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 FF` (13 B) |
| Sprite `0x37` anims (`dat_0810DAC5`) | `0x0810DA88` | `0x0810DA9C` | `00 01 02 03 04 05 06 07 08 09 0A 0B` (12 B) |
| Sprite palettes (`dat_0810DACB`) | `0x0810DA8C` | `0x0810DAA0` | `00 01 01 01 00 00 00 01 02 00 02 01` (12 B) |

*(Note: `BN4_STATUS_SOUL_FLAGS` and `BN4_STATUS_ICON_SOUL_FLAGS` are identical 13-byte arrays and can share a single 13-byte table in free space.)*

### 2.3 Reading `slot + page_offset` in `StatusSoulIconInit`

In `StatusSoulIconInit` (`RS 0x0810D9F8` / `BM 0x0810DA0C`):
```arm
; Original (RS 0x0810D9FA..0x0810DA01 / BM 0x0810DA0E..0x0810DA15, 8 bytes):
    movs r2, #1
    ldrb r6, [r5, #4]       ; slot 0..5
    ldr  r1, =0x0201D000
    ldrb r4, [r1, r6]       ; Soul flag = buf[slot]
```
Replace with a `bl` (4 bytes, plus `nop`s) to a small helper in free space (or inline via `bl` + `bx` trampoline if `code_10` is > 4 MiB from free space — see §4):
```arm
StatusSoulLoadFlag:
    ldrb r6, [r5, #4]       ; keep r6 = 0..5 (VRAM tile, OAM slot, (X,Y) all stay on slots 0..5!)
    ldr  r1, =0x0201D000
    ldrb r2, [r1, #0x18]    ; page_offset = 0 or 6
    adds r2, r2, r6         ; index = slot + page_offset (0..11)
    ldrb r4, [r1, r2]       ; Soul flag (or 0xFF if empty on Page 2)
    movs r2, #1
    bx   lr
```
Because `r6` stays `0..5`, all 6 icons reuse the exact existing VRAM tiles (`0x50..0x64`), OAM slots (`0x00..0x50`), and `(X, Y)` coordinates (`(28,42)..(84,78)`). Empty slots on Page 2 read `0xFF` and automatically hide themselves (`[r5] = 1`).

### 2.4 Triggering the refresh on `L` / `R`

UI object `0x45` (`StatusSoulIconTask`, `RS 0x0810D9D4` / `BM 0x0810D9E8`) dispatches on `[obj + 8]`:
- `0` → `StatusSoulIconInit` (reloads sprite `0x37` frame, palette, visibility, and calls `StatusSoulIconUpdate`)
- `4` → `StatusSoulIconUpdate`

All UI objects live in the 24-slot pool at **`0x02008BB0`** (stride **`0x58`** bytes, runner `sub_08053FA8`). So inside the `sub_08034EB0` page-toggle hook:
```arm
StatusSoulPageToggle:
    push {r4-r7, lr}
    ldr  r4, =0x0201D000
    ldrb r0, [r4, #6]           ; 7th owned Soul present?
    cmp  r0, #0xFF
    beq  @@done                 ; <= 6 Souls: no Page 2
    mov  r0, sl
    ldr  r0, [r0, #4]           ; input struct
    ldrh r0, [r0, #2]           ; newly pressed buttons
    ldr  r1, =0x0300            ; L | R (or 0x0304 to include SELECT)
    tst  r0, r1
    beq  @@done
    ldrb r0, [r4, #0x18]        ; toggle 0 <-> 6
    movs r1, #6
    eors r0, r1
    strb r0, [r4, #0x18]
    movs r0, #0x6E              ; page-flip cursor SFX
    bl   PlaySoundEffect        ; 0x08000560 (RS & BM)
    ldr  r5, =0x02008BB0        ; UI object pool (24 slots x 0x58)
    movs r6, #24
@@loop:
    ldrb r0, [r5, #0]           ; active?
    lsrs r0, r0, #1
    bcc  @@next
    ldrb r0, [r5, #1]           ; type == 0x45 (StatusSoulIconTask)?
    cmp  r0, #0x45
    bne  @@next
    movs r0, #0
    strb r0, [r5, #8]           ; reset state -> re-runs StatusSoulIconInit this frame!
@@next:
    adds r5, #0x58
    subs r6, #1
    bne  @@loop
    ; (optional: update page marker here, see §3)
@@done:
    pop  {r4-r7, pc}
```
Because `sub_08053FA8` runs right after `StatusScreenState0` inside `StatusScreenMain`, all 6 Soul icons update **on the exact same frame** with zero flicker.

---

## 3. How to mark which page is shown

Two clean options (both shown only when `[0x0201D006] != 0xFF`, i.e., ≥ 7 Souls owned):

1. **Tilemap `1/2` ↔ `2/2` badge (recommended — zero extra OAM or sprite graphics):**
   - On `status_4752.bmp`, the top-right edge of the MegaMan box (`X = 96..118, Y = 22..32`, tiles `(12, 3)..(14, 3)`) is empty header space just right of the HP box (`X = 18..102`).
   - Write 3 halfwords directly to the Status screen's text BG tilemap (`0x0201C000` / BG0 screen buffer, flushed every frame by `sub_080334C0`) using the already-loaded digit/slash tiles: `'1' '/' '2'` when `[0x0201D018] == 0`, `'2' '/' '2'` when `[0x0201D018] == 6`.
2. **Directional arrow sprite (zero tilemap math):**
   - Reuse the Status screen's existing green arrow UI object (`0x13` / `0x14` from `dat_0803324C`, which only uses 12 / 24 UI object slots) positioned at `(108, 60)` — pointing right on Page 1 and left on Page 2.

---

## 4. Hook table, RS/BM addresses & code-size estimate (V, R)

> [!IMPORTANT]
> **Thumb `bl` range note:** Free space (`0x087F7420+`) is **`> 4 MiB`** away from `code_02` (`0x08033xxx`) and `code_10` (`0x0810Dxxx`), so a direct Thumb `bl` cannot reach `0x087F7420+`. Either:
> - Place the ~110 bytes of helper code in a **dead/unused block in `code_02`/`code_03`** (within ±4 MiB `bl` range, like `wp_d2`/`wp_d3`), **or**
> - Use a 10-byte Thumb long-call stub (`ldr r3, =target; bx r3` / `blx` via `mov lr, pc; bx r3`) at each hook site.

| Hook / Patch Site | RS Address | BM Address | Delta | Change |
|---|---|---|---|---|
| `StatusCollectOwnedSouls` (`_08034A6C` literal) | `0x08034A6C` | `0x08034A70` | `+0x04` | Repoint to 13-byte 12-Soul flag list in free space |
| `StatusCollectOwnedSouls` (zero `0x0201D018`) | `0x08034A64` | `0x08034A68` | `+0x04` | Hook `bl sub_08053F8C` to clear `[0x0201D018] = 0` before spawning |
| `StatusScreenState0` idle stub (`sub_08034EB0`) | `0x08034EB0` | `0x08034EB4` | `+0x04` | Replace empty 4-byte stub with `StatusSoulPageToggle` (`L`/`R` toggle + reset `0x45` objects) |
| `StatusSoulIconInit` (slot read) | `0x0810D9FA` | `0x0810DA0E` | `+0x14` | Read `0x0201D000[slot + page_offset]` via `StatusSoulLoadFlag`, preserving `r6 = 0..5` |
| `StatusSoulIconInit` (`_0810DA78` flags literal) | `0x0810DA78` | `0x0810DA8C` | `+0x14` | Repoint to shared 13-byte 12-Soul flag list |
| `StatusSoulIconInit` (`_0810DA88` anims literal) | `0x0810DA88` | `0x0810DA9C` | `+0x14` | Repoint to 12-byte anim list `00..0B` |
| `StatusSoulIconInit` (`_0810DA8C` palettes literal) | `0x0810DA8C` | `0x0810DAA0` | `+0x14` | Repoint to 12-byte palette list `00 01 01 01 00 00 00 01 02 00 02 01` |
| `PlaySoundEffect` (called by toggle) | `0x08000560` | `0x08000560` | `0` | Existing SFX helper (`r0 = 0x6E`) |

### Free-space & code size estimate

| Component | Size | Location |
|---|---|---|
| Shared 12-Soul flag list (`17..22 FF`), anims (`00..0B`), palettes (12 B) | **40 B** (aligned) | Free space (`0x087F7420+`) |
| `StatusSoulInitPage` (clears `0x0201D018`, chains `sub_08053F8C`) | **20 B** | Free space (or `code_02` dead space) |
| `StatusSoulLoadFlag` (`buf[slot + page_offset]`) | **20 B** | Free space (or `code_10` dead space) |
| `StatusSoulPageToggle` (`L`/`R` check, toggle, SFX, reset 6 × `0x45`, page badge) | **~96 B** | Free space (or `code_02` dead space) |
| **Total** | **~176 B** | Zero new VRAM, zero new OAM slots, zero new EWRAM variables outside `0x0201D000..0x0201D01F` |
