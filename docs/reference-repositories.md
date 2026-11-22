# GitHub Repository Directory: MMBN4 Overhaul & NDS/3DS Porting

To execute the game design overhaul and hardware porting roadmap outlined in the main blueprint, development relies on three core ecosystems: **MMBN Disassembly & Scripting Tools**, **GBA Binary Patching & Reverse Engineering**, and **Nintendo DS / 3DS Native Homebrew SDKs**.

Below is a curated directory of verified, open-source GitHub repositories required for each stage of the project.

> **Verified 2026-11-21:** all 22 linked repos exist and none are archived. Corrections from that check are marked ⚠️ below. Pinned commits are in [toolchain.md](toolchain.md).

---

## 1. MMBN Disassembly, Modding & Scripting Frameworks

These repositories provide the foundational assembly offsets, script compilers, and data tables specific to the *Mega Man Battle Network* engine.

### Core Engine & Disassembly References
* **[dism-exe/bn6f](https://github.com/dism-exe/bn6f)** & **[Silenthal/bn1](https://github.com/Silenthal/bn1)**
  * **Purpose:** Complete, buildable disassemblies of *MMBN6* and *MMBN1*.
  * **Relevance to MMBN4:** *MMBN4*, *MMBN5*, and *MMBN6* share the same core architectural engine (event scripting opcodes, battle state machines, and sprite/map formats). Studying the labeled routines in `dism-exe/bn6f` makes identifying tournament bracket generators, Double Soul flags, and virus spawn tables in *MMBN4* dramatically faster.
* **[nettaisen/nettai](https://github.com/nettaisen/nettai)**
  * **Purpose:** Rollback netplay and engine documentation for *MMBN4* (*Red Sun* & *Blue Moon*), *MMBN5*, and *MMBN6*.
  * **Relevance to MMBN4:** Contains `docs/engine`, which documents exact memory addresses, battle routines, and serial I/O hooks directly reverse-engineered from *MMBN4*'s disassembly—essential for replacing GBA Link Cable routines with NDS/3DS wireless networking.

### Text, Script & Scenario Editing
* **[Prof9/TextPet](https://github.com/Prof9/TextPet)**
  * **Purpose:** Command-line binary script extraction and insertion tool specifically built for *Mega Man Battle Network* games.
  * **Relevance to MMBN4:** Required for rewriting tournament dialogue, removing forced backtracking triggers, and modifying the event scripts that govern Double Soul unlocks and Nebula story integration.
* **[RockmanEXEZone/MMBN45-English-Translation](https://github.com/RockmanEXEZone/MMBN45-English-Translation)**
  * **Purpose:** Full build pipeline and source scripts for translating and modifying *Rockman EXE 4.5* (which runs on the exact same engine revision as *MMBN4*).
  * **Relevance to MMBN4:** Serves as a working template for structuring an automated `armips` + `TextPet` build pipeline for an *MMBN4*-era ROM.

### Battle Chip, Bug Fixes & Balance Modding
* **[StraDaMa/mmbn-chip-tables](https://github.com/StraDaMa/mmbn-chip-tables)**
  * **Purpose:** Disassembled Battle Chip data tables for *MMBN1–6* formatted for `armips`.
  * **Relevance to MMBN4:** Allows clean editing of chip damage values, Zenny shop costs, Dark Chip penalties, and chip drop tables.
* **[RockmanEXEZone/BN4-WoodMan-Fix](https://github.com/RockmanEXEZone/BN4-WoodMan-Fix)**
  * **Purpose:** Assembly patch fixing the infamous game-breaking WoodMan scenario crash that happens on Nintendo DS hardware and some emulators.
  * ⚠️ **Correction:** the bug affects **both** Red Sun and Blue Moon (the patch ships armips sources for RS/BM in US/JP/EU). Its cause is DS/emulator timing, not player actions. The author allows free reuse (attribution welcome).
  * **Relevance to MMBN4:** Directly solves one of the critical hardware compatibility bugs required for an NDS port.
* **[saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless)**
  * ⚠️ **Correction:** this is **not a ROM hack**. It is an MIT-licensed C roguelike built around BN6 that runs the original games' code on an **embedded mGBA core** from the player's own ROMs (BN4 Red Sun and BN5 Team Colonel join as content), on Windows, Linux, macOS, 3DS, Switch, Vita, Android, iOS and the web, with online play.
  * **Relevance to MMBN4:** (1) `docs/symbols/bn4-red-sun-us.sym` is a ~700-symbol map of the exact US Red Sun ROM we use, plus `docs/rom/` notes (BN4 tables, Souls, battle, encounters, rewards). (2) It is a working example of a **third porting architecture** (embedded emulator core + host-side hooks), including 3DS stereo and second-screen notes.

---

## 2. GBA Assembly & Reverse Engineering Toolchain

Before porting the codebase to ARM9 (NDS) or ARM11 (3DS), all structural changes to *MMBN4* must be compiled and verified against the ARM7TDMI binary.

* **[Kingcom/armips](https://github.com/Kingcom/armips)**
  * **Purpose:** The industry-standard assembler for GBA/NDS ROM hacking.
  * **Usage:** Used to assemble custom ARM/Thumb patches (such as the "Frequency Tuner" virus-tier switcher, persistent Mystery Data flags, and Duo's hitbox adjustments) directly into the base ROM or C-source disassembly.
* **[mgba-emu/mgba](https://github.com/mgba-emu/mgba)**
  * **Purpose:** Cycle-accurate GBA emulator with a built-in GDB stub, Lua scripting engine, and memory/VRAM viewer.
  * **Usage:** Essential for debugging custom tournament bracket logic, tracing `REG_SIOCNT` serial communication registers, and dumping tilemaps/palettes for the NDS/3DS UI overhaul.
* **[NationalSecurityAgency/ghidra](https://github.com/NationalSecurityAgency/ghidra)** (paired with **[SiD3W4y/GhidraGBA](https://github.com/SiD3W4y/GhidraGBA)**)
  * **Purpose:** Reverse-engineering suite with GBA ROM memory-map loaders.
  * **Usage:** Used to decompile unmapped *MMBN4* functions into readable C code, facilitating a clean C/C++ port to NDS and 3DS SDKs.

---

## 3. Nintendo DS (NDS) Porting & Hypervisor Repositories

Depending on whether you choose a **Native ARM9 Port** (like *MMBN5: Double Team DS*) or a **Dual-Screen Hypervisor Wrapper**, these repositories provide the required hardware drivers.

### Native NDS Homebrew Development
* **[devkitPro/libnds](https://github.com/devkitPro/libnds)**
  * **Purpose:** The core C/C++ hardware abstraction library for the Nintendo DS (ARM9 and ARM7).
  * **Usage:** Provides APIs for managing **2D Engine A** (Top Screen battlefield) and **2D Engine B** (Bottom Screen touch PET interface), DMA transfers, hardware timers, and Slot-2 GBA cartridge detection.
* **[devkitPro/nds-examples](https://github.com/devkitPro/nds-examples)**
  * **Purpose:** Official reference implementations for `libnds`.
  * **Usage:** Contains templates for touch-screen coordinate polling, dual-screen tiled background setups, and `dswifi` local wireless (NiFi) packet transmission.
* **[blocksds/sdk](https://github.com/blocksds/sdk)**
  * **Purpose:** Modern, actively maintained open-source Nintendo DS SDK alternative with enhanced ARM7/ARM9 tooling and improved wireless/filesystem support.
  * **Usage:** Ideal for building a modernized NDS native port with Maxmod audio streaming and NitroFS asset loading.

### GBA Hypervisor & Border Injection (Fast-Track NDS Port)
* **[Gericom/GBARunner2](https://github.com/Gericom/GBARunner2)** & **[Gericom/GBARunner3](https://github.com/Gericom/GBARunner3)**
  * **Purpose:** Bare-metal GBA hypervisor that runs GBA code natively on the NDS ARM9/ARM7 CPUs without software emulation.
  * **Usage:** Allows running the overhauled *MMBN4* ROM on DS hardware at full speed while rendering a custom PET bezel on the second screen.

---

## 4. Nintendo 3DS Porting & Stereoscopic 3D Repositories

For a native 3DS port featuring 400x240 widescreen expansion, PICA200 stereoscopic 3D layers, and unified *Red Sun / Blue Moon* cart functionality.

### Native 3DS SDK & GPU Libraries
* **[devkitPro/libctru](https://github.com/devkitPro/libctru)**
  * **Purpose:** Core user-mode system library for Nintendo 3DS homebrew and native ports.
  * **Usage:** Handles Circle Pad analog-to-grid input mapping, 3D depth slider (`osGet3DSliderState()`) polling, touch-screen input, and **3DS UDS (Local Wireless)** multiplayer.
* **[devkitPro/citro3d](https://github.com/devkitPro/citro3d)** & **[devkitPro/citro2d](https://github.com/devkitPro/citro2d)**
  * **Purpose:** Hardware-accelerated wrappers for the 3DS **PICA200 GPU**.
  * **Usage:** Used to render *MMBN4*'s background layers (`BG0`–`BG3`) and sprite objects as textured quads with distinct Z-depth coordinates, creating true stereoscopic 3D separation between the Cyberworld background, battle panels, and attack animations.
* **[devkitPro/3ds-examples](https://github.com/devkitPro/3ds-examples)**
  * **Purpose:** Reference implementations for `libctru`, `citro3d`, and `citro2d`.
  * **Usage:** Provides working code for stereoscopic left/right eye framebuffer rendering and DSP1 audio playback.

### Bare-Metal 3DS GBA Execution (Alternative Deployment)
* **[profi200/open_agb_firm](https://github.com/profi200/open_agb_firm)**
  * **Purpose:** Bare-metal firmware that boots the 3DS's built-in GBA hardware (`AGB_FIRM`) directly with custom display scaling, color correction, and save-type overrides.
  * **Usage:** Provides a zero-latency hardware deployment target for testing the patched *MMBN4* ROM on real 3DS hardware before completing the full `citro3d` widescreen/dual-screen engine rewrite.

---

## Recommended Repository Setup Workflow

```bash
# 1. Clone core MMBN scripting and chip table tools
git clone https://github.com/Prof9/TextPet.git
git clone https://github.com/StraDaMa/mmbn-chip-tables.git
git clone https://github.com/RockmanEXEZone/BN4-WoodMan-Fix.git

# 2. Clone engine documentation for MMBN4 memory/battle routines
git clone https://github.com/nettaisen/nettai.git

# 3. Clone NDS / 3DS reference examples (requires devkitPro toolchain installed)
git clone https://github.com/devkitPro/nds-examples.git
git clone https://github.com/devkitPro/3ds-examples.git
```