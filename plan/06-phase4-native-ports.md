# 06 — Phase 4: Native Ports (PC → NDS → 3DS)

Depends on Phase 3. Covers the design doc's Parts II and III.

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
        PC/SDL backend   |   NDS backend   |   3DS backend
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

## 5. Exit criteria

- PC: a full playthrough on both versions.
- NDS: a full playthrough on hardware, local wireless netbattle between two DS units.
- 3DS: a full playthrough on hardware with stereo 3D on, and the dual-version save shared.
