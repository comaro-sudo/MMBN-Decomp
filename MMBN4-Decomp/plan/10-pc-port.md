# 10 — PC Port (started 2026-11-21)

The owner chose to start the PC port now. No decompilation exists, so it uses **option E** ([05](05-phase3-decompilation.md)): BN4's own code runs from the player's ROM on an embedded mGBA core. The host app (the "director", as Cyberworld Endless calls it) owns windows, input, audio, saves and the PET second screen, reads and writes game RAM between frames, and later hooks game routines.

Code: [ports/pc/](../ports/pc/README.md). Target: ThinkPad T14s Gen 6, Windows 11 **ARM64**, native build. An x64 build is also possible.

## Why this order works for the whole port plan

- The same core plus director design runs on 3DS, Switch, Vita and the web in Cyberworld Endless, so the platform layer (§1 of [06](06-phase4-native-ports.md)) is "mGBA core + per-platform presentation".
- The Phase 1 ROM patches (`build/*.gba`) run unchanged inside it. Overhaul and port don't compete.
- The Wii U TV + GamePad layout = this app's game window + PET window.

## Milestones

| ID | Goal | Status |
|---|---|---|
| **PC-1 Boot** | Native ARM64 app: mGBA 0.10.5 static core, SDL3 window with integer scaling, audio, keyboard + gamepad, SRAM saves in mGBA format, save states, 59.7275 fps pacing, PET window with live RAM, headless test mode | ✅ done: RS/BM/hello build/JP boot; 59.75 fps measured; about 11% of one core; SRAM 32 KB |
| **PC-2 Hooks** | BKPT-based hooks on game routines (see Cyberworld Endless `docs/engine/emulation/hooks.md`). Event queue to the director. First hooks: battle start/end, map change, Custom Screen open | next |
| **PC-3 PET v1** | PET shows the real state: area or map name, HP, Zenny, current chips/folder, battle HP. Needs RAM addresses for both versions (RE-01, and porting the RS symbols to BM) | |
| **PC-4 Touch/mouse Custom Screen** | The chip selection mirrored in the PET window and selectable with mouse or touch, written back as game input | needs PC-2/PC-3 |
| **PC-5 Net Map** | Live map of the current net area in the PET | needs map data RE (RE-16) |
| **PC-6 Settings & polish** | Key remapping, a config file, a scaling/filter option, LCD colour correction, window state memory, x64 build, packaging (zip with licences) | |
| **PC-7 Link / NetBattle** | Two cores in one process (local), then network (study nettai rollback and Cyberworld Endless online) | stretch |

## Decisions taken while building PC-1

- **SDL3 3.4.20** (VC package with ARM64 libs) rather than SDL2.
- **Renderer:** D3D11 first, with D3D12, Vulkan and software as fallbacks. Adreno's OpenGL is weak.
- **Video:** mGBA software renderer, 32-bit XBGR8888, integer-scaled texture. The window scale is the largest that fits the game plus the PET side by side (4× on the 1920×1200 panel).
- **Pacing:** a timer at 280896/16777216 s per frame, ±2% nudged by the audio queue (50 ms target). Vsync is off.
- **Saves:** named by the header game code, so the patched and original ROMs of a version share one save. Format compatible with mGBA.
- **Logging:** mGBA's default logger prints every BIOS call. Only warnings and errors are kept.

## Open questions

- Should the PC port's PET become the place where overhaul features live (Frequency Tuner toggle, Mystery Data tracker), instead of in-ROM UI? That could replace some ROM-hack UI work in Phase 1 for the PC build, but the GBA, DS and 3DS hypervisor builds wouldn't have it.
- Blue Moon RAM map: port Cyberworld Endless's Red Sun symbols by byte-pattern search (D2).
