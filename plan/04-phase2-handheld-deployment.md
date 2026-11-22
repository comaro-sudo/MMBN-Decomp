# 04 — Phase 2: Handheld Deployment (Fast Track)

Goal: play the Phase 1 overhaul on real DS and 3DS hardware **without** a native port. This delivers most of the "play it on DS/3DS" value for a small fraction of the cost.

## 2.1 Nintendo DS — GBARunner3 (or GBARunner2)

| Task | Notes |
|---|---|
| Compatibility test of the unmodified US BM/RS | Baseline. Record known issues (the WoodMan softlock is reportedly DS-related). |
| Compatibility test of the overhaul ROM | Make sure new ASM hooks don't rely on emulator-only timing. |
| Per-game settings | Save type, timing fixes, or patches GBARunner needs for BN4 (check its compatibility list). |
| **PET bezel / bottom-screen border** | Design a 256x192 PET-themed image for the second screen. Purely cosmetic: a hypervisor can't put game UI there without deep integration. |
| Install guide | `docs/install-ds.md`: flashcart vs. DSi/3DS TWiLight Menu++ setups. |

**Limitation:** no touch Custom Screen, no Net Map and no NiFi in this route. Those need the native port (Phase 4).

## 2.2 Nintendo 3DS — open_agb_firm

| Task | Notes |
|---|---|
| Compatibility test (baseline + overhaul) | open_agb_firm runs the GBA hardware natively, so compatibility should be close to real GBA. |
| Config profile | Scaling mode, colour correction, save type override for the overhaul ROM. |
| Border | Optional frame image if supported. |
| Install guide | `docs/install-3ds.md` |

Alternative for convenience: virtual console injection (GBA VC .cia via community tools). Document it, but don't depend on it.

## 2.3 Exit criteria (M6)

- A full playthrough smoke test on at least one DS setup and one 3DS setup.
- Install guides written.
- Known issues listed in the release notes.
