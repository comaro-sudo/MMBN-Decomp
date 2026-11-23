# 03 — Wii U

**Layer:** mGBA 0.10.5 core cross-compiled with devkitPPC + wut (big-endian PowerPC 750 "Espresso", 3 cores at 1.24 GHz). **Why next after PC:** the toolchain is already installed (WSL aarch64: devkitPPC, wut, `libSDL2`, `libz`), a native ARM64 Cemu is in `tools/cemu`, and you already have a GX2 TV + GamePad renderer from the Silent Hill port.

## Hardware mapping

| Area | Plan |
|---|---|
| Core build | libmgba with the GBA-only flags, `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/WiiU.cmake`. mGBA has no upstream Wii U frontend, but its core builds for big-endian PPC (the official Wii port), and RetroArch's mGBA core runs on Wii U. Expect small platform fixes (threads, file I/O, `FIXED_ROM_BUFFER`) |
| Threads | Core on Espresso core 1 (the main one). Director, PET and audio on core 0/2 via `coreinit` threads. Start single-threaded, and add a core thread only if the benchmark says so |
| **TV** | Game frame from mGBA → GX2 texture, integer **6×** (1440×960, centred in 1920×1080) or 4× in 720p. Optional LCD colour correction. Optional PET bezel art around it |
| **GamePad (854×480)** | `PET_WIDE` layout: Net Map, battle telemetry, touch Custom Screen, dock (Folder, SubChip, NaviCust, Jack-out, Frequency Tuner) |
| Off-TV play | Toggle: game at 2× (480×320) on the GamePad plus a compact PET in the remaining 374 px. Same layout as PC single-window mode |
| Rendering API | **Option A (recommended):** reuse the Silent Hill port's GX2 TV/DRC setup (`gpu_gx2.c`, `gamepad_screen.c`). Proven, but **GPL-3.0** (see D-EL3). **Option B:** SDL2 for Wii U (`libSDL2.a` present). Check whether it can drive the DRC as a second target; the Silent Hill port also has `gpu_sdl_wiiu.c` to compare |
| Audio | 32 kHz or 48 kHz s16 from blip → SDL2 audio or AX. PET sounds routed to the GamePad speaker (AX DRC channel) |
| Input | VPAD (GamePad buttons, stick snapped to 4-way in battle, touch → PET) + KPAD (Pro Controller) |
| Saves | `sd:/wiiu/apps/mmbn4/saves/<code>.sav` (mGBA format, the same file as PC) |
| ROM | The player puts their `.gba` (or the patched overhaul) in `sd:/wiiu/apps/mmbn4/`. Nothing shipped |
| Packaging | `.wuhb` via wut-tools (icon, TV/DRC splash) |
| Endianness | Director reads via `gba_r16/32` (01 §3). mGBA's internals handle the guest |

## Milestones

| ID | Goal |
|---|---|
| WU-1 | libmgba builds for Wii U. A minimal `.wuhb` boots the ROM with the picture on the TV (Cemu ARM64) |
| WU-2 | Audio, GamePad input, SRAM save and load. `--bench` log to SD |
| WU-3 | **Hardware benchmark** (needs a Wii U): battle/net/menu states, ms per frame. Gate G-WU: under 12 ms mean single-threaded, or under 16 ms with the core thread |
| WU-4 | GamePad PET (`PET_WIDE`) with live state; Off-TV toggle |
| WU-5 | Touch Custom Screen, Net Map (reaching the consoles from PC-4/PC-5) |
| WU-6 | Pro Controller, settings, packaging, install guide |

## Testing

- Cemu (ARM64, Vulkan) for function and layout. **Cemu timing doesn't say anything about real hardware speed.** Only hardware benchmarks count for G-WU.
- The same input scripts as the PC `--test` mode, run in a debug `.wuhb` that writes screenshots and `PetModel` dumps to SD, compared with PC output.
