# 00 — Decisions & Feasibility

## 1. Reality check

| Goal from the design doc | Feasibility | Why |
|---|---|---|
| Gameplay overhaul (Part I) | **High** | Standard MMBN ROM hacking. Tools exist (armips, TextPet, chip tables), and similar hacks exist (WoodMan fix, Cyberworld Endless). |
| Running on DS / 3DS hardware | **High** | GBARunner2/3 (DS) and open_agb_firm (3DS) already run GBA ROMs. A bezel or border is a config/asset task. |
| Native NDS port (touch Custom Screen, NiFi, SDAT) | **Low → Medium, only after decomp** | Needs C source. There is **no MMBN4 decompilation**. bn6f is an *assembly* disassembly of a *different* game. Decompiling roughly 4–8 MB of Thumb code is a multi-year community-scale effort. |
| Native 3DS port (400x240, stereo 3D, dual version) | **Low, only after decomp + platform layer** | Same blocker, plus a renderer rewrite (GBA tile/sprite → GPU quads). |
| Slot-2 crossover bonuses, Pretendo online | **Very low / defer** | Niche hardware. Online play needs server work. Treat as stretch ideas. |

**Conclusion:** Phase 1 is the real product. Phases 3–4 are a separate, much larger project that Phase 1 does not depend on.

## 2. Decisions (recommended defaults; confirm or change)

| ID | Decision | Recommendation | Alternatives |
|---|---|---|---|
| D1 | Implementation method for Phase 1 | **Binary patching with armips** against the original ROMs, plus TextPet for scripts | Start a disassembly first (slow; delays every feature) |
| D2 | Base ROMs | **US Blue Moon (B4BE) and US Red Sun (B4WE)**, one shared source with version-specific offset files. Develop on Blue Moon first. | Single version only |
| D3 | JP Blue Moon (B4BJ) | **Reference only** (compare original text/scripts, cross-check offsets with JP-based community notes) | Also patch JP |
| D4 | Distribution | **BPS patches only.** Never commit or ship ROMs. | xdelta/IPS |
| D5 | Scope of Part I | Ship in **tiers** (see [03](03-phase1-rom-overhaul.md)): quick wins → progression → narrative content | All at once |
| D6 | DS/3DS deployment in the short term | **Hypervisor route** (GBARunner3, open_agb_firm) | Wait for native ports |
| D7 | Native port strategy | **Decomp → platform abstraction layer → PC/SDL port (ThinkPad) → NDS → 3DS** ✅ confirmed | Go straight to NDS (painful debugging, no shared layer) |
| D8 | Version control | Git, private repo; ROMs and build outputs in `.gitignore` | — |
| D9 | Asset policy for native ports | Port **extracts assets from the user's own ROM at build time** (sm64/pokeemerald-PC model). No Capcom assets in the repo. | — |

## 3. Design-doc claims to verify before building on them

- [ ] The exact current rules for how Double Souls are earned per playthrough (which tournament gives which Soul, and how opponents are chosen). WP-P1 is designed on top of this.
- [ ] Whether the WoodMan softlock is Blue-Moon-only and DS-specific as stated, and what `BN4-WoodMan-Fix` actually patches.
- [ ] "Waiting Room buffer overwrites Soul flags". Confirm the bug exists and reproduce it.
- [ ] Every repository in the repo list exists and matches its description (`Silenthal/bn1`, `nettaisen/nettai`, `saschb2b/...Cyberworld-Endless` and `RockmanEXEZone/MMBN45-English-Translation` especially). Fix the `[ blocksds/sdk]` link typo.
- [ ] 3DS bottom screen: the doc says "capacitive/resistive". It is **resistive only**. Fix the doc.
- [ ] Whether the GBA save type is 64 KB Flash (affects save-layout work in WP-P4/P8).

## 4. Open questions for the project owner

1. ~~Is this a solo project?~~ **Answered: yes, solo.** Keep process light: a local `docs/bugs.md` instead of issue trackers, a build script instead of CI, no contribution docs. Releasing early and in small pieces (Tier A) matters more because there is only one maintainer.
2. Should Phase 1 include **new content** (new dungeons, Black Earth sub-areas, Bass XX / Duo Omega)? Content work is about 5x the effort of the systemic changes. Recommendation: plan it as a separate tier (Tier C) after a 1.0 release of the systemic changes.
3. Should the 1.75x Zenny and the 60% price cuts be fixed values, or configurable options in-game?
4. Hard Mode NG+: do you want "Omega Navis" as new bosses (new content) or as re-tuned existing ones (data only)?
5. ~~Is a PC port acceptable?~~ **Answered: yes. Target: the owner's ThinkPad.** D7 is confirmed. The PC port is an actual target, not only a debugging aid (see [06 §2](06-phase4-native-ports.md)).

Still open: 2, 3, 4, and confirmation of D1–D6, D8 and D9.
