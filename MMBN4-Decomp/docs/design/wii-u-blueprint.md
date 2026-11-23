# Mega Man Battle Network 4: Nintendo Wii U Porting & Engineering Blueprint

> **Reviewed 2026-11-21.** Corrections are marked ⚠️. How this fits the project: [plan/06 §5](../../plan/06-phase4-native-ports.md) (native) and [plan/04 §2.3](../../plan/04-phase2-handheld-deployment.md) (VC injection).

## Executive Summary

The **Nintendo Wii U** provides a uniquely suited dual-screen ecosystem for *Mega Man Battle Network 4*. Unlike the Nintendo DS or 3DS, the Wii U pairs a **1080p HD Television Display (TV)** with the **Wii U GamePad (854x480 resistive touch screen)**, essentially transforming the GamePad into a life-sized, interactive **PErsonal Terminal (PET)** while rendering the Cyberworld and 6x3 tactical battles in crisp HD on the big screen.

This document outlines the complete technical architecture, dual-screen UX design, rendering pipeline, networking layer, and open-source toolchain required to build a **Native Wii U Port (`.rpx` / `.wuhb`)** of the overhauled *MMBN4* codebase, alongside a secondary **M2 Virtual Console (VC) Injection** workflow for rapid hardware deployment.

## 1. Hardware Target & Porting Approaches

There are two distinct architectural pathways to bringing the overhauled *MMBN4* to Wii U hardware:

| Feature / Aspect | Pathway A: Native Wii U Port (`.wuhb` / `.rpx`) | Pathway B: Wii U GBA Virtual Console (M2 Wrapper) |
| :--- | :--- | :--- |
| **Execution Method** | Native PowerPC (Espresso) C/C++ engine via **WUT** | Official Nintendo/M2 GBA emulator (`alldata.bin` injection) |
| **Resolution & Aspect Ratio** | Native **1920x1080 (16:9)** TV + **854x480** GamePad | Scaled 240x160 (4:3 / pixel-perfect) mirrored on TV & GamePad |
| **Dual-Screen Utilization** | **Asymmetric:** TV shows battles/overworld; GamePad acts as a live touch-screen PET | **Mirrored:** Identical output on both screens (plus VC Restore Point menu) |
| **Multiplayer / Netplay** | Online NetBattles & Chip Trading via **Pretendo / Socket TCP/UDP** | Single-player only (GBA Link Cable unemulated in stock M2 VC) |
| **Development Complexity** | High (Requires C/C++ engine port & GX2 renderer) | Low (Requires patched GBA ROM + `injector` packaging) |

*The remainder of this blueprint focuses primarily on **Pathway A (Native Wii U Port)**, with a deployment appendix for **Pathway B**.*

## 2. Asymmetric Dual-Screen UX Architecture (TV + GamePad)

The defining feature of a native Wii U port is treating the **Wii U GamePad** as Lan Hikari's physical **Advanced PET**.

### 2.1 Television Display (1920x1080 @ 60fps)

* **Overworld & Cyberworld Exploration:**

  * Expand the horizontal and vertical camera bounds from GBA's `240x160` to a native **16:9 widescreen viewport (`480x270` internal integer-scaled 4x to `1920x1080`)**.

  * Relocate HUD elements (HP counter, Zenny, BugFrags, Location banner) cleanly into the corners with crisp HD vector typography, freeing up the central playfield.

* **Tactical Battle Screen:**

  * Render the 6x3 battle grid centered in HD with smooth integer nearest-neighbor scaling (or optional *xBRZ / CRT-Royale* GX2 shaders).

  * Display high-resolution character portraits in the **Emotion Window** (Full Synchro, Anger, Exhausted, Dark Soul) and render dynamic particle overlays for **Double Soul Unison** transformations.

### 2.2 Wii U GamePad Display (854x480 Touch Interface)

The GamePad operates as an interactive, stateful OS running alongside the main game loop (`DRC` — Display Remote Controller):

1. **Exploration Mode (Live PET Dashboard):**

   * **Interactive Real-Time Net Map:** Displays the current Cyberworld area (e.g., *Town Area*, *Park Area*, *Undernet*) with player coordinates, Mystery Data markers, and warp points—eliminating the need to pause or memorize branching paths.

   * **Quick-Action Touch Dock:** One-tap touch buttons for **Jack-In / Jack-Out**, **SubChip Usage** (*SneakRun*, *Untrap*, *LocEnemy*, *Frequency Tuner*), and **MegaMan Status**.

   * **Live PET Email & BBS Ticker:** Incoming emails from Dex, Mayl, Yai, or Dad pop up on the GamePad screen with an audible PET chime, allowing players to read messages without interrupting overworld movement.

2. **Combat Mode (Touch Custom Screen):**

   * **Direct Chip Selection:** During the Custom Gauge phase, the 5–10 drawn Battle Chips appear directly on the GamePad touch screen. Players tap chips to queue them, drag to reorder, and tap the **Program Advance** preview indicator.

   * **Unison & Dark Chip Triggers:** Dedicated illuminated touch buttons on the left and right edges of the GamePad screen allow instant activation of **Double Soul** sacrifice menus or **Dark Chip** emergency draws.

   * **Real-Time Battle Telemetry:** During active combat, the GamePad displays enemy HP totals, elemental weaknesses, active field effects, and a touch-triggered **Pause / Retreat** switch.

3. **Menu Mode (Drag-and-Drop NaviCust & Folder Builder):**

   * **Navi Customizer (NaviCust):** Players use the Wii U stylus to drag, rotate (`L`/`R`), and snap NaviCust programs directly onto MegaMan's 5x5 memory grid with immediate visual feedback for Command Line alignment and bug warnings.

   * **Chip Folder Editor:** Dual-pane touch interface for sorting, filtering by Chip Code (`*`, `A`–`Z`), and building multi-chip folders effortlessly.

4. **Off-TV Play Mode:**

   * Pressing the `-` (**Select**) button dynamically toggles **Off-TV Play**, collapsing the dual-screen layout into a unified single-screen interface on the GamePad so the entire game can be played without a TV.

## 3. Wii U Hardware & Technical Porting Pipeline

### 3.1 CPU & Memory Architecture

* **CPU (`Espresso` — Tri-Core IBM PowerPC 750CL @ 1.24 GHz):**

  * **Core 0:** Dedicated to audio mixing (`AX` / `SNDCore2`) and background network socket polling (NetBattles / Chip Trading).

  * **Core 1 (Main Game Thread):** Executes the ported *MMBN4* C/C++ game logic, collision detection, AI state machines, and GBA bytecode event interpreter at a locked 60.00 Hz (matching GBA's `59.7275 Hz` adjusted cleanly for 60Hz VSync).

  * **Core 2:** Dedicated to asynchronous asset streaming (HD portraits, SDAT/WAV audio banks) and **DRC (GamePad)** UI layout preparation.

* ⚠️ **Note:** endianness only matters for the native (decomp-based) route. With the embedded-emulator route (plan option E), the mGBA core already handles big-endian hosts (it runs on Wii/PPC). Only host-side code that reads GBA RAM needs byte swaps.

* **Endianness Conversion (CRITICAL):**

  * The GBA (`ARM7TDMI`) is **Little-Endian**, whereas the Wii U (`PowerPC Espresso`) is **Big-Endian**.

  * Every multi-byte struct read from original *MMBN4* data archives (map tilemaps, chip tables, sprite OAM headers, script pointers) **must** pass through byte-swapping macros (`__builtin_bswap16` / `__builtin_bswap32`) during asset loading or build-time asset compilation.

### 3.2 Graphics Pipeline: `GX2` Dual-Framebuffer Renderer

The Wii U's AMD Radeon **Latte GPU** is programmed via the **GX2** graphics API:

* **Texture Atlas Compilation:**

  * Extract all GBA 4bpp/8bpp indexed tilesets and sprite sheets into **RGBA8888** or **BC3/DXT5** texture atlases at build time, preserving palette-swapping capability via a lightweight **GLSL/GX2 Fragment Shader** (essential for MegaMan's Double Soul color shifts, Full Synchro glow, and virus tier V1/V2/V3 recolors).

* **Dual-Scanout Execution:**

  * Each frame executes two distinct GX2 render passes:

    1. **Pass 1 (`GX2_SCAN_TARGET_TV`):** Renders the 1920x1080 widescreen Cyberworld/Battlefield to the TV framebuffer.

    2. **Pass 2 (`GX2_SCAN_TARGET_DRC`):** Renders the 854x480 interactive PET touch interface to the GamePad framebuffer.

* **Shader Enhancements:**

  * Implement an optional **Integer Pixel-Art Scaler** alongside a **GBA LCD Color-Correction Shader** (desaturating overly bright GBA colors designed for non-backlit screens so they look natural on modern OLED/LED TVs).

### 3.3 Audio Subsystem (`SNDCore2` / `AX`)

* Convert *MMBN4*'s GBA Sappy MIDI sequences and sample banks into high-bitrate **PCM / DSP-ADPCM** streams played through the Wii U's hardware **AX Audio DSP**.

* **Dual-Speaker Routing:** Route standard BGM and battle SFX to the TV speakers while routing **PET-specific audio cues** (incoming email chimes, Double Soul activation voice clips, Custom Gauge full alerts, and touch-screen button clicks) directly to the **Wii U GamePad stereo speakers** for immersive tactile feedback.

### 3.4 Input & Controller Support

Using the Wii U's `VPAD` (GamePad) and `WPAD` (Pro Controller / Wiimote) libraries:

* **Wii U GamePad:** Full D-Pad / Left Stick movement + Touch Screen PET OS.

* **Wii U Pro Controller / Classic Controller:** Supported for Player 1 (with a toggleable on-screen mini-PET overlay) or **Player 2 in Local Couch PvP NetBattles**.

* **Control Remapping:**

  * `A` Button: Use Battle Chip / Confirm / Talk

  * `B` Button: MegaBuster / Cancel / Dash (auto-dash toggleable in options)

  * `X` Button: Quick-Use Selected SubChip / Folder Quick-Swap

  * `Y` Button: Shield / Reflector / Special NaviCust Ability

  * `L` / `R` Triggers: Open Custom Screen (when gauge is full) / Rotate NaviCust pieces

  * `ZL` / `ZR` Triggers: Instant Full-Synchro Counter Macro / Toggle Off-TV Mode

## 4. Networking & Multiplayer Architecture

Because the Wii U features built-in Wi-Fi and Ethernet adapter support, all GBA Link Cable features are upgraded to full online and local-couch multiplayer:

* **Local Couch NetBattles (1v1 Split-Role):**

  * Player 1 uses the **Wii U GamePad** (picking chips secretly on the GamePad touch screen).

  * Player 2 uses a **Wii U Pro Controller** on the TV (or a second connected GamePad via `DRC2` multi-GamePad patches).

* **Online NetBattles & Chip Library Sync (Pretendo Network / Custom Socket Relay):**

  * Replace GBA `REG_SIOCNT` serial byte transfers with asynchronous **BSD Sockets (`nsysnet`)** over UDP with rollback netcode (adapting the rollback state-saving logic from `nettaisen/nettai`).

  * Support online **Chip Trading**, **Record Comparing** (sharing Ghost Navi times), and **Waiting Room Navi Transfers** across the internet.

## 5. Essential GitHub Repositories for Wii U Development

To build, compile, and package the Wii U port, integrate the following open-source repositories alongside the *MMBN4* disassembly tools:

### Core Wii U Toolchain & SDK

* [**devkitPro/wut**](https://github.com/devkitPro/wut) *(Wii U Toolchain)*

  * **Purpose:** The foundational open-source C/C++ library for native Wii U homebrew development.

  * **Usage:** Provides complete bindings for `gx2` (GPU), `vpad` (GamePad & Touch Screen), `padscore` (Pro Controller), `sndcore2` (Audio), `nsysnet` (Sockets/Multiplayer), and `coreinit` (Multi-core threading).

* [**devkitPro/wut-tools**](https://github.com/devkitPro/wut-tools)

  * **Purpose:** Build utilities for compiling `.elf` binaries into Wii U executable `.rpx` files and packaging self-contained `.wuhb` (Wii U Homebrew Bundle) applications with custom icons, TV/GamePad boot splash screens, and RomFS asset archives.

* [**devkitPro/wiiu-examples**](https://github.com/devkitPro/wiiu-examples)

  * **Purpose:** Official reference implementations for `wut`.

  * **Usage:** Contains working examples for dual-screen TV + GamePad GX2 rendering, resistive touch-screen coordinate polling, and hardware audio mixing.

### Rendering, Shaders & Cross-Platform Engines

* [**decaf-emu/decaf-emu**](https://github.com/decaf-emu/decaf-emu) & [**cemu-project/Cemu**](https://github.com/cemu-project/Cemu)

  * **Purpose:** High-accuracy Wii U emulators with built-in GX2 shader debuggers, memory inspectors, and dual-window (TV + GamePad) desktop rendering.

  * **Usage:** Allows rapid iteration and debugging of the dual-screen PET interface and Big-Endian memory structures on PC before deploying to physical Wii U hardware.

* [**Exzap/CafeGLSL**](https://github.com/Exzap/CafeGLSL)

  * **Purpose:** On-console GLSL-to-GX2 shader compiler for Wii U.

  * **Usage:** Simplifies writing custom palette-swap shaders (for Double Soul recolors) and pixel-art scaling filters without needing proprietary AMD shader compilers.

### Pathway B: Wii U GBA Virtual Console Injection Tools

If deploying a fast-track release using Nintendo's official M2 GBA emulator on Wii U:

* [**stuff-by-3-random-dudes/UWUVCI-AIO-WPF**](https://github.com/stuff-by-3-random-dudes/UWUVCI-AIO-WPF) *(Ultimate Wii U VC Injector)*

  * ⚠️ **Correction:** `NicoAICP/UWUVCI-AIO-WPF` does not exist. The project lives at the link above (the older `NicoAICP/UWUVCI-AIO` is archived).

  * **Purpose:** Automated tool for injecting modified GBA ROMs (such as our single-playthrough, bug-fixed *MMBN4* overhaul) into an official Wii U GBA Virtual Console WUP package.

  * **Usage:** Packages the patched ROM with custom GamePad/TV boot banners, pixel-perfect scaling configs, and save-state support installable directly to the Wii U home menu.

* [**xhp-creations/WUP-Installer-GX2**](https://github.com/xhp-creations/WUP-Installer-GX2)

  * ⚠️ **Correction:** `Chadderz121/wup-installer-gx2` does not exist. The link above is the original (last updated 2017). Check which installer current Aroma setups use before writing the install guide.

  * **Purpose:** On-console WUP package installer for deploying Virtual Console builds to Wii U NAND or USB storage.

## 6. Step-by-Step Wii U Build & Deployment Workflow

```
# 1. Install devkitPro PPC / WUT toolchain
dkp-pacman -S wiiu-dev

# 2. Build the overhauled MMBN4 data assets (Big-Endian byte-swapped for PowerPC)
python3 tools/export_big_endian_assets.py --input mmbn4_overhaul.gba --output romfs/

# 3. Compile the C/C++ Wii U Native Engine (.rpx + RomFS -> .wuhb)
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/WiiU.cmake ..
make -j$(nproc)

# 4. Test dual-screen TV + GamePad rendering in Cemu before hardware deployment
#    (⚠️ Cemu ships x64-only builds; on the ARM64 ThinkPad it runs under Prism emulation. Untested.)
cemu -g mmbn4_solar_eclipse.wuhb

# 5. Deploy to physical Wii U via Wiiload over local Wi-Fi
WIILOAD=tcp:192.168.1.XXX wiiload mmbn4_solar_eclipse.wuhb
```

## 7. Summary Implementation Milestones

1. **Milestone 1: Data Endianness & Asset Pipeline**

   * Write Python build scripts to extract *MMBN4* maps, sprites, chip tables, and `TextPet` scripts into Big-Endian structs packed inside a Wii U `romfs` archive.

2. **Milestone 2: Core Gameplay Loop on `GX2` (TV Render)**

   * Port the C/C++ game loop to `wut`, implementing the palette-swap fragment shader for Double Souls and 16:9 widescreen camera expansion on the TV framebuffer.

3. **Milestone 3: Interactive GamePad PET OS (`DRC` Render)**

   * Build the 854x480 touch-screen UI for live Net Maps, touch Custom Screen chip selection, drag-and-drop NaviCust editing, and GamePad speaker audio routing.

4. **Milestone 4: Multiplayer & Packaging**

   * Implement local couch PvP (GamePad vs. Pro Controller) and `nsysnet` socket netplay, then bundle the final build into a standalone `.wuhb` package with custom TV/GamePad boot artwork.