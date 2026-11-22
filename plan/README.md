# MMBN4 Overhaul — Project Plan

This folder turns the two source documents into an ordered, executable plan:

- [../docs/design/design-document.md](../docs/design/design-document.md) (what we want)
- [../docs/reference/reference-repositories.md](../docs/reference/reference-repositories.md) (tools we expect to use)
- [../docs/design/wii-u-blueprint.md](../docs/design/wii-u-blueprint.md) (Wii U target: TV + GamePad PET)
- [../docs/hardware/thinkpad-t14s-gen6.md](../docs/hardware/thinkpad-t14s-gen6.md) (dev machine and PC port target: **Windows 11 on ARM64**)

M0 (setup, build pipeline, hello-world patch) is done. Write no overhaul code until the decisions in [00](00-decisions-and-feasibility.md) are confirmed.

> **Project goal:** a definitive version on every port: every Soul, every chip, every boss in one save. See [DEFINITIVE-EDITION.md](DEFINITIVE-EDITION.md).

## Plan files

| # | File | What it covers |
|---|------|----------------|
| 00 | [00-decisions-and-feasibility.md](00-decisions-and-feasibility.md) | Reality check, key decisions, open questions. **Read first.** |
| 01 | [01-setup-and-toolchain.md](01-setup-and-toolchain.md) | Repo layout, tools, ROM verification, build pipeline |
| 02 | [02-reverse-engineering.md](02-reverse-engineering.md) | What must be located in the ROM before each feature can be built |
| 03 | [03-phase1-rom-overhaul.md](03-phase1-rom-overhaul.md) | Every Part I design feature as a work package |
| 04 | [04-phase2-handheld-deployment.md](04-phase2-handheld-deployment.md) | Running the patched ROM on DS/3DS/Wii U hardware (fast track) |
| 05 | [05-phase3-decompilation.md](05-phase3-decompilation.md) | Decompiling MMBN4 to C, needed before any native port |
| 06 | [06-phase4-native-ports.md](06-phase4-native-ports.md) | Platform layer, then PC, NDS, 3DS and Wii U native ports |
| 07 | [07-testing-and-qa.md](07-testing-and-qa.md) | Regression strategy, test saves, automation |
| 08 | [08-risks.md](08-risks.md) | Risk register |
| 09 | [09-milestones.md](09-milestones.md) | Milestone roadmap and the backlog in order |
| — | [emulation-layer/](emulation-layer/README.md) | **Ports on an emulation layer:** shared director + mGBA (PC, Wii U, 3DS) / GBARunner hypervisor (NDS), per-port plans |
| — | [decomp/](decomp/README.md) | **Decompilation plan:** matching disassembly → names → functional C run inside the PC port |
| 10 | [10-pc-port.md](10-pc-port.md) | **PC port (started):** embedded mGBA core + SDL3 + PET window, milestones PC-1..PC-7 |

## The plan in one paragraph

First, ship the gameplay overhaul as a **ROM patch** (armips + TextPet, released as `.bps` files for US Red Sun and Blue Moon). That is achievable and useful on its own. Next, make it **playable on DS, 3DS and Wii U hardware** with existing hypervisors and emulators (GBARunner3, open_agb_firm, Wii U GBA Virtual Console injection) plus a PET bezel. That needs almost no new engineering. Only after that, if the project still has momentum, start a **decompilation** of MMBN4. Native NDS and 3DS ports with touch UI, wireless play and stereoscopic 3D depend on having that C source, and they go through a **PC/SDL port first** so the platform layer can be debugged in comfort. A native **Wii U** port (TV = game, GamePad = PET) follows the PC port, since its two-screen layout is nearly identical. An embedded-mGBA approach (option E, [05](05-phase3-decompilation.md)) may replace the decompilation for all of these.

## Phase map vs. the original roadmap

| Original design doc | This plan |
|---|---|
| Phase 1: Core ROM refactoring | Phase 1 (unchanged in intent; delivered as patches) |
| Phase 2: NDS engine & dual-screen UI | Split: **Phase 2** hypervisor deployment (cheap), **Phase 3** decompilation (huge), **Phase 4** native NDS |
| Phase 3: 3DS widescreen & stereoscopic | **Phase 4** native 3DS (after NDS shares the platform layer) |
| Wii U blueprint (added later): native + VC injection | **Phase 2** VC injection (cheap), **Phase 4** native Wii U (reuses the PC TV/PET layout) |
