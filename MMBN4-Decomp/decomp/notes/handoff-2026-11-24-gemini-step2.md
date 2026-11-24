# Handoff: Gemini Violet Eclipse Patch Build — Step 2 Complete (2026-11-24)

## Goal & Current Status

Built, verified (`python patch/build.py` + `--check`, both US RS and US BM ROMs + `.bps` round-trip), tested in the PC port (`mmbn4.exe`), and committed **all 6 packages** from Claude's handoff queue (`handoff-claude-to-gemini-2026-11-23.md` Step 2):

1. **`76383e4` — Bass restore + `sub_0800A144` veneer pool + Duo story grant:**
   - Restored Bass's deletion state (`RS 0x080C4780 / BM 0x080C478C`, sets `0x006F`) to 100% vanilla in both ROMs.
   - Added contiguous 6-slot veneer pool in verified dead function `sub_0800A144` (`0x0800A144`–`0x0800A16C`, RS == BM) and moved `VENEER_BATTLE_EXIT` to `0x0800A144`.
   - Hooked Duo's exclusive deletion-state table pointer `dat_080C04F8[2]` (`RS 0x080C0500 / BM 0x080C050C`, originally `sub_08010DF8`) directly to `duo_defeat_hook` (no veneer needed).
2. **`f8da808` — Prop sprites:**
   - Duo's Signal (Meteor Comp 1) → `list[7] 0x07` (Duo's hexagonal crest, `25 07 1C`, anim 0).
   - ACDC Town Trash Can → `list[7] 0x4A` (debris/wreckage pile, `25 4A 1C`, anim 0).
3. **`4c71998` — DuoSoul complete (palette, passive, emotion-window face fix):**
   - Custom Duo palette (`duo_form_palette_hook` at `0x080107E2`, RS == BM, via `VENEER_DUO_PAL` `0x0800A164`, using `[sprite_hdr+0x34]`).
   - DuoSoul passive (`BN4_FORM_ENTERS[13]` at `0x0800E0D4`): AirShoes + FloatShoes (`0x30`) + B+Left Reflect Shield (`0x6A`), auto-cleaned by `sub_0800D8C2`.
   - `BN4_FACE_GET` (`0x08016A20`) thunk: clamps OOB faces (`> 10` unsigned → clean face `0`; BM form 13 → MetalSoul face `7` with turn counter).
4. **`532543d` — WP-D3 SP+ (`wp_d3_sp_plus.asm`):**
   - 4,095 HP + 10-entry roaming SP table (`0x08006806`), ATK ×1.5 (`0x08013A06`), Speed ×1.25 (`0x08003196`), random 80% 15,000Z (`0x7A98`) / 20% 50 BF (`0xC032`) reward (`RS 0x080F5804 / BM 0x080F5810`), wired into lobby rematches (`0x0E5D`–`0x0E68`), DuoΩ (`0x0E5C`), and roaming SP bosses (`0x0E69`–`0x0E71`).
5. **`9ab3301` — WP-B1 Friendly Matches (`wp_b1_friendly_matches.asm`):**
   - Relocated DenDome Hall `(02,01)`, Castillo WaitRm `(03,02)`, and Colosseum Lobby `(04,06)` NPC lists; added 9 friendly-match NPCs (`0xE0`–`0xE8`, `FC 0E <eid> <sidx>`, normal story tier, retryable, grants Soul on win, hides once owned or after tournament round window).
   - Moved `FS_TEXT` from `0x087FC000` → `0x087FD000` (`FS_TEXT_SIZE = 0x3000`) to give code/data packages headroom (`next free slot: 0x087FC170`).
6. **PC-port bugfixes (this final commit):**
   - Caught & fixed `duo_form_palette_hook` site: `bl sub_08002DEC` is at **`0x080107E2`** (not `0x080107E8`, which sliced `bl sub_0800ACA6` / `bl sub_08002FAE` and froze battle entry on a white screen!).
   - Caught & fixed `sp_plus_reward_hook`: moved from `bl BN4_REWARD_PICK` (`0x080F57F6`, where `r3` and `[sp+0..0x14]` are live arguments) to `BN4_REWARD_CHOOSER` exit **`RS 0x080F5804 / BM 0x080F5810`** (`07 98 08 99`).
   - Preserved `r4-r7` across `duo_defeat_hook` and `duo_grant_rewards`.
   - Restored Duo rematch header to `b4 = 0x33, b6 = 0x9F` (Duo's arena skips results screen natively).

### PC Port Verification Results (All PASS on `bn4_rs_overhaul.gba`)
- **DuoSoul:** transformed cleanly (`form = 13`), custom silver/red/blue/gold Duo palette rendered on screen, top-left emotion window 100% clean (no garbled tiles), `status_bits = 0x30` (AirShoes + FloatShoes), `b_left = 0x6A` (Reflect Shield).
- **Duo's Signal Crest (`list[7] 0x07`):** rendered cleanly in Meteor Comp 1, launched Duo V1 & DuoΩ SP+ battles.
- **Duo Story/Rematch Grant (`duo_defeat_hook`):** killing Duo V1 set **`0x0023 = True` (DuoSoul)** and **`0x0E72 = True` (Duo D)**, and `battle_exit_clear` zeroed `0x020015EC`–`0x020015EF`.
- **SP+ (4,095 HP):** DuoΩ with `0x0E5C` set spawned with **`4095` HP** (`0x0FFF`) on screen (`0x020015EE = 0x01`, `0x020015ED` phase ticking).

## Open Issues / Next Steps

1. **Duo Rematch tier climb (`0x0E5A`–`0x0E5C`):** Since Duo's battle skips the results screen natively (`b4 = 0x33`), `lobby_results_hook` doesn't run after Duo rematches. If S-rank tier climbing (V1 → α → β → Ω → SP+) is needed for Duo rematches, we can advance `0x0E5A`–`0x0E5C` inside `duo_defeat_hook` whenever `[0x020015EF] == 0x8D` (or check Busting Level / battle time).
2. **Friendly matches & Trash Can visual check:** In-game smoke test of DenDome Hall / Castillo WaitRm / Colosseum Lobby friendly-match NPCs and ACDC Town `list[7] 0x4A` debris pile.
3. **Owner decision pending:** WP-A4 key-item prices (`decomp/notes/key-item-prices.md`, Options 1–3) and WP-B6 Waiting Room guards (`decomp/notes/waiting-room-protection.md`).
