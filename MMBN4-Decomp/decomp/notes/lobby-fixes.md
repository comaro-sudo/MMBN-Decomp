# Lobby Fixes — Exact Values (Round 8, Task 1, 2026-11-23)

All values below are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`** and visually confirmed from decoded sprite sheets (`list[6]` @ `RS 0x08027CE8 / BM 0x08027CEC`, `list[7]` @ `RS 0x08027E80 / BM 0x08027E84`).

---

## 1. Exact Operator Sprites & Trash Can / Junk Pile Sprite (V, R)

> **Correction on sprite list numbering:** NPC opcode `0x17 <spr>` (`sub_080A8880`) sets `[r5, #0x78] = 0x18` (`6 × 4`), which indexes **`list[6]`** (`RS 0x08027CE8 / BM 0x08027CEC`, 102 entries: humans + Navis). **`list[7]`** (`RS 0x08027E80 / BM 0x08027E84`, 110 entries) is **overworld props/objects**, selected by NPC opcode **`0x25 <spr> 0x1C`** (`sub_080A8980`, which sets `[r5, #0x3c] = spr` and `[r5, #0x78] = 0x1C = 7 × 4`). Every sprite pointer below has **identical pixel/palette data in both RS and BM**.

| Operator / Object | Navi (`navi_id`) | Sprite List & Index | NPC Opcode | Status |
|---|---|---|---|---|
| **Chaud** | ProtoMan (`0x0A`) | `list[6]` **`0x04`** | `.byte 0x17, 0x04` | Verified (already exact) |
| **Mayl** | Roll (`0x04`) | `list[6]` **`0x01`** | `.byte 0x17, 0x01` | Verified (already exact) |
| **Dex** | GutsMan (`0x00`) | `list[6]` **`0x02`** | `.byte 0x17, 0x02` | Verified (already exact) |
| **Higsby** | NumberMan (`0x02`) | `list[6]` **`0x08`** | `.byte 0x17, 0x08` | Verified (already exact) |
| **Mr. Match** | FireMan (`0x01`) | `list[6]` **`0x1D`** | `.byte 0x17, 0x1D` | **Fixed** (was `0x09`) |
| **Shuko** | AquaMan (`0x03`) | `list[6]` **`0x20`** | `.byte 0x17, 0x20` | **Fixed** (was `0x14`) |
| **Lily** | WindMan (`0x05`) | `list[6]` **`0x34`** | `.byte 0x17, 0x34` | **Fixed** (was `0x2E`) |
| **Tamako** | MetalMan (`0x06`) | `list[6]` **`0x1F`** | `.byte 0x17, 0x1F` | **Fixed** (was `0x13`) |
| **Sal** | WoodMan (`0x07`) | `list[6]` **`0x2C`** | `.byte 0x17, 0x2C` | **Fixed** (was `0x17`) |
| **Raika** | SearchMan (`0x09`) | `list[6]` **`0x3B`** | `.byte 0x17, 0x3B` | **Fixed** (was `0x11`) |
| **Raoul** | ThunderMan (`0x08`) | `list[6]` **`0x32`** | `.byte 0x17, 0x32` | **Fixed** (was `0x39`) |
| **JunkMan** | JunkMan (`0x0B`) | `list[6]` **`0x5A`** | `.byte 0x17, 0x5A` | Verified (already exact) |
| **Trash Can / Junk Pile** | *(jack-in to `(8E, 00)`)* | **`list[7]` `0x44`** (Junk/Trash Pile; alt: `list[7]` `0x5A` grey bin/pedestal) | **`.byte 0x25, 0x44, 0x1C`** | **Fixed** (was `0x17, 0x2B` old lady) |

---

## 2. The "Met That Navi" Gate — Why `0x0001`–`0x000C` Were Wrong & Exact Replacement (V, R)

### What `0x0001`–`0x000C` actually do
In `TournamentFillEntrants` (`sub_08044058`, `_080441D2`):
```arm
ldrb r0, [r5, #2]       ; slot +2 = first "other entrant" (NEVER slots +0/+1, the tournament's own Soul pair!)
movs r1, #1
adds r1, r1, r0
adds r0, r1, #0
bl   sub_0802504C       ; SetEventFlag(id + 1)
```
Confirmed across all 17 community saves:
- `4752.sav` (Red Sun, PT 3, all 6 RS Souls): flags `0x01..0x0C` are **all 0**; only `0x0D..0x12` (the 6 shared generic opponents) are set.
- `4569.sav` (Blue Moon, PT 2, all 6 BM Souls): flags `0x01..0x0C` are **all 0**; only `0x0D..0x12` are set.
- `22565.sav` (Red Sun, PT 4, traded BM Navis): only the 6 **traded** BM Navis (`0x03, 0x04, 0x07, 0x08, 0x0B, 0x0C`) are set; the 6 native RS Navis are still **0**.

So flags `0x0001`–`0x000C` are **only** set when a *traded* other-version Navi is drawn into slot `+2` — they are **never** set for your own version's 6 story Soul Navis. Gating the lobby on `0x0001..0x000C` hides all 6 native operators in every non-traded save.

### Exact replacement flags: the 12 Double Soul flags (`0x0017`–`0x0022`)
In Violet Eclipse, every Soul Navi is met in either their canon story tournament match (AquaMan, Roll, ProtoMan) or their friendly match (the other 9), and **every one of those 12 fights grants that Navi's Double Soul flag (`0x0017`–`0x0022`)** (`docs/re/souls.md`). Using `0x0017`–`0x0022`:
- unlocks each lobby operator the moment you finish their story or friendly match;
- works immediately on all existing vanilla saves (`4752.sav`, `4569.sav`, `22565.sav`, etc.);
- requires zero extra flags.

Formula: `soul_flag = 0x0016 + dat_08044B4C[navi_id]` (identical in RS and BM):

| Operator | Navi (`navi_id`) | Old (wrong) flag | **Correct flag (Soul flag)** |
|---|---|---:|---:|
| Dex | GutsMan (`0x00`) | `0x0001` | **`0x0018`** (GutsSoul) |
| Mr. Match | FireMan (`0x01`) | `0x0002` | **`0x001B`** (FireSoul) |
| Higsby | NumberMan (`0x02`) | `0x0003` | **`0x001E`** (NumberSoul) |
| Shuko | AquaMan (`0x03`) | `0x0004` | **`0x0021`** (AquaSoul) |
| Mayl | Roll (`0x04`) | `0x0005` | **`0x0017`** (RollSoul) |
| Lily | WindMan (`0x05`) | `0x0006` | **`0x0019`** (WindSoul) |
| Tamako | MetalMan (`0x06`) | `0x0007` | **`0x001F`** (MetalSoul) |
| Sal | WoodMan (`0x07`) | `0x0008` | **`0x0022`** (WoodSoul) |
| Raoul | ThunderMan (`0x08`) | `0x0009` | **`0x001C`** (ThunderSoul) |
| Raika | SearchMan (`0x09`) | `0x000A` | **`0x001A`** (SearchSoul) |
| Chaud | ProtoMan (`0x0A`) | `0x000B` | **`0x001D`** (ProtoSoul) |
| JunkMan (+ Trash Can) | JunkMan (`0x0B`) | `0x000C` | **`0x0020`** (JunkSoul) |

### Drop-in table edit for `wp_d3_lobby_rematches.asm`
```arm
; 9 Higsby's operators (list[6] sprites, gated on Soul flags 0x0017..0x0022)
lobby_npc lobby_npc_chaud,  0x04, 0x001D, 0xFA,   36,  42, 5  ; ProtoMan  (0x0A)
lobby_npc lobby_npc_mayl,   0x01, 0x0017, 0xF4,   36,  70, 5  ; Roll      (0x04)
lobby_npc lobby_npc_dex,    0x02, 0x0018, 0xF0,   36,  98, 5  ; GutsMan   (0x00)
lobby_npc lobby_npc_match,  0x1D, 0x001B, 0xF1,   12, 112, 7  ; FireMan   (0x01)
lobby_npc lobby_npc_shuko,  0x20, 0x0021, 0xF3,  -54, 112, 7  ; AquaMan   (0x03)
lobby_npc lobby_npc_lily,   0x34, 0x0019, 0xF5,  -84, 112, 7  ; WindMan   (0x05)
lobby_npc lobby_npc_tamako, 0x1F, 0x001F, 0xF6, -140,  44, 3  ; MetalMan  (0x06)
lobby_npc lobby_npc_sal,    0x2C, 0x0022, 0xF7, -134,  72, 1  ; WoodMan   (0x07)
lobby_npc lobby_npc_number, 0x08, 0x001E, 0xF2,  -56,  56, 3  ; NumberMan (0x02)

; 2 ACDC Town street operators
lobby_npc lobby_npc_raika,  0x3B, 0x001A, 0xF9, -134, -76, 5  ; SearchMan  (0x09)
lobby_npc lobby_npc_raoul,  0x32, 0x001C, 0xF8, -164, -56, 7  ; ThunderMan (0x08)

; Trash Can / Junk Pile on ACDC Town (uses op 0x25 -> list[7] 0x44 Junk Pile, gated on JunkSoul 0x0020)
.align 4
lobby_npc_trashcan:
    .byte   0x08
    .byte   0x25, 0x44, 0x1C        ; list[7] 0x44 = Junk/Trash Pile (alt: 0x5A = grey bin)
    .byte   0x05, 0x20, 0x00        ; hide if JunkSoul (0x0020) is clear
    .word   @@hide
    .byte   0x18, 0xFC
    .byte   0x14
    .halfword (-150) & 0xFFFF, (-90) & 0xFFFF, 0
    .byte   0x16, 3
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00

; JunkMan inside WtrGodComp1 (8E,00) (list[6] sprite 0x5A, gated on JunkSoul 0x0020)
lobby_npc lobby_npc_junkman, 0x5A, 0x0020, 0xFB, 0, -24, 3    ; JunkMan (0x0B)
```

---

## 3. JunkMan's Spot: `WtrGodComp1 (8E, 00)` Clash Audit (V)

Checked `WtrGodComp1 (8E, 00)` in `dat_08029CC4` (CW map-group scripts) and `dat_08029E14` (CW NPC lists) in both RS and BM:
- **Init script (`RS 0x08075FA8`):** `1C FF 00 11  1C FF 20 11  00` — only clears two temporary flags (`0x1100`, `0x1120`) and returns. **Zero AquaMan cutscene triggers.**
- **Continuous script (`RS 0x08075FB1`):** `00` (**empty**). (AquaMan's scenario triggers live in `WtrGodComp2 (8E, 01)` at `0x08075FBD`, not `(8E, 00)`.)
- **Existing NPC[0] (`RS 0x08076520 / BM 0x0807652C`):** gated on AquaMan scenario flag `0x0B2E`, placed at `(36, -4, 0)` facing `5`, `talk_idx = 0x82`.
- **Our JunkMan NPC:** gated on JunkSoul `0x0020`, placed at `(0, -24, 0)` facing `3`, `talk_idx = 0xFB`; warp lands MegaMan at `(0, 0, 0)` facing `7`.
- **Verdict:** **Zero clash** (scripts, flags, coordinates, and talk indices are completely disjoint). `WtrGodComp1 (8E, 00)` is safe to keep as-is.
