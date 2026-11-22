# 09 — Milestones & Ordered Backlog

Effort estimates assume one person working part-time. Treat them as order-of-magnitude.

| Milestone | Content | Depends on | Rough effort | Release |
|---|---|---|---|---|
| **M0 Setup** | Git, layout, tools, ROM hashes, hello-world patch, verified repo list | — | 1 week | — |
| **M1 RE baseline** | RE-01..05, RE-14, RE-17 confirmed (BM), symbols file | M0 | 3–6 weeks | — |
| **M2 Quick wins** | WP-A1, A2, A4, A6 (both versions) | M1 | 2–3 weeks | **v0.1** |
| **M3 QoL complete** | WP-A3, A5 | M2 | 3–5 weeks | **v0.2** |
| **M4 Progression rework** | WP-B1, B2, B3, B4, B9 | M3, RE-06/07/15/18 | 3–5 months | v0.5 beta |
| **M5 Polish & balance** | WP-B5, B6, B7, B8, full playthrough tests | M4 | 1–2 months | **v1.0** |
| **M6 Handheld & console deployment** | GBARunner3 / open_agb_firm / Wii U VC (UWUVCI) testing, bezel, install guides | v0.1+ (can run in parallel from M2) | 1–2 weeks | docs |
| **— Gate G2 —** | Decide whether to do Tier C content and/or Phase 3 | v1.0 | — | — |
| **M7 Narrative content** | Tier C (C1–C6) | G2, RE-16 | 6–12+ months | v2.0 |
| **M8 Decomp / recomp** | Phase 3 (community-coordinated) | G2/G3 | years | — |
| **M9 PC port (ThinkPad)** | Embedded mGBA core (option E) + SDL3, PET window. See [10](10-pc-port.md). **Started 2026-11-21, PC-1 done** | ~~M8~~ none (option E) | months | PC build |
| **M10 NDS native** | Phase 4b | M9 | months | NDS build |
| **M11 3DS native** | Phase 4c | M9 (+M10 UI) | months | 3DS build |
| **M12 Wii U native** | Phase 4d: TV + GamePad PET, Off-TV, `.wuhb` | M9 (can run parallel to M10/M11) | months | Wii U build |

## Gates

- **G1 (end of M1):** is the RE knowledge good enough to do B1 (deterministic brackets) safely? If not, reorder: B2/B3 first, B1 last.
- **G2 (after v1.0):** content (Tier C) vs. porting (Phase 3) vs. stop. Do not do both at full scale at once.
- **G3 (before M8):** does a community BN4 disassembly or decomp exist? Join it, or start one? Did the static-recomp spike work?

## Immediate next actions (the first 10)

1. Answer the remaining open questions (2–4) in [00 §4](00-decisions-and-feasibility.md) and confirm decisions D1–D6, D8 and D9. Questions 1 (solo) and 5 (PC port on the ThinkPad) are answered.
2. ✅ `git init` + `.gitignore`. Move the ROMs into `roms/` and the docs into `docs/`.
3. ✅ Check the ROM hashes against No-Intro.
4. ✅ Verify every repo in the repo list exists. Fix the list (typo, the 3DS touch screen note).
5. ✅ Install armips, TextPet, Flips, mGBA and Python. (Ghidra + GhidraGBA moved to M1.)
6. ✅ Clone the reference repos into `ext/`. Read BN4-WoodMan-Fix. (nettai's docs/engine is still to be read.)
7. Search the community for existing BN4 RAM maps, flag lists and any disassembly effort.
8. ✅ Build the hello-world patch end to end (string or chip-damage change → `.bps`). The manual in-game check is still pending.
9. Make test saves S01–S04 on Blue Moon.
10. Start RE-01 (save layout) and RE-03 (playthrough counter readers).
