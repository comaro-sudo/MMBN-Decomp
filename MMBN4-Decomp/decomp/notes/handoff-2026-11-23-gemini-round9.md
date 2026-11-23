# Handoff — Gemini Round 9 Research (2026-11-23)

## Goal & Current Status
Completed both research-only tasks from `decomp/notes/task-gemini-round9.md` (zero edits to restricted files):

1. **Task 1 — `decomp/notes/duo-soul-research.md`:**
   - **Pseudo-chip `0x16D` (`DuoSoul`):** Complete ROM record in both RS & BM (name `DuoSoul` in Bank 1, `A B C *`, class `0x0F`, `sub_0800B924` already maps `0x16D -> soul_id 13`); only missing its `0x80`-byte Soul icon (`icon` points to blank placeholder `RS 0x087400F8 / BM 0x0873FBEC`).
   - **Core battle tables (all RS == BM) already have 16 entries (`0..15`) with a live `13 = DuoSoul` slot:**
     - `BN4_FORM_LOADOUTS` (`0x080184F0`): `[13] = 0x19FF00FF` (unique charge-shot ID `0x19`).
     - `BN4_FORM_DURATIONS` (`0x08018088`): `[13] = 0x0258` (3 turns).
     - `BN4_FORM_ELEMENTS` (`0x0800C964`): `[13] = 0x00` (Null).
     - `BN4_FORM_ENTERS` (`0x0800E0A0`): `[13] = 0x0800E0E1` (safe `bx lr` no-op; ready to repoint to custom DuoSoul passive).
   - **Free flag & unite class:** Flag **`0x0023`** is 100% free (0 code refs, 0 in all 17 saves); chip byte `+7` unite class **`0x0D`** has 0 chips (100% free).
   - **Missing form sprite:** `list[0]` (`RS 0x08027964 / BM 0x08027968`) has only 13 entries (`0..12`, ~33–40 KB each); `sub_0800B90A` must redirect `soul_id == 13` to avoid reading `list[1][0]`.
   - **Recommendation:** Scope **(b) (palette-swap of MetalSoul `list[0][9]` or GutsSoul `list[0][2]` + 32-byte Duo palette + `0x80`-byte icon + Duo abilities)** is **M** effort and < 1 KB of free space; Scope **(a) (full 40 KB new form sprite)** is **L–XL** and tight against the ~21.5 KB remaining free space.

2. **Task 2 — `decomp/notes/duo-rematch-research.md`:**
   - **Map:** **Meteor Comp 1 `(82, 00)`** (`RS slot 0x0806DA1C / BM 0x0806DA28`, 3/16 NPCs, warp-in `(236, 132, 0)` facing `5`), recommended over Asteroid Server `(82, 04)` (0/16 NPCs, active finale cutscene script `0x0806DC6E`).
   - **Post-game gate flag:** **`0x0500`** (Game Clear star flag, set after credits and preserved across NG+).
   - **Plain-battle launch:** `BN4_STORY_BATTLES` 93/94 (`RS 0x080FAD5C / 0x080FAD68`, `BM 0x080FAD68 / 0x080FAD74`) have header `00 93 64 16 33 17 1F 00`. Launching a 12-byte static record with **`b1 = 0x13`** (clears skip-results bit 7) and **`b6 = 0x9F`** (suppresses `BN4_ID_RAISE`) via `START_OW_BATTLE` (`0x08005544`) replays the exact final battle (BG `0x64`, BGM `0x16`) for `0xE0..0xE3` (+ SP+), reaches the results screen, and returns cleanly with no credits replay.
   - **Signal sprite:** `list[7]` `0x2B` (blue Meteor/Core orb via `.byte 0x25, 0x2B, 0x1C`) or `list[6]` `0x63` (Duo's Comet/Satellite probe via `.byte 0x17, 0x63`).
   - **Free tier flags:** **`0x0E5A`–`0x0E5C`** (3 bits for `0xE0..0xE3 + SP+`; verified 0 in all 17 saves and clear of all Mystery Data flags `0x0E3A..0x0E80`).

## Modified & Relevant Files
- `MMBN4-Decomp/decomp/notes/duo-soul-research.md` (new)
- `MMBN4-Decomp/decomp/notes/duo-rematch-research.md` (new)
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round9.md` (new)

## Immediate Next Steps (for Owner / Claude)
1. Owner decides Duo Soul scope: **(b)** palette-swap MetalSoul/GutsSoul + Duo abilities (recommended, M) vs **(a)** full new form sprite vs drop.
2. Claude implements the Meteor Comp 1 `(82, 00)` Duo rematch NPC using `duo-rematch-research.md` (`0x0500` gate, `0x0E5A..0x0E5C` tier flags, `00 13 64 16 33 17 9F 00` battle records).
