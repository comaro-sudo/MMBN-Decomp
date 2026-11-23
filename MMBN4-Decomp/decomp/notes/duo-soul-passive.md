# Duo Soul Passive: Verified `BN4_FORM_ENTERS` Options (Round 11, Task 4, 2026-11-23)

Disassembly-verified audit of **every** entry in `BN4_FORM_ENTERS` (`0x0800E0A0`, 16 × u32, **RS == BM identical**), how `BN4_FORM_STATE` (`0x0800E05C`) / `sub_0800D8C2` cleans up every passive when a Soul ends, and 3 verified Duo Soul passive options that work cleanly with MetalSoul's sprite.

---

## 1. Complete Verified Table of `BN4_FORM_ENTERS` (`0x0800E0A0`, RS == BM) (V)

| Form | Soul | Entry Address | Disassembly-Verified Behaviour & Helper | How Removed When Soul Ends |
|---:|---|---|---|---|
| `0` | Normal | `0x0800E0E1` | `mov pc, lr` (no-op) | — |
| `1` | Roll | `0x0800E0E3` | `mov pc, lr` (no-op; RollSoul's heal & chip-destroy run inline on chip use) | — |
| `2` | **Guts** | `0x0800E0E5` | Calls **`0x0800E1CA(side)`**: loops through current hand chips; if `chip[+9] & 2` (damage chip) and `chip[+7] ∈ {0x08, 0x0C}` (Guts/Null), adds **`+30` (`0x1E`)** to `[hand_slot + 0x24]` (per-chip bonus ATK). *(Not SuperArmor!)* | `BN4_FORM_STATE` (`0x0800E06C`) calls `0x08000918(hand + 0x26, 0x0C)` to zero all per-chip ATK bonuses. |
| `3` | **Wind** | `0x0800E0F5` | (1) Calls **`0x08012F82(0x30)`**: ORs **`0x30`** into `[r5+0x54]->[0x64]` (**AirShoes `0x10` + FloatShoes `0x20`**). (2) Calls `0x08012ED0(r1=0x10, r2=2, r3=0)` + `0x080E68F8` (spawns Wind fan companion stored in `[r5+0x58]->[0x5C]`). (3) Calls **`0x0800E208(side)`**: adds **`+10`** to hand chips of class `0x0A` (Wind). | `sub_0800D8C2` (`0x0800D8E0`) calls `0x08012F8C` (clears status bits, then re-applies NaviCust defaults via `0x0800B026`), despawns `[r7, #0x5C]` via `0x080E69BC`, and `BN4_FORM_STATE` zeroes hand ATK bonuses. |
| `4` | **Search** | `0x0800E13F` | Calls **`0x080E6E4C()`**: spawns battle effect `0x44` (`0x08003344(0x44)`, invisible-reveal / Search target cursor). | Despawns when `NaviByte(0x24)` leaves form 4. |
| `5` | **Fire** | `0x0800E11D` | Calls **`0x080E3604()`**: spawns battle effect `0x12` (`0x08003344(0x12)`, FireSoul charge-boost aura). | Despawns when `NaviByte(0x24)` leaves form 5. |
| `6` | Thunder | `0x0800E0E1` | `mov pc, lr` (no-op; ThunderSoul's stun-on-Elec runs inline on hit) | — |
| `7` | **Proto** | `0x0800E12D` | Writes **`0x6A`** into **`[r5+0x58]->[0]`** (**B+Left Reflect Shield** command). | `sub_0800D8C2` (`0x0800D8D6`) restores `[r7]` from `[r6, #0x0C]` (MegaMan's NaviCust B+Left default). |
| `8` | **Number** | `0x0800E135` | Calls **`0x0800E194(side)`**: loops through current hand chips; if `chip[+9] & 2` and `chip[+7] == 0x0C` (Null), adds **`+10`** to `[hand_slot + 0x24]`. | `BN4_FORM_STATE` (`0x0800E06C`) zeroes hand ATK bonuses. |
| `9` | Metal | `0x0800E0E1` | `mov pc, lr` (no-op; MetalSoul's Break×2 charge runs inline) | — |
| `10` | **Junk** | `0x0800E147` | Calls **`0x080CD81E()`**: spawns JunkSoul's poltergeist/recycle field object. | Despawns when `NaviByte(0x24)` leaves form 10. |
| `11` | Aqua | `0x0800E0E1` | `mov pc, lr` (no-op; AquaSoul's Aqua×2 runs inline) | — |
| `12` | **Wood** | `0x0800E175` | Calls **`0x0800E294(side)`**: scans hand for Wood chips (`chip[+7] == 3`) followed by Null chips (`0x0C`) to power up Gaia-style absorption. | `BN4_FORM_STATE` zeroes hand ATK bonuses. |
| `13` | **Duo** | `0x0800E0E1` | `mov pc, lr` (**slot at `0x0800E0D4`, free for DuoSoul!**) | Automatic via `sub_0800D8C2` + `BN4_FORM_STATE`! |
| `14` | Forte | `0x0800E0E1` | `mov pc, lr` | — |
| `15` | **Dark** | `0x0800E17F` | Calls `0x0800F4DE(side, 0)`, **`0x08012F82(mask)`** (DarkSoul status flags), `0x0800EAC6()`. | Cleared by `sub_0800D8C2`. |

---

## 2. Three Verified Duo Soul Passive Options (All RS == BM, Safe with MetalSoul Sprite)

### Option A (Recommended): **AirShoes + FloatShoes (`0x30`) + B+Left Reflect Shield (`0x6A`)**
- **Theme:** Duo hovers above space/broken panels (Meteor/Comet float) and blocks attacks with his justice barrier (Reflect Shield).
- **Sprite safety:** Uses zero custom animations (Reflect Shield spawns the standalone shield effect `kind 3` over any MegaMan sprite; AirShoes/FloatShoes are pure status bits in `[r5+0x54]->[0x64]`).
- **Exact code** (repoint `0x0800E0D4` → `duo_soul_enter | 1`):
  ```arm
  duo_soul_enter:
      push    r14
      mov     r0, 0x30                ; AirShoes (0x10) | FloatShoes (0x20)
      ldr     r3, =0x08012F82 | 1     ; BattleSetStatusBits (RS == BM)
      bl      @@call_r3
      ldr     r0, [r5, 0x58]
      mov     r1, 0x6A                ; B+Left Reflect Shield (ProtoSoul's exact byte)
      strb    r1, [r0]
      pop     r15
  @@call_r3:
      bx      r3
      .pool
  ```
- **Removal on Soul end:** `sub_0800D8C2` automatically calls `0x08012F8C` (clears `0x30`, restores NaviCust defaults via `0x0800B026`) AND restores `[r7]` (B+Left) from `[r6, #0x0C]`. **Zero cleanup code needed.**

### Option B: **B+Left Reflect Shield (`0x6A`) + Hand Null/Break Chips `+30` ATK (`0x0800E1CA`)**
- **Theme:** Duo's overwhelming offensive + defensive might (heavy brawler + barrier).
- **Exact code:**
  ```arm
  duo_soul_enter:
      push    r14
      ldrb    r0, [r5, 0x16]          ; side
      ldr     r3, =0x0800E1CA | 1     ; BattleBoostHandNullBreak30 (GutsSoul helper, RS == BM)
      bl      @@call_r3
      ldr     r0, [r5, 0x58]
      mov     r1, 0x6A                ; B+Left Reflect Shield
      strb    r1, [r0]
      pop     r15
  @@call_r3:
      bx      r3
      .pool
  ```
- **Removal on Soul end:** `sub_0800D8C2` restores `[r7]` and `BN4_FORM_STATE` zeroes `[hand + 0x26]` (`0x0C` bytes).

### Option C: **AirShoes + FloatShoes (`0x30`) + Hand Null Chips `+10` ATK (`0x0800E194`)**
- **Theme:** Lighter, balanced cosmic-flight + Null-chip boost (no Reflect Shield overlap with ProtoSoul).
- **Exact code:** Same as Option B, calling `0x08012F82(0x30)` + `0x0800E194(side)`.
- **Removal on Soul end:** Automatic via `sub_0800D8C2` + `BN4_FORM_STATE`.

---

## 3. Proposed Names (for `decomp/symbols/names.txt`)

```
0800D8C2 0800D8C2 code   GEMINI       BattleResetFormPassives          Resets B+Left command, status bits (Air/FloatShoes), and form companions on Soul change/end
0800E194 0800E194 code   GEMINI       BattleBoostHandNull10            r0 side: adds +10 ATK to damage chips of class 0x0C in hand (NumberSoul enter)
0800E1CA 0800E1CA code   GEMINI       BattleBoostHandNullBreak30       r0 side: adds +30 ATK to damage chips of class 0x08/0x0C in hand (GutsSoul enter)
0800E208 0800E208 code   GEMINI       BattleBoostHandWind10            r0 side: adds +10 ATK to damage chips of class 0x0A in hand (WindSoul enter)
08012F82 08012F82 code   GEMINI       BattleSetStatusBits              r0 mask: ORs bits into [r5+0x54]->[0x64] (0x10 AirShoes, 0x20 FloatShoes)
08012F8C 08012F8C code   GEMINI       BattleClearStatusBits            r0 mask: clears bits in [r5+0x54]->[0x64]
```
