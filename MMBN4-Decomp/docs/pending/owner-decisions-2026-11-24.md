# Owner decisions, 2026-11-24 (after Gemini's round 12 and step-2 build)

| Topic | Decision | Facts it rests on |
|---|---|---|
| **Duo rematch climb** | **Keep it S-rank only, like the lobby. Research first** how to get a Busting Level for Duo's battle: show its results screen, or compute the rank. Duo's arena skips the results screen natively (header `b4 = 0x33`; `0x3F` broke the battle intro). | Gemini, `decomp/notes/handoff-2026-11-24-gemini-step2.md` |
| **Key-item prices (WP-A3)** | **Option 2: 60% off (40% of the price)** on HP Memory, the Zenny NaviCust programs, Unlocker and LocEnemy. Combined HP Memory + NaviCust: 300,600Z → 119,900Z; Unlocker 4,000Z → 1,600Z; LocEnemy 10,000Z → 4,000Z. BugFrag traders unchanged. | Gemini, `decomp/notes/key-item-prices.md` §3 (exact bytes; BM = RS + 8) |
| **Waiting Room (WP-B6)** | **Slot bounds check only (Guard A):** `WaitingRoomReceiveTradedNavi` rejects a slot outside 1–7 before any write. The Waiting Room never writes Soul flags or chip keys, so no other guard. | Gemini, `decomp/notes/waiting-room-protection.md` §2–3 |
| Push | Gemini's reviewed commits up to `31261d5` pushed. | Claude's review: both builds + `--check` PASS; the `sub_0800A144` veneer pool is unreferenced in both ROMs; `FS_TEXT` (`0x087FD000`) clear of code (ends `0x087FC170`) |
| **Console hardware owned (`D-EL1`, 2026-11-25)** | **Wii U, New 3DS, DSi** (all three primary target gates `G-WU`, `G-3DS`, `G-NDS` can be benchmarked on real hardware; Old 3DS not owned). | Owner reply 2026-11-25 (`plan/emulation-layer/06-milestones-and-risks.md`) |
| **Ports licence (`D-EL3`, 2026-11-25)** | **GPL-3.0 OK for the ports** — reuse the Silent Hill Wii U GX2/GamePad code (compatible with mGBA's MPL-2.0). | Owner reply 2026-11-25 (`plan/emulation-layer/06-milestones-and-risks.md`) |
| **Principia Trilogy (2026-11-25)** | **Deferred ("later")** — design after BN4..BN6. | Owner reply 2026-11-25 |
