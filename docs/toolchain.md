# Toolchain (installed 2026-11-21)

Dev machine: ThinkPad T14s Gen 6, Windows 11 Pro **ARM64** ([hardware/thinkpad-t14s-gen6.md](hardware/thinkpad-t14s-gen6.md)).
Everything under `tools/` and `ext/` is gitignored. Rebuild it from this file.

## Pre-installed on the machine

| Tool | Version | Arch | Notes |
|---|---|---|---|
| Python | 3.12 | ARM64 native | runs `build.py` |
| Git for Windows | 2.55.0 | ARM64 (clangarm64) | |
| CMake | 4.4.0 | | |
| VS 2022 Build Tools (MSVC) | 2022 | Hostarm64 → ARM64 | used to build armips |
| .NET Framework | 4.8.1 | ARM64 native (built into Win 11) | TextPet |
| .NET runtime | 6.0.29 | | not needed by current tools |
| Microsoft OpenJDK | 17.0.21 | | for Ghidra later |

## Project tools (`tools/`)

| Tool | Version | Arch | Source | How installed |
|---|---|---|---|---|
| armips | v0.11.0 (master `62adab4`, 2026-08-01) | **ARM64 native** | github.com/Kingcom/armips | `git clone --recursive` into `ext/armips`, then `cmake -G "Visual Studio 17 2022" -A ARM64`, `cmake --build --config Release --target armips-bin`, copy `armips.exe` to `tools/armips/` |
| Flips | v198 | x64 (Prism emulation) | github.com/Sir-Walrus/Flips (renamed from Alcaro/Flips) | release `flips-windows.zip` |
| TextPet | v1.0.0 | .NET Framework 4.8.1 (AnyCPU → ARM64) | github.com/Prof9/TextPet | release `TextPet.v1.0.0.zip`. Ships `mmbn4bm-us.tpi` / `mmbn4rs-us.tpi` indexes |
| mGBA | 0.10.5 | x64 (Prism emulation) | github.com/mgba-emu/mgba | release `mGBA-0.10.5-win64.7z`, extracted with Windows `tar.exe` |
| Cemu (Wii U emulator) | rhemfur/Cemu ARM64 fork `e488d8c` (2026-10-08) | **ARM64 native** | github.com/rhemfur/Cemu | Binary copied from `C:\proyectos\Silent-Hill-Decomp\tools\cemu\Cemu.exe` (built from that commit; verified there with Vulkan on the Adreno X1-85). Only `graphicPacks` was copied, with a fresh `portable/` (no Silent Hill saves, shader cache or SD card). Source shallow-cloned into `ext/cemu`. Run homebrew with `Cemu.exe -g <file.rpx/.wuhb>`, with the Vulkan graphics API |

| SDL3 | 3.4.20 (`SDL3-devel-3.4.20-VC.zip`) | **ARM64** + x64 + x86 libs | github.com/libsdl-org/SDL | extracted to `tools/sdl3/`. Used by the PC port |

Download SHA-256 prefixes (for reference): mGBA 7z `b497a57c7d909383`, TextPet zip `93cd5e91d5b70230`, Flips zip `802bfb315dca08a2`.

### WSL toolchains (Ubuntu-24.04, **aarch64**)

Already installed (set up for the Silent Hill project): `/opt/devkitpro` with **devkitPPC, wut, libogc**, and portlibs for `wiiu`, `wii`, `gamecube`, `ppc`. That covers the Wii U native port. **Not installed:** devkitARM (needed for NDS and 3DS).

Deferred to M1: **Ghidra + GhidraGBA** (GhidraGBA was last updated in 2021 and may need rebuilding for current Ghidra).

## Reference repos (`ext/`, shallow clones; read-only, never executed)

| Repo | Commit | Date |
|---|---|---|
| Kingcom/armips (full, recursive) | 62adab4 | 2026-08-01 |
| StraDaMa/mmbn-chip-tables | 5d14440 | 2024-10-25 |
| RockmanEXEZone/BN4-WoodMan-Fix | c50df4c | 2016-04-23 |
| nettaisen/nettai | 3a18efe | 2026-11-19 |
| dism-exe/bn6f | d57c196 | 2026-08-10 |
| RockmanEXEZone/MMBN45-English-Translation | 61c8df3 | 2021-09-05 |
| saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless | f2049e7 | 2026-11-19 |
| mgba-emu/mgba (tag 0.10.5; **built** as a static libmgba for the PC port, `build/mgba-<arch>`) | 26b7884 | 2025-03-08 |
| rhemfur/Cemu (ARM64 fork, source of `tools/cemu`) | e488d8c | 2026-10-08 |
| dism-exe/exe4rs (JP Red Sun disassembly, IDA-generated, dormant; reference for function boundaries only) | 1d82058 | 2018-07-27 |

## ROM verification

All three ROMs match **No-Intro** (DAT shipped with mGBA 0.10.5, version 20240104):

| File | No-Intro name |
|---|---|
| roms/bn4_bm_us.gba | Megaman - Battle Network 4 - Blue Moon (USA) |
| roms/bn4_rs_us.gba | Megaman - Battle Network 4 - Red Sun (USA) |
| roms/bn4_bm_jp.gba | Rockman EXE 4 - Tournament Blue Moon (Japan) |

## Build

```
python build.py --check      # identity tests: chip table re-assembles byte-identical (BM + RS)
python build.py              # both versions -> build/bn4_{bm,rs}_overhaul.{gba,bps}
python build.py --hello bm   # hello-world test patch (Cannon 40 -> 99 damage)
```
