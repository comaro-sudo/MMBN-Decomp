# Gemini round 13 (written by Claude, 2026-11-24; owner decisions in `docs/pending/owner-decisions-2026-11-24.md`)

Your step-2 build was reviewed by Claude: builds and `--check` pass, the `sub_0800A144` pool is unreferenced in both ROMs, `FS_TEXT` is clear of code. Pushed. You may still edit `patch/` and `docs/` until Friday (AGENTS.md §1). Same rules: both ROMs, no ROM bytes, a `.if` overrun guard per slot, `patch/README.md` status per change, `python patch/build.py` + `--check` + a disassembly check in both builds + an in-game test per package, stage by path, don't push, one handoff.

1. **WP-A3 key-item prices, Option 2** (`key-item-prices.md` §3): HP Memory (14 slots), Zenny NaviCust (shops 14–15, 12 slots), Unlocker (4) and LocEnemy (4) at 40% of the price.
   - A new package `wp_a3_key_item_prices.asm`.
   - The prices are save-copied shop stock (`docs/re/shops.md`), so patch the ROM init tables. Say in the README that **existing saves keep their old prices** until the load hook (TODO step 9).
   - Check the new prices in-game on a new save.
2. **WP-B6 Guard A** (`waiting-room-protection.md` §3): bounds-check the slot (1–7) at `WaitingRoomReceiveTradedNavi` (RS `0x0803FA90` / BM `0x0803FA98`), returning the error path without saving. A package `wp_b6_waiting_room_guard.asm`.
3. **Duo rematch S rank, research** (`decomp/notes/duo-rematch-rank.md`; the owner wants S-rank-only climbing):
   - why Duo's battle skips the results screen (`b4 = 0x33` arena? Duo's own end-of-battle code?);
   - whether a Duo rematch can show the normal results screen without breaking the intro;
   - or where the Busting Level is computed, so a hook can read it at Duo's deletion.
   - Exact RS/BM code. Build it if it's safe, else a design.
4. **In-game smoke tests** (`decomp/notes/patch-test-results-6.md`):
   - the 9 friendly matches (spots, gating by tournament round, a win grants the Soul, a loss can retry, the NPC hides once owned);
   - the ACDC Town debris-pile trash can;
   - one regression pass of the lobby, Tuner, status pages and SP+ in the lobby (an S rank against SP unlocks SP+, then 4,095 HP and the 80/20 reward).
5. **Handoff:** list every `patch/` change with its commit, so Claude can update `docs/handoff-violet-eclipse.md` when back.
