# 06 — Phase 4: Native Ports (PC → NDS → 3DS, + Wii U)

> **Port strategy updated 2026-11-22:** every port runs on an emulation layer first. See [emulation-layer/](emulation-layer/README.md) (mGBA on PC/Wii U/New 3DS, GBARunner hypervisor on NDS). This file describes the *native* (decomp-based) end state, which is now optional and per feature.

Depends on Phase 3 (decomp, or option E: embedded mGBA core). Covers the design doc's Parts II and III and [docs/wii-u-blueprint.md](../docs/wii-u-blueprint.md).

## 1. Architecture: platform abstraction layer (PAL)

```
          ┌─────────────────────────────────────────┐
          │  Game logic (decomp C) — unchanged      │
          └───────────────┬─────────────────────────┘
                          │ PAL interface
   ┌────────────┬─────────┼──────────┬──────────────┬────────────┐
   │ Video      │ Input   │ Audio    │ Save         │ Link/Net   │
   │ (BG0-3,OBJ │ (pad,   │ (Sappy   │ (Flash →     │ (SIO →     │
   │  palettes) │  touch) │  seq →   │  file/EEPROM)│  socket/   │
   │            │         │  mixer)  │              │  NiFi/UDS) │
   └────────────┴─────────┴──────────┴──────────────┴────────────┘
        PC/SDL backend  |  NDS backend  |  3DS backend  |  Wii U backend
```

Two ways to implement the video layer:
1. **GBA PPU emulation in software** (render the GBA tile/sprite state into a framebuffer). Easiest and most faithful. Fine on PC and 3DS, tight on DS.
2. **Native mapping** (NDS 2D engines are close relatives of the GBA PPU; 3DS uses textured quads on the GPU). Needed for stereo 3D depth layers and for DS performance.

## 2. Step 4a — PC / SDL2 port (confirmed target: the owner's ThinkPad)

- Purpose: debug the PAL with modern tools (sanitizers, debugger, hot reload) before dealing with handheld constraints. The PC build is also a deliverable in its own right.
- Deliverable: the full game playable on the ThinkPad from the user's ROM (asset extraction at build or first run).
- Hardware (from [docs/hardware/thinkpad-t14s-gen6.md](../docs/hardware/thinkpad-t14s-gen6.md)): ThinkPad T14s Gen 6, **Snapdragon X Elite X1E78100 (ARM64, 12 cores)**, Adreno GPU, 32 GB RAM, Windows 11 Pro ARM64.
  - **Build natively for `ARM64` Windows.** An x64 build would also run under Prism, but native avoids emulation overhead and is what the machine is for. Optionally also produce an x64 build so other PCs can run it.
  - Use the **software GBA PPU renderer** (video option 1 in §1), upscaled with integer scaling and presented with SDL's renderer (D3D11 on Adreno). CPU and GPU headroom is huge for this workload.
  - Input: keyboard plus any XInput gamepad. Mouse/TrackPoint (or touch, if this SKU's panel has it) for the bottom-screen window.
  - Laptop-friendly: frame limiter or vsync (no busy-waiting, which protects battery life), and windowed or borderless fullscreen.
  - Graphics driver caveat: Adreno's Windows drivers have weaker OpenGL than D3D. Prefer the D3D11/D3D12 backend in SDL, and avoid depending on desktop OpenGL.
- Toolchain: **MSVC (Visual Studio 2022+, ARM64 target) + CMake + SDL2/SDL3 (both ship ARM64 Windows support)**. Native build on the laptop, no cross-compiling. If the Phase 3 route ends up being static recompilation (option D), the generated C compiles for ARM64 the same way.
- Bonus features that test the PAL: a second window simulating the DS bottom screen (touch Custom Screen, Net Map). **Prototype the dual-screen UX here.**

## 3. Step 4b — Native NDS (design doc Part II)

| Feature | Plan | Notes |
|---|---|---|
| Top screen game, centred 240x160 + bezel or HUD relocation | Engine A, BG/OBJ mapped from GBA state | Decide bezel vs. HUD move with mockups |
| Bottom screen touch PET: Net Map, objectives, SubChips, Jack-in/out | Engine B, new UI code | New UI code, not a port |
| Touch Custom Screen (tap, drag, Soul/Dark Chip buttons) | Engine B | Needs refactoring of the Custom Screen logic so it can be driven by touch input |
| Audio | **Revised:** run the Sappy (m4a) mixer on ARM7, or convert to SDAT/Maxmod | The design doc's "convert to SDAT" is a big asset task. Prototype the mixer route first. |
| Saves | Flash/EEPROM cart save. 3 slots + library partition | Slot count limited by save size |
| Link → NiFi | Replace SIO with dswifi / local wireless | Netbattle and trade timing must be reworked as messages, not register polling |
| Slot-2 crossover bonuses | **Stretch** | Only DS/DS Lite have Slot-2 |
| Toolchain | BlocksDS (preferred, modern) or devkitPro libnds | Choose at kickoff |

Performance risk: ARM9 at 67 MHz is about 4x the GBA's CPU. That is enough for logic, but software rendering would be tight, so use native 2D engine mapping.

## 4. Step 4c — Native 3DS (design doc Part III)

| Feature | Plan | Notes |
|---|---|---|
| Rendering | citro3d/citro2d. BG layers and OBJs as textured quads | Reuses the PAL video state |
| Stereoscopic depth | Per-layer Z as in the doc (−2.0 / −0.5 / 0 / +1.0), scaled by the 3D slider | Make it configurable. Verify each map's layer use (layers differ per scene). |
| 400x240 widescreen overworld | Widen the camera, load extra map columns | **Hard:** maps, NPC culling, scripts and camera locks assume 240px. Expect per-map fixes. Battle stays 4:3 with side art. |
| Bottom 320x240 PET UI | Reuse the NDS UI logic at a higher resolution | Touch screen is resistive |
| Circle Pad with grid snapping in battle | Input layer | Small |
| ZL/ZR hotkeys | Input layer | New 3DS only |
| Dual-version (RS+BM) in one title | Build both version datasets, choose on the title screen, shared library/vault save | Legal model: extract from both user ROMs |
| UDS local wireless | Replace SIO as on NDS | Share the NDS netcode message protocol |
| Online (Pretendo / NEX) | **Stretch.** Consider a simple relay server or rollback netplay (nettai) instead | Big scope |

## 5. Step 4d — Native Wii U (from docs/wii-u-blueprint.md)

Order: after the PC port. It can run in parallel with 3DS. The PC build's two-window layout (game + PET) is the prototype: **TV = game window, GamePad = PET window.**

| Feature | Plan | Notes |
|---|---|---|
| Toolchain | devkitPPC + `wut` + `wut-tools` (`dkp-pacman -S wiiu-dev`), output `.wuhb` | Run devkitPro under WSL2 ARM64 or x64 emulation (same question as NDS/3DS, see [01 §3.1](01-setup-and-toolchain.md)) |
| **Endianness** | Big-endian PowerPC. With **option E** the mGBA core already handles it (it runs on Wii). Only host code that reads GBA RAM swaps bytes. With **decomp**, every struct loaded from ROM data needs swapping or build-time conversion | One more reason to prefer option E. Add `pal_read16/32` helpers from day one, even on PC, so the code is endian-clean |
| TV rendering (GX2) | GBA frame (software PPU) → texture → integer-scaled to 1080p with optional LCD colour correction | The blueprint's 16:9 `480x270` widescreen has the same per-map caveat as 3DS widescreen (§4). Treat it as a stretch goal |
| GamePad PET (854x480 touch) | Reuse the PET UI from the PC/NDS/3DS work: Net Map, touch Custom Screen, NaviCust drag and drop, folder editor, mail ticker | Resolution-independent UI layout is needed. Design it for 320x240 (3DS) and 854x480 from the start |
| Off-TV play | Toggle: the whole game plus a compact PET on the GamePad | |
| Audio | Mixer output → AX. PET cues routed to the GamePad speakers | Routing needs the audio layer to tag sounds as "PET" vs. "world" |
| Input | VPAD (GamePad) + WPAD/KPAD (Pro Controller) | Same input abstraction as PC |
| Local couch NetBattle (GamePad vs. Pro Controller) | Two link-cable endpoints in one process (two emulated GBAs, or two game instances) | New. Interesting, but stretch |
| Online (sockets, rollback) | Share the netcode with PC/NDS/3DS. Study nettai's rollback design | Stretch, as for 3DS |
| Testing | Cemu (x64-only release, so it runs under Prism on the ThinkPad; Vulkan on Adreno is untested) plus real hardware | **Spike early:** does Cemu run acceptably on this laptop? If not, test on hardware only |

Wii U hardware: the CPU is a 1.24 GHz tri-core PowerPC 750 (Espresso). That is plenty for an mGBA core plus the UI, since mGBA already runs full speed on the 729 MHz Wii.

## 6. Exit criteria

- PC: a full playthrough on both versions.
- NDS: a full playthrough on hardware, local wireless netbattle between two DS units.
- 3DS: a full playthrough on hardware with stereo 3D on, and the dual-version save shared.
- Wii U: a full playthrough on hardware with the GamePad PET working, Off-TV play, and `.wuhb` packaging.
