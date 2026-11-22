# 04 — Phase 2: Handheld & Console Deployment (Fast Track)

Goal: play the Phase 1 overhaul on real DS, 3DS and Wii U hardware **without** a native port. This delivers most of the "play it on DS/3DS" value for a small fraction of the cost.

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

## 2.3 Wii U — GBA Virtual Console injection

| Task | Notes |
|---|---|
| Pick a base VC title | UWUVCI needs a GBA VC base game. Prefer one known to have good compatibility and the right save type (BN4 uses **SRAM** saves, `SRAM_V113` (verified in all three ROMs), so pick an SRAM-save base; check UWUVCI's base list) |
| Compatibility test (baseline + overhaul) | The M2 VC emulator is not mGBA. Check the WoodMan scenario, saving and loading, and the new ASM hooks |
| Package with UWUVCI-AIO-WPF | `stuff-by-3-random-dudes/UWUVCI-AIO-WPF`. It is a Windows WPF app (x64 under Prism is fine). TV and GamePad banners use the PET bezel art |
| Install guide | `docs/install-wiiu.md` (Aroma environment, installer, SD layout) |

**Limitations:** the same picture is mirrored on the TV and GamePad, and there is no link cable, so no NetBattles or trades. The asymmetric PET UI needs the native port (Phase 4, [06 §5](06-phase4-native-ports.md)).

Alternative: RetroArch for Wii U with the mGBA libretro core (verify it is available). It gives better accuracy than VC and supports save states, but it is still single-screen.

## 2.4 Exit criteria (M6)

- A full playthrough smoke test on at least one DS setup and one 3DS setup. The Wii U VC inject boots and saves (needs Wii U hardware).
- Install guides written.
- Known issues listed in the release notes.
