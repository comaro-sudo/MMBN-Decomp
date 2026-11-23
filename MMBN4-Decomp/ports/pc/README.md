# MMBN4 Overhaul — PC port

A native Windows app (ARM64 first, x64 also builds). It runs BN4's own code from **your own ROM** on an embedded [mGBA](https://mgba.io) core (0.10.5, MPL-2.0), presented with SDL3, plus a second **PET window** that prototypes the DS/3DS bottom screen and Wii U GamePad. This is "option E" in [plan/05](../../plan/05-phase3-decompilation.md). The plan is [plan/10-pc-port.md](../../plan/10-pc-port.md).

No Capcom data is in this folder or in the build output. The ROM is read at runtime.

## Build

```
python ports/pc/build_pc.py              # ARM64 -> build/pc-arm64/Release/mmbn4.exe
python ports/pc/build_pc.py --arch x64   # x64 build
python ports/pc/build_pc.py --run        # build + launch from the repo root
```

Needs VS 2022 Build Tools (ARM64 target), CMake, `ext/mgba` (tag 0.10.5) and `tools/sdl3` (SDL3 VC package). See [docs/reference/toolchain.md](../../docs/reference/toolchain.md). The first build also compiles a static libmgba (GBA core only) into `build/mgba-<arch>`.

## Run

```
build\pc-arm64\Release\mmbn4.exe [rom.gba]
```

Without an argument, run from the repo root: it uses `build/bn4_rs_overhaul.gba`, then `roms/bn4_rs_us.gba`.

| Keyboard | Gamepad | GBA |
|---|---|---|
| Arrows | D-pad / left stick (4-way) | D-pad |
| X / Z | East / South | A / B |
| A / S | LB / RB | L / R |
| Enter / Backspace | Start / Back | Start / Select |
| Tab (hold) | Right trigger | Fast forward (4x) |
| F5 / F7 | | Save / load state |
| F1 / F11 / Esc | | PET window / fullscreen / quit |

Saves: `%APPDATA%\MMBN4Overhaul\PC\<code>.sav`, 32 KB SRAM in the same format as mGBA's `.sav` (copy them back and forth). Patched and original builds of one version share a save.

## Dev / test modes

```
mmbn4.exe rom.gba --test 1500 --input "700=START,706=,900=A,906=" --shot out.bmp [--save f.sav]
mmbn4.exe rom.gba --quit-after 420 --pet-shot pet.bmp
```

- `--dump FILE`, `--state-in FILE`, `--state-out FILE` (with `--test`): write EWRAM (`0x02000000`, 256 KB) raw at the end of the run; start from a save state; save the final state. These chain long scripted runs for reverse engineering ([docs/re/](../../docs/re/save-and-flags.md)).
- `--test`: headless and unthrottled. It runs N frames with a scripted input (`frame=KEYS`, keys joined by `+`, empty releases all), saves the last frame as a BMP, and prints the game code, CRC32, audio sample count and RAM values.
- `--quit-after`: the normal windowed path that quits by itself. It logs window geometry and FPS, and optionally saves the PET window's own pixels.

## Layout

| File | Role |
|---|---|
| `src/emu.c/.h` | mGBA core wrapper: load ROM + SRAM, run a frame, video, audio (blip → s16 stereo), keys, bus reads, save states, a quiet logger |
| `src/game.c/.h` | Identifies the version by game code. RAM addresses known per version (Red Sun from Cyberworld Endless's MIT symbol map; Blue Moon not mapped yet) |
| `src/pet.c/.h` | The PET window (status, live RAM, controls) |
| `src/main.c` | SDL3 app: windows sized to the display, input, 59.7275 fps pacing nudged by the audio queue, test modes |

## Licences

- mGBA is MPL-2.0. It is linked unmodified from `ext/mgba`. Binary releases must ship its licence and point to its source.
- SDL3 is zlib.
