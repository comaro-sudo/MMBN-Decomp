# 01 — Setup & Toolchain

## 1. Proposed repository layout

```
MMBN4/
├─ plan/                      # this plan
├─ docs/                      # design doc, repo list, RE notes, offset tables
│  └─ re/                     # one .md per researched system (see 02)
├─ roms/                      # (gitignored) original ROMs, verified by SHA-1
├─ tools/                     # (gitignored or submodules) armips, TextPet, mGBA, flips
├─ ext/                       # (gitignored) cloned reference repos (bn6f, nettai, chip tables…)
├─ src/
│  ├─ common/                 # version-agnostic armips sources
│  ├─ bm/                     # Blue Moon-specific offsets / defines
│  ├─ rs/                     # Red Sun-specific offsets / defines
│  ├─ scripts/                # TextPet script dumps (edited)
│  └─ data/                   # chip tables, encounter tables, shop tables
├─ test/
│  ├─ saves/                  # test save files at key story points
│  └─ lua/                    # mGBA Lua automation scripts
├─ build/                     # (gitignored) output ROMs + .bps patches
└─ build script (PowerShell or Python)
```

✅ Done (2026-11-21): the layout above exists, the ROMs are in `roms/` under canonical names (hashes unchanged), the docs are in `docs/`, and `.gitignore` is written. `build/`, `tools/` and `ext/` exist but are empty and gitignored.

## 2. ROM verification (from the header check already done)

| Canonical name | Game code | SHA-1 |
|---|---|---|
| bn4_bm_us.gba | B4BE | 5e017c803ddc768efb8010314b46bc17e9757f71 |
| bn4_rs_us.gba | B4WE | a97e96a7da03abd70f7953328e511b9fa29179f1 |
| bn4_bm_jp.gba | B4BJ | c4302ada1ff652c740dff51df6dec801fd5b7854 |

- [x] All three match No-Intro (the DAT shipped with mGBA). See [docs/toolchain.md](../docs/toolchain.md).
- The build script must refuse to run if the input hash does not match.

## 3. Tools to install

| Tool | Use | Phase |
|---|---|---|
| Git | Version control | 0 |
| armips | Assemble Thumb/ARM patches into the ROM | 1 |
| TextPet | Extract and insert MMBN text and event scripts | 1 |
| Flips (Floating IPS) | Create and apply `.bps` patches | 1 |
| mGBA (dev build) | Debugger, memory viewer, Lua scripting, GDB stub | 1 |
| Ghidra + GhidraGBA | Static analysis and decompilation of ROM functions | 1, 3 |
| No$GBA (debug) *(optional)* | Alternative debugger with good breakpoint UX | 1 |
| Python 3 | Table generation, build glue, data extraction | 1 |
| devkitPro (devkitARM, libnds, libctru, citro2d/3d) | Native ports | 4 |
| BlocksDS *(alternative to libnds)* | Native NDS port | 4 |
| UWUVCI-AIO-WPF | Wii U GBA VC injection | 2 |
| devkitPPC + wut + wut-tools | Native Wii U port | 4 |
| Cemu *(x64, Prism)* | Wii U testing on the laptop (spike) | 4 |

### 3.1 Dev machine is Windows on ARM64 (ThinkPad T14s Gen 6, Snapdragon X Elite)

Most tools are published as x64 Windows binaries. Windows 11's Prism emulator runs them, slower but fine for these workloads. Prefer native ARM64 where it exists:

| Tool | ARM64 approach |
|---|---|
| armips | Build from source with MSVC ARM64 (CMake), or run the x64 release under emulation |
| TextPet | .NET, so it runs natively if built for or run on the ARM64 .NET runtime |
| Flips | x64 under emulation (tiny tool) |
| mGBA | Check for an ARM64 Windows build. Otherwise run x64 under emulation, or build from source. Confirm the Lua scripting and headless mode work. |
| Ghidra | Java, so install an ARM64 JDK (for example Microsoft OpenJDK or Temurin aarch64) |
| Python | Native ARM64 installer from python.org |
| devkitPro (Phase 4) | Its Windows installer is x86/x64. Option 1: run it under emulation. Option 2: **WSL2 (Ubuntu ARM64) + devkitPro's Linux aarch64 packages.** Test both at Phase 4 kickoff. |

Note: App Control and HVCI are enforced on this machine. If an unsigned tool is blocked, check Windows Security before debugging the tool itself.

## 4. Reference repos to clone into `ext/`

Same as the list in the repo document, plus the items below. Clone only, and **verify each one exists first** (see 00 §3):

- bn6f (engine structure and naming conventions)
- nettai (BN4 memory addresses and battle routines; confirm it really covers BN4)
- TextPet (plus its BN4 game definition, if one exists)
- mmbn-chip-tables (BN4 chip table format)
- BN4-WoodMan-Fix (reference patch, possibly reused directly with credit)
- MMBN45 translation (build pipeline template)
- Cyberworld Endless (ROM-expansion techniques)
- **Also look for:** community BN4 RAM maps and flag lists (The Rockman EXE Zone, Prof9's notes, the TCRF BN4 pages).

## 5. Build pipeline (design only; implement in M0)

```
verify hash → copy ROM → TextPet insert scripts → armips assemble (common + version)
           → fix header checksum → output build/bn4_bm_overhaul.gba
           → flips create .bps → (optional) run smoke tests in mGBA headless
```

Requirements:
- One command builds both versions.
- Deterministic output (same input gives the same SHA-1). Useful for CI and bug reports.
- Free-space management: keep a single `freespace.asm` that allocates ROM regions (the ROM is 8 MB; check for padding at the end, and expand to 16 MB if needed).

## 6. Tasks (M0)

- [x] `git init` + initial commit (`.gitignore` included)
- [x] Move the ROMs into `roms/`, verify hashes
- [x] Move the design docs into `docs/`
- [x] Install the tools and record their versions in [docs/toolchain.md](../docs/toolchain.md) (armips built native ARM64. Flips, TextPet and mGBA installed. Ghidra deferred to M1.)
- [x] Clone the reference repos and record the commit hashes used
- [x] Hello-world patch: Cannon damage 40 → 99 (`python build.py --hello`). It builds, the BPS round-trips, and it changes exactly 1 byte. Chip-table identity test passes for BM and RS.
- [ ] Confirm the hello patch in mGBA by hand: Library/Folder shows Cannon at 99, and it does 99 in battle.
