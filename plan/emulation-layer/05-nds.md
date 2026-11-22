# 05 — Nintendo DS (and DSi)

## Why not an emulator

Cyberworld Endless's analysis (`docs/engine/emulation/psp-and-ds.md`):

- A DS frame holds about **1.1 M cycles** (ARM9 at 67 MHz), a DSi's about **2.2 M** (133 MHz).
- A BN battle frame took mGBA about 8.4 M and even the dynarec gpSP about 2.4 M cycles.
- gpSP's ARM backend needs ARMv6+. The DS's ARM946E-S is ARMv5.
- The 8 MB ROM is **twice the DS's 4 MB RAM** (the DSi has 16 MB).

**So no software GBA emulator can be the DS layer.**

## The layer: a hypervisor (GBARunner)

GBARunner2/3 run the game's own ARM7TDMI code **directly on the ARM9**, with the GBA's hardware (video, sound, timers, DMA, save) emulated around it through memory protection and abort handlers. It is the only proven way to run GBA games at speed in DS mode. For this project, the "emulation layer" on DS = a fork of GBARunner plus a small PET module.

| Area | Plan |
|---|---|
| Base | **GBARunner3** (newer, active until 2025) or GBARunner2 (mature, DSi-mode compatibility list). Pick after a source read in NDS-1 |
| **Licence (blocker for release)** | Neither repo has a LICENSE file. GBARunner2 is described as GPLv2 on the Emulation General wiki. GBARunner3 states nothing. **Ask Gericom before any public release.** Private experiments are fine |
| Top screen | The hypervisor's GBA picture (Engine A), as today |
| **Bottom screen 256×192** | **PET on Engine B**, `PET_DS` layout: tile-based (BG text layer + a few OBJ icons), updated at **10–15 Hz**, so it costs almost nothing on the ARM9 |
| Reading game state | GBA EWRAM/IWRAM live at fixed DS addresses under the hypervisor, so the director polls them directly (no bus calls, no hooks) |
| Touch | The ARM7 reads the touch panel (it already handles input). The PET maps taps to **key macros** injected into the emulated KEYINPUT (01 §5, mechanism 1) |
| Where PET code runs | In the hypervisor's VBlank handling on the ARM9, with a strict time budget (measure). Fallback: the ARM7, if it has idle time (it runs sound and IO for the hypervisor) |
| Memory | **DS mode:** the ROM is streamed from SD in pages (the hypervisor's cache). The PET must use a few KB. **DSi mode (16 MB):** the whole ROM fits and there are fewer stalls. Prefer DSi/3DS-in-DSi-mode as the primary target |
| Saves | The hypervisor's SRAM handling (BN4 = SRAM 32 KB). Same file format as other ports, if the hypervisor allows |
| States, hooks, depth, audio routing | **Not available** (`caps() = POLL_ONLY`) |
| Required patch | **WoodMan fix (WP-A1)**: the DS-timing crash. Ship the overhaul ROM with it |
| Launcher | TWiLight Menu++ / flashcart, `.nds` + the player's ROM on SD |

## Milestones

| ID | Goal |
|---|---|
| NDS-1 | Build GBARunner3 and GBARunner2 from source (devkitARM in WSL). Run BN4 RS + the WoodMan fix in melonDS and on hardware. Pick a base. Ask about the licence |
| NDS-2 | Measure the hypervisor's spare ARM9 and ARM7 time per frame in a battle and in a net area |
| NDS-3 | Minimal PET on Engine B: HP, area, Zenny from polled RAM, at 10 Hz. Gate G-NDS: no measurable slowdown in battles |
| NDS-4 | Touch dock + touch Custom Screen via key macros |
| NDS-5 | Net Map (tile-based), packaging, install guide |

**If G-NDS fails** (no spare time, or no licence): the DS falls back to Phase 2 deployment (stock GBARunner + bezel, [../04-phase2-handheld-deployment.md](../04-phase2-handheld-deployment.md)), and the DS-style touch experience lives on 3DS and Wii U.
