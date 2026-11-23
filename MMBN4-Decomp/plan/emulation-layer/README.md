# Emulation Layer per Port — Plan

Every port runs BN4's own code from the player's ROM on an **emulation layer**. A shared **director** around that layer adds the second screen (PET), hooks, saves and platform features. No port waits for the decompilation ([../decomp/](../decomp/README.md)). Decompiled C only gets added later for features the layer can't reach (true widescreen, per-layer stereo beyond what the core exposes, deep battle changes). Written 2026-11-22.

| File | Contents |
|---|---|
| [01-shared-architecture.md](01-shared-architecture.md) | The director library, the emulation backend interface, PET model/UI, hooks, idle hook, benchmark mode, repo layout |
| [02-pc.md](02-pc.md) | PC (Windows ARM64/x64): mGBA, the reference frontend (PC-1 done) |
| [03-wii-u.md](03-wii-u.md) | Wii U: mGBA on Espresso (big-endian), TV game + GamePad PET |
| [04-3ds.md](04-3ds.md) | 3DS: mGBA on the New 3DS (Old 3DS can't keep up), touch PET, stereo depth |
| [05-nds.md](05-nds.md) | NDS: no emulator fits, so a **hypervisor** (GBARunner) with a PET on Engine B |
| [06-milestones-and-risks.md](06-milestones-and-risks.md) | Order, milestones EL-0 to EL-7, gates, risks, decisions |

## Per-port summary

| Port | Emulation layer | Why this one | CPU budget (measured or to measure) | Screens | Status |
|---|---|---|---|---|---|
| **PC** | mGBA 0.10.5 core (interpreter) | Accurate, MPL-2.0, already embedded | About 11% of one Oryon core at 60 fps (measured) | Window: game + PET window | **PC-1 done** |
| **Wii U** | mGBA core built with devkitPPC/wut | Same core as PC. mGBA supports big-endian hosts (official Wii port) | Not yet measured. Expected comfortable (1.24 GHz PPC750, versus the 729 MHz Wii that mGBA supports) | TV: game. GamePad 854×480: PET (touch). Off-TV mode | Toolchain + ARM64 Cemu ready |
| **3DS** | mGBA core (3DS platform code exists upstream) | Proven by Cyberworld Endless on the **New 3DS** | Cyberworld Endless measured: **11 of 16.7 ms** on the New 3DS with an idle hook. **Old 3DS: over a frame, so excluded** (gpSP dynarec = optional spike) | Top 400×240: game (+ stereo depth). Bottom 320×240: PET (touch) | Needs devkitARM in WSL |
| **NDS** | **GBARunner hypervisor**: the game's ARM code runs natively on the ARM9, GBA hardware emulated around it | No software emulator fits (a frame needs about 2× the DS's cycles, and the ROM is 2× its RAM; Cyberworld Endless's analysis) | Whatever the hypervisor leaves. PET must be cheap (tile-based, low refresh) | Top: game. Bottom 256×192: PET on Engine B (touch) | Licence must be clarified first |

## Key ideas

1. **One director, many backends.** The PET, hooks, game knowledge (RAM addresses, flags), saves and config are portable C in `ports/common/`. Each platform adds a backend: video, audio, input, touch, file paths and threads.
2. **Same emulation core where possible (mGBA).** PC, Wii U and 3DS share the core and its behaviour, so a bug found on PC reproduces on the consoles. NDS is the exception and gets a slimmer director.
3. **Measure, don't guess.** Every port starts with a **benchmark mode** (a fixed battle savestate, N frames, ms per frame), as Cyberworld Endless did before excluding the Old 3DS, the PSP and the DS.
4. **Phase 1 patches run everywhere.** The ROM overhaul (armips `.bps`) is just the ROM the layer loads.
