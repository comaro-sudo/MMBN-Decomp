# 01 — Shared Architecture

## 1. Layers

```
┌──────────────────────────────────────────────────────────────────────────┐
│ Frontend (per port): main loop, windows/screens, audio out, input, touch │  ports/pc  ports/wiiu  ports/3ds  ports/nds
├──────────────────────────────────────────────────────────────────────────┤
│ Director (portable C, ports/common):                                     │
│   game.c     version detection, RAM/ROM addresses (from decomp symbols)  │
│   state.c    reads game state once per frame into a PetModel             │
│   pet_ui.c   PET screens laid out per screen size, drawn via ui_draw API │
│   hooks.c    named hooks (battle start/end, map change, custom screen)   │
│   input.c    logical actions → GBA keys; touch → PET or key macros       │
│   saves.c    SRAM files, states, config                                  │
│   bench.c    benchmark + headless test modes                             │
├──────────────────────────────────────────────────────────────────────────┤
│ Emulation backend interface (emu_backend.h)                              │
│   mgba backend (PC, Wii U, 3DS)       gbarunner shim (NDS, reduced)      │
└──────────────────────────────────────────────────────────────────────────┘
```

## 2. Emulation backend interface (`ports/common/emu_backend.h`)

This generalizes today's `ports/pc/src/emu.h`:

| Call | mGBA backend | GBARunner shim (NDS) |
|---|---|---|
| `open(rom, save)` / `close` | as PC-1 | The hypervisor boots the ROM. The shim attaches to its shared state |
| `run_frame()` | `core->runFrame` (optionally on a worker thread) | No-op: the hypervisor runs the game. The director runs from a VBlank callback |
| `set_keys(mask)` | `core->setKeys` | Write the emulated KEYINPUT the hypervisor serves |
| `video()` + `layer_tags()` | Frame buffer, plus mGBA's per-pixel layer info (what Cyberworld Endless uses for 3DS depth) | Not available (the hypervisor draws to Engine A directly) |
| `audio(out, n)` | blip → s16 stereo | Hypervisor-owned |
| `mem_ptr(region)` | `getMemoryBlock` (EWRAM, IWRAM) | Direct pointers: GBA RAM lives in DS RAM where the hypervisor maps it |
| `read/write8/16/32` | Bus calls. Endian-safe helpers (Wii U) | Direct |
| `hook(addr, fn)` | BKPT in the core's ROM copy (Cyberworld Endless model) | Limited. Polling RAM per VBlank instead; hooks only if the hypervisor exposes a trap |
| `save_state/load_state` | Supported | **Not supported** |
| `caps()` | `HOOKS | STATES | LAYERS | AUDIO` | `POLL_ONLY` |

The director checks `caps()` and degrades gracefully: on NDS the PET polls RAM instead of reacting to hooks, and there are no save states.

## 3. Endianness and memory access

- GBA memory is little-endian. The Wii U host is big-endian.
- All director reads of game memory go through `gba_r16/gba_r32` (and writes through `gba_w*`), which swap on big-endian hosts. Never cast a `mem_ptr` to a struct pointer for multi-byte fields. Byte fields are fine.
- Rule enforced from day one on PC, so the Wii U build doesn't surface new bugs.

## 4. The PET: model and UI

- **`PetModel`** (portable): a plain struct filled each frame from RAM or hooks. It holds the scene (overworld / net / battle / custom screen / menu), area, HP/max HP, Zenny, emotion and Soul state, the chips in hand and the selection, enemy HPs, mail count, and the overhaul options (Frequency Tuner tier). Fields fill in as their RAM addresses are found (RE plan and decomp DC-3). Each field has a "known" flag so the UI hides what isn't mapped yet.
- **`pet_ui`** (portable): layouts **per screen class**:

| Class | Size | Port |
|---|---|---|
| `PET_WIDE` | 854×480 | Wii U GamePad, PC PET window (scaled) |
| `PET_QVGA` | 320×240 | 3DS bottom screen |
| `PET_DS` | 256×192 | NDS Engine B. Simplified and **tile-based**, drawn by its own small renderer |

  Layouts follow [docs/design/touch-screen-ux.md](../../docs/design/touch-screen-ux.md) (the MMBN1 DS-style permanent Net Map, MMBN5 DS-style touch Custom Screen and dock, Star Force 3-style battle telemetry).
- **`ui_draw` API** (immediate mode): `rect`, `line`, `text` (built-in bitmap font), `image` (RGBA or 4bpp+palette), `clip`. Implemented by SDL3 (PC), SDL2 or GX2 (Wii U), citro2d (3DS). NDS uses tiles instead.
- **Graphics from the player's ROM:** chip icons, element icons and Navi mugshots are decoded at runtime from ROM pointers (the chip table's icon/palette pointers at entry +0x20). Nothing is shipped.

## 5. Touch → game

Two mechanisms, chosen per feature:

1. **Key macros:** the PET turns a tap into a short GBA key sequence (e.g. tap chip 3 → Right, Right, A), fed through `set_keys` over the next frames. Works on every backend, NDS included. Fragile if the game's cursor state is misread, so it reads the cursor from RAM between steps.
2. **State writes / hooks:** write the game's selection state directly, or answer a hook (mGBA backends). More robust. Needs the RAM/structure knowledge from RE/decomp.

Start with (1) for the touch Custom Screen. Move to (2) when the structures are named.

## 6. Shared performance tools

- **Idle hook:** BN4 busy-waits for VBlank. Halt the CPU there, as Cyberworld Endless's `idle.c` does for BN6 (about 30% less core time). Find BN4's VBlank wait (bn6f's `main_awaitFrame` equivalent, via fingerprinting) and add it to every mGBA backend. It's essential on 3DS.
- **Benchmark mode** (`--bench state.ss N`): load a fixed savestate (battle, net area, menu), run N frames, report mean/p90/p99/max ms. It runs on PC and in the console builds, writing a log file to SD. The results decide each port's go/no-go.
- **Core thread option:** run the core on its own thread where the platform has a spare core (New 3DS third core, Wii U core 0/2). The director takes each finished frame as Cyberworld Endless's `thread.md` describes.

## 7. Repo layout

```
ports/
├─ common/        director + emu_backend.h + mgba backend + pet_ui + tests (no platform headers)
├─ pc/            SDL3 frontend (from PC-1)
├─ wiiu/          wut frontend (GX2 or SDL2), CMake toolchain file, .wuhb packaging
├─ 3ds/           libctru/citro2d frontend, Makefile, .3dsx/.cia packaging
└─ nds/           GBARunner fork notes + PET-on-Engine-B module (if G-NDS passes)
```

- libmgba is built per platform from `ext/mgba` (tag 0.10.5) with the same GBA-only flags (`ports/pc/build_pc.py` list). Cross-compiled with devkitPPC / devkitARM in WSL.
- The PET logic and the backend interface get **unit tests on PC** (headless): the same `--test` input scripts, comparing `PetModel` values at given frames.

## 8. First step: refactor PC-1 (EL-0)

Move `emu.c` → `common/emu_mgba.c` behind `emu_backend.h`. Move `game.c` → `common/`. Split `pet.c` into `common/pet_model.c`, `common/pet_ui.c` (portable, `ui_draw` calls) and `pc/ui_draw_sdl3.c`. Add the `gba_r*` helpers, `--bench`, and the idle hook. The PC behaviour stays the same, checked with the existing `--test` screenshots and PET read-back.
