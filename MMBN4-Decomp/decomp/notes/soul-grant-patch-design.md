# Soul Grant Routine, `checkDoubleSouls` (12 Souls), & Full Soul-Reader Audit (2026-11-23)

Patch design for order step 3:
1. A reusable "grant any Soul `idx`" routine & inline script sequence;
2. `ScriptCmd_checkDoubleSouls` (`RS 0x0804F494` / `BM 0x0804F4A0`) — what it guards and how to update it for 12 Souls;
3. Complete ROM-wide audit of every reader of Soul flags `0x17`–`0x22` and counts `6`/`12`, plus any other menus/screens that list Souls.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. Granting Any of the 12 Souls (`idx = 0..11`) (V, R)

### 1.1 Existing script commands (zero new code needed inside scripts!)
Both script engines **already** have opcodes for `SetEventFlag` and `WriteChipKey` (`soul-grant.md` §2):
- **Cutscene / event scripts (`BN4_CUTSCENE_SCRIPT_CMDS`, `0x0802C140`):** **11 bytes**, no new code needed:
  ```text
  26 FF 15 00                 ; SetEventFlag(0x0015) — Double Soul unlocked
  26 FF <0x17 + idx> 00       ; SetEventFlag(0x0017 + idx)
  66 <0x61 + idx> 01          ; WriteChipKey(0x0161 + idx) — auto-uses SAVE_XOR_KEY_1 (0x64 RS / 0x47 BM)
  ```
- **Map scripts (`map_scripts_commands`, `0x0802A7D0`):** same **11 bytes** using opcodes `1C` and `3F`:
  ```text
  1C FF 15 00                 ; SetEventFlag(0x0015)
  1C FF <0x17 + idx> 00       ; SetEventFlag(0x0017 + idx)
  3F <0x61 + idx> 01          ; WriteChipKey(0x0161 + idx)
  ```
*(So friendly-match cutscene scripts in WP-B1 do **not** even need a new script opcode to grant any of the 12 Souls!)*

### 1.2 Callable Thumb routine `GrantSoulByIndex` (for ASM patches)
For C/ASM hooks (e.g. post-battle hooks or debug/catch-up patches), a **28-byte** Thumb helper in free space (`0x087F7420+`, or a dead block in `code_02`/`code_03` if called via `bl`):

```arm
; r0 = idx (0..11: 0=Roll, 1=Guts, 2=Wind, 3=Search, 4=Fire, 5=Thunder,
;                  6=Proto, 7=Number, 8=Metal, 9=Junk, 10=Aqua, 11=Wood)
GrantSoulByIndex:
    push {r4, lr}
    movs r4, r0
    movs r0, #0x15
    bl   SetEventFlag          ; RS 0x0802504C / BM 0x08025050
    adds r0, r4, #0x17
    bl   SetEventFlag          ; RS 0x0802504C / BM 0x08025050
    ldr  r0, =0x0161
    adds r0, r0, r4
    bl   WriteChipKey          ; RS & BM 0x080063B4 (auto-uses 0x64 RS / 0x47 BM)
    pop  {r4, pc}
    .pool                      ; 4 bytes (0x00000161) -> 28 bytes total
```

---

## 2. `ScriptCmd_checkDoubleSouls` (`RS 0x0804F494` / `BM 0x0804F4A0`) (V, R)

- **Text-script opcode:** **`EE 2B <script_if_true> <script_if_false>`** (entry `43 = 0x2B` of `ScriptGroup_EE` `dat_0804F020`).
- **What it guards (verified in ROM text scan):** **Exactly 1 script in the entire game calls `EE 2B`**:
  - **RS `0x08777AE8`** (inside archive `0x08777ACC`, referenced at `0x08052E40` — **Black Earth 1 / Undernet "6 Souls" gate**):
    - Text: *"Show the union of 6 souls."* (checks `EE 2B FF 05`: if all Souls are owned, falls through to open the gate; otherwise jumps to script `05`).
- **Disassembly (`code_04.s`):**
  ```arm
  sub_0804F494:                ; RS 0x0804F494 / BM 0x0804F4A0 (NOTE: BM delta is +0x0C!)
      push {lr}
      movs r0, #FIRST_SOUL_FLAG ; RS 0x0804F496: 17 20 (0x17) | BM 0x0804F4A2: 1D 20 (0x1D)
      movs r1, #6               ; RS 0x0804F498: 06 21 (6)    | BM 0x0804F4A4: 06 21 (6)
      adds r0, r0, #0
      adds r2, r1, #0
      bl   TestEventFlagRangeAll ; RS 0x08025138 / BM 0x0802513C
  ```
- **Patch for 12 Souls (2 bytes RS, 4 bytes BM):**
  - **Red Sun:** patch `0x0804F498` from `06 21` → **`0C 21`** (`movs r1, #12`; `0x0804F496` is already `17 20`).
  - **Blue Moon:** patch `0x0804F4A2` from `1D 20 06 21` → **`17 20 0C 21`** (`movs r0, #0x17; movs r1, #12`).
  - *(Note: If the owner prefers the Black Earth gate to open at 12 Souls, apply the patch above and optionally update the text `"6 souls"` → `"12 souls"` in archive `0x08777ACC`. If keeping it at 6 Souls, change BM's `0x1D` → `0x17` or check any 6 of 12, since a unified Violet Eclipse run gets Aqua/Roll/Proto + friendlies from both halves!)*

---

## 3. Complete ROM-Wide Audit of Soul Flag Readers & Menus (V, R)

Every routine in `code_00`–`code_10` that reads Soul flags `0x17`–`0x22`, calls `TestEventFlagRangeAll` (`0x08025138`), or checks Soul counts `6`/`12`:

| Routine / Table | RS Address | BM Address | Vanilla count | Status for 12 Souls |
|---|---|---|---|---|
| `GetCompletionStars` (`BN4_STAR_SOUL_FLAGS` + `cmp r5, #6`) | `0x08000D56` / `0x08000DB4` | same | 6 | **Already fixed** (`wp_d4_star_twelve_souls.asm`, `79398a4`, verified PASS in Round 5) |
| `BN4_UNITE_CHECK` (`BN4_SOUL_FLAGS`) | `0x0801FF84` / `0x08020018` | same | Loop is **already 12** (`r3 = 0x0B..0`); table had 6 `0xFF`s | **Already fixed** (`wp_d4_twelve_souls.asm`) |
| `StatusCollectOwnedSouls` (`BN4_STATUS_SOUL_FLAGS`) | `0x08034A30` / `0x08034A78` | `+0x04` | 6 | **Claude's step 2** (design in `status-screen-two-pages.md`) |
| `StatusSoulIconInit` (`dat_0810DAB8` / `DAC5` / `DACB`) | `0x0810D9F8` / `0x0810DAB8` | `+0x14` | 6 | **Claude's step 2** (design in `status-screen-two-pages.md`) |
| `ScriptCmd_checkDoubleSouls` (`EE 2B`) | `0x0804F494` | `0x0804F4A0` (`+0x0C`) | **6** (`06 21`) | **Needs patch above** (§2: `0x17`, count `12`) |
| `TestEventFlagRangeAll` other callers (`0x0802A968`, `0x0802A9C4`, `ScriptCmd_checkMultiFlag` `0x0804F0F8`) | `0x08025138` | `0x0802513C` | Data-driven | Safe (no hardcoded 6) |

### Other screens / menus that list or display Souls
1. **Battle Custom Screen (Soul Unison prompt):** Does **not** list all owned Souls at once — `BN4_UNITE_CHECK` (`0x0801FF84`) checks only the single Soul matching the currently highlighted chip's element/type (`BN4_SOUL_KINDS`, 12 entries) and displays that one Soul's `UNITE` prompt. Already supports all 12 Souls once `BN4_SOUL_FLAGS` is expanded.
2. **Title Screen (`SOUL COMPL` badge):** Already fixed and verified for 12 Souls (`wp_d4_star_twelve_souls.asm`).
3. **MegaMan Status Screen:** The **only** menu in BN4 that lists owned Souls as a grid (handled by Claude's step 2 via `status-screen-two-pages.md`). Neither the NaviCust screen, Library, nor Records screen lists Double Souls.

---

## 4. Proposed symbol names (`names.txt` format)

```text
0804F494 0804F4A0 code   GEMINI       ScriptCmd_checkDoubleSouls       Text opcode EE 2B (Black Earth gate, archive 0x08777ACC): TestEventFlagRangeAll(FIRST_SOUL_FLAG, 6); NOTE BM delta is +0x0C
```
