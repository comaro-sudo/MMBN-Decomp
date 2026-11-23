# Duo Rematch in the Meteor Comp — Research (Round 9, Task 2, 2026-11-23)

All addresses, tables, flags, and battle records below are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`**.

---

## 1. Meteor Comp Maps & Post-Game Reachability (V, R)

The Meteor Comp is CW map group **`0x82`** (5 maps: `(82, 00)` Meteor Comp 1 through `(82, 04)` Asteroid Server / Duo's Chamber):

| Map | Name | RS NPC slot / list (count) | BM NPC slot / list (count) | Warp-in coords `(x, y, z)`, dir | Notes |
|---|---|---|---:|---|---|
| **`(82, 00)`** | **Meteor Comp 1** | `0x0806DA1C` / `0x0806DCCC` (**3 / 16**) | `0x0806DA28` / `0x0806DCD8` (**3 / 16**) | `(236, 132, 0)`, facing `5` (SW) | **Recommended spot:** immediately reachable from Space Center `(04, 09)` in post-game without re-solving all 4 Meteor Comp puzzles; 13 free NPC slots |
| `(82, 01)` | Meteor Comp 2 | `0x0806DA20` / `0x0806DE28` (3 / 16) | `0x0806DA2C` / `0x0806DE34` (3 / 16) | — | Mid-dungeon |
| `(82, 02)` | Meteor Comp 3 | `0x0806DA24` / `0x0806DF7C` (3 / 16) | `0x0806DA30` / `0x0806DF88` (3 / 16) | — | Mid-dungeon |
| `(82, 03)` | Meteor Comp 4 | `0x0806DA28` / `0x0806E0D0` (4 / 16) | `0x0806DA34` / `0x0806E0DC` (4 / 16) | — | Final puzzle room |
| **`(82, 04)`** | **Asteroid Server (Duo's Chamber)** | `0x0806DA2C` / `0x0806E374` (**0 / 16**) | `0x0806DA38` / `0x0806E380` (**0 / 16**) | `(-372, 68, 0)`, facing `1` (NE) | Where story Duo is fought; **has active story continuous script** (`0x0806DC6E`) that triggers the finale cutscene when approaching the server |

- **Post-game reachability:** After beating Duo and saving Clear Data, reloading the Clear Save places Lan right before the finale, so jacking in at Space Center `(04, 09)` immediately enters **Meteor Comp 1 `(82, 00)`** (and the player can also reach `(82, 04)`). Placing Duo's rematch signal in **Meteor Comp 1 `(82, 00)`** avoids both the 4-room puzzle trek and `(82, 04)`'s finale cutscene script (`0x0806DC6E`).

---

## 2. The Post-Game Gate Flag (`0x0500`) (V, R)

Checked against `docs/re/completion-and-new-game-plus.md` §1 and all 17 community saves:
- **`0x0500` (Game Clear flag — RECOMMENDED):** Set by `sub_08000DC2` (the Clear Save right after credits) and **preserved across New Game+** (`sub_08000DE8` explicitly sets `0x0500` on NG+ init).
- **`0x006E`:** Set by `sub_08000DC2` on Clear Save, **but cleared when starting NG+** (`code_00.s:1656`).
- **`0x006F`:** Only set for the Final Boss completion star (DuoΩ / Bass; confirmed `0` on PT 1 and PT 2 saves `4454.sav`, `4569.sav`, `4579.sav`, `7830.sav`, and `1` only on PT 3/4 saves `4752.sav`, `4891.sav`, `22565.sav`).

**Verdict:** Gate the Duo rematch NPC on **`0x0500`** (`.byte 0x05, 0x00, 0x05; .word @@hide`). It unlocks immediately after beating Duo and viewing the credits on any playthrough, and stays unlocked permanently.

---

## 3. Final-Battle Records (`BN4_STORY_BATTLES` 93 & 94) & Plain-Battle Launch (V, R)

`BN4_STORY_BATTLES` (`RS 0x080FA900 / BM 0x080FA90C`, 12 bytes per entry):

| Battle ID | RS addr | BM addr | 8-byte header | Object list (`+8`) |
|---:|---|---|---|---|
| **93** (Duo `0xE0`) | `0x080FAD5C` | `0x080FAD68` | `00 93 64 16 33 17 1F 00` | `00 02 02 00` (MegaMan col 2, row 2), `E0 05 02 01` (Duo `0xE0` col 5, row 2), `FF` |
| **94** (DuoΩ `0xE3`) | `0x080FAD68` | `0x080FAD74` | `00 93 64 16 33 17 1F 00` | `00 02 02 00` (MegaMan col 2, row 2), `E3 05 02 01` (DuoΩ `0xE3` col 5, row 2), `FF` |

### Header byte breakdown & plain-battle recipe
- `b0 = 0x00`: stage/panel layout (plain panels).
- **`b1 = 0x93` (mode byte):** **Bit 7 (`0x80`) skips the battle results screen and disables fleeing.** Clearing bit 7 → **`0x13`** keeps Duo's exact battle mode while **reaching the vanilla results screen** (Busting Level + S-rank check + reward!).
- `b2 = 0x64`: background (**Asteroid / Duo space background**).
- `b3 = 0x16`: BGM (**Save a Planet / VS Duo theme**).
- `b4 = 0x33`, `b5 = 0x17`: battle/camera config.
- **`b6 = 0x1F` (flags byte):** Setting bit 7 → **`0x9F`** suppresses `BN4_ID_RAISE`, so any of the 4 Duo IDs (`0xE0` Duo, `0xE1` Duoα, `0xE2` Duoβ, `0xE3` DuoΩ) is fought at its exact tier!
- `b7 = 0x00`.

### Launching via `START_OW_BATTLE` (`0x08005544`, RS == BM)
**Yes — works as a plain battle with zero story script attached!**
Provide a 12-byte static record per Duo version (4 × 12 + 4 × 9 = 84 bytes total in free space) with header `00 13 64 16 33 17 9F 00` and `obj_ptr -> { 00 02 02 00, <0xE0..0xE3> 05 02 01, FF }`, then call:
```arm
ldr r0, =duo_rematch_rec_vX     ; selected by Duo's tier (0..3 -> 0xE0..0xE3; tier 4 -> 0xE3 + [0x020015EE]=1 for SP+)
mov r1, 1
mov r2, 0xFF
mov r3, 1
bl  START_OW_BATTLE             ; 0x08005544 (RS == BM)
```
This replays the exact final battle (arena, music, Duo AI), reaches the **results screen** (so Busting Level S climbs `0xE0 -> 0xE1 -> 0xE2 -> 0xE3 -> SP+`), and returns MegaMan cleanly to the Meteor Comp with **no ending or credits replay**.

---

## 4. Spot & Sprite for "Duo's Signal" NPC (V, R)

- **Recommended map:** **Meteor Comp 1 `(82, 00)`** (`RS slot 0x0806DA1C / BM slot 0x0806DA28`, 3 existing NPCs → adding 1 uses **4 / 16** slots).
  - Warp-in lands MegaMan at `(236, 132, 0)` facing `5` (SW).
  - Recommended NPC placement: **`(212, 112, 0)`** facing `3` (SE) (one step SW/NW from the entry pad, immediately visible on jack-in; verify visually on `mmbn4.exe` when building).
- **Existing Duo / Meteor sprites (all byte-identical in RS & BM, verified from our Round 8 contact sheets):**
  - **Best match 1:** **`list[7]` `0x2B`** — glowing blue **Meteor/Core orb** (via `.byte 0x25, 0x2B, 0x1C`).
  - **Best match 2:** **`list[6]` `0x63`** — **Duo's Comet/Satellite probe** (via `.byte 0x17, 0x63`).
  - **Best match 3:** **`list[7]` `0x08`** — floating blue **Crystal/Signal** (via `.byte 0x25, 0x08, 0x1C`).

---

## 5. Free Flags for Duo's Tier Counter (V, R)

- **Mystery Data scan (`0x0E30`–`0x0E90`):** Mystery Data flags jump from **`0x0E3A`** straight to **`0x0E80`**, leaving `0x0E3B`–`0x0E7F` completely free of Mystery Data.
- **Current Violet Eclipse allocations in that gap:**
  - `0x0E40`–`0x0E57`: 12 Soul Navis × 2-bit lobby rematch tier.
  - `0x0E58`–`0x0E59`: WP-B2 Frequency Tuner mode.
- **Recommended Duo tier flags:** **`0x0E5A`–`0x0E5C`** (3 bits, supporting all 5 tiers in one counter: `0` Duo `0xE0`, `1` Duoα `0xE1`, `2` Duoβ `0xE2`, `3` DuoΩ `0xE3`, `4` Duo SP+; or 2 bits **`0x0E5A`–`0x0E5B`** if SP+ is tracked separately).
- **Community save check:** Confirmed **`0` across all 17 community saves** (`0x0E5A`–`0x0E5F` all zero).
