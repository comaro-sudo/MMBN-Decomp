# 05 — Phase 3: Decompilation (prerequisite for native ports)

The design doc's native NDS and 3DS ports assume "recompile the C/C++ codebase". **That codebase doesn't exist.** This phase creates it. It is the largest and most uncertain part of the whole project. Decide explicitly whether to start it (gate G3 in [09](09-milestones.md)).

## 1. Options

| Option | Description | Effort | Output quality |
|---|---|---|---|
| **A. Matching decomp** (pokeemerald style) | C that compiles to a byte-identical ROM with the original compiler (likely agbcc / an old GCC; must be identified) | Highest (years, multiple people) | Best: provably correct, community-grade |
| **B. Non-matching decomp** | Functionally equivalent C, verified by behaviour tests rather than bytes | High | Good enough for porting, but bugs are harder to rule out |
| **C. Disassembly first** (bn6f style), then incremental C | Buildable assembly, with functions converted to C over time | Medium to start, then incremental | Starts useful early (RE, modding); C coverage grows |
| **D. Static recompilation** (machine-translate Thumb to C, N64Recomp style) | Generate C from the binary automatically, then replace subsystems (graphics, sound, I/O) by hand | Medium | Ugly code, but it is the fastest route to running on PC or 3DS. Hardware-register emulation is still needed. |
| **E. Embedded emulator core + host hooks** (Cyberworld Endless model) | Ship mGBA's core inside a native app. The original code runs unmodified, and the host reads and writes RAM, hooks routines and draws its own UI (second screen, menus) around it | **Low–Medium** | Already proven for BN4/5/6 on PC, 3DS and Switch. Limits: GBA-resolution rendering (no true per-layer stereo or widescreen without PPU work), and mGBA is MPL-2.0 (file-level copyleft). |

**Recommendation (revised 2026-11-21):** **evaluate E first.** Study how Cyberworld Endless structures its mGBA embedding, hooks and 3DS second screen. If E covers the PC + NDS-style UX goals, it replaces most of this phase, and decompilation (C) becomes optional, only for features E can't do (true widescreen, per-layer stereo). The original recommendation was C, with D as an experiment.
- C (a buildable disassembly) also pays off for Phase 1 modding. Once it exists, Phase 1 patches can be ported into it.
- D can be tried as a spike. If a static recompiler gets BN4 booting on PC with a GBA-hardware shim, it can shortcut years of decomp. The native ports would then be "recomp + platform layer + UI rewrites".

## 2. Steps for Option C

1. Identify the compiler (look for agbcc signatures). Check whether bn6f's build setup carries over.
2. Split the ROM into code, data and graphics/audio sections (a splat/ROM-split config, or a custom script).
3. Get a byte-identical rebuild of the ROM from the split disassembly. This is the **first milestone**.
4. Bring in Phase 1 RE labels (`docs/re/`) as symbols.
5. Convert subsystems to C in this order: data structures/save → event script interpreter → battle engine → field/overworld → menus → graphics/sound drivers (last, because these get replaced by the platform layer anyway).
6. Coordinate with the community. Check whether a BN4 disassembly project already exists (Rockman EXE Zone, dism-exe Discord) **before starting**. Joining is far better than duplicating it.

## 3. Exit criteria

- The ROM rebuilds byte-identical from source.
- The game logic (battle, field, scripts, menus) is in C, or at least in labelled asm with clear interfaces to the hardware layer.
- Hardware access (VRAM, OAM, IO registers, SIO, sound) is isolated behind identifiable functions. That is what Phase 4 replaces.
