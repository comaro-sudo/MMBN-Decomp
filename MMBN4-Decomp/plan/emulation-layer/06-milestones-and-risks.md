# 06 — Milestones, Order, Risks, Decisions

## Order and milestones

| ID | Milestone | Depends on | Notes |
|---|---|---|---|
| **EL-0** | Refactor PC-1 into `ports/common` + `ports/pc`: backend interface, endian-safe access, `PetModel`/`pet_ui`/`ui_draw`, `--bench`, idle hook | PC-1 | PC behaviour unchanged (screenshot + PET read-back checks) |
| **EL-1** | Hooks in the mGBA backend (= PC-2) | EL-0 | BKPT model |
| **EL-2** | **Wii U** WU-1..WU-3 (boot, I/O, hardware bench) | EL-0 | Toolchain and Cemu ready. Needs a Wii U for WU-3 |
| **EL-3** | PET v1 on PC + GamePad (`PET_WIDE`), Off-TV | EL-1, EL-2, RAM map (RE / decomp DC-3) | First "real" second screen |
| **EL-4** | **3DS** 3D-1..3D-3 (boot, bench, bottom-screen PET) | EL-0, devkitARM | Needs a New 3DS for 3D-2 |
| **EL-5** | Touch Custom Screen + Net Map on all mGBA ports | EL-3 | Key macros first, then state writes |
| **EL-6** | **NDS** NDS-1..NDS-3 (hypervisor spike, PET on Engine B) | Licence answer, devkitARM | Gate G-NDS |
| **EL-7** | 3DS stereo depth (PC prototype first), polish, packaging per port | EL-4 | |

Recommended order: **PC → Wii U → 3DS → NDS.** Wii U goes before 3DS because its toolchain, emulator and a proven TV/GamePad renderer already exist, and its CPU margin is likely larger. NDS goes last because it's a different layer, needs a licence answer, and is the riskiest.

## Gates

| Gate | Pass condition | If it fails |
|---|---|---|
| G-WU | mGBA under 12 ms/frame mean on Wii U hardware (or under 16 ms with the core thread) | Idle hook + threading. Then lower audio quality. Then consider gpSP (no PPC dynarec: unlikely to help) |
| G-3DS | Under 14 ms mean on New 3DS with the idle hook | Same mitigations as Cyberworld Endless. The New 3DS is already proven for BN6 |
| G-O3DS | gpSP spike fits a frame on the Old 3DS, *and* you want Old 3DS support | New 3DS only |
| G-NDS | Licence OK and the PET costs no measurable game slowdown | DS = Phase 2 deployment only |

## Risks

| ID | Risk | L | I | Mitigation |
|---|---|---|---|---|
| ER1 | Wii U mGBA build needs platform work (threads, files, big-endian bugs in the director) | Med | Med | Endian helpers from EL-0. RetroArch's Wii U mGBA core proves the core works |
| ER2 | No real hardware to benchmark (Wii U / New 3DS / DS) | ? | High | **Confirm what you own (D-EL1).** Emulator timing is not evidence |
| ER3 | GBARunner licence unclear / not granted | Med | High (for NDS) | Ask early. Fallback to Phase 2 for DS |
| ER4 | Key-macro touch input desyncs from the game's cursor | Med | Med | Read the cursor from RAM between steps. Move to state writes once structures are named |
| ER5 | Reusing GPL-3.0 Silent Hill Wii U code sets this project's licence | Certain if reused | Med | Decide D-EL3. Otherwise write the GX2 layer fresh (or use SDL2) |
| ER6 | Scope: four frontends for a solo developer | High | High | The shared director keeps per-port code small (target under 1.5k lines each). Ports can stop at "boots + PET v1" |
| ER7 | RAM map gaps (Blue Moon unmapped; many PET fields unknown) | High | Med | The PET hides unknown fields. The decomp DC-1/DC-3 symbol work fills them for both versions |

## Decisions for you

| ID | Question | Recommendation |
|---|---|---|
| D-EL1 | Which hardware do you own: Wii U (with Aroma)? New 3DS / New 2DS XL? Old 3DS? DS / DS Lite / DSi? | Determines which gates can be run at all |
| D-EL2 | Port order | PC → Wii U → 3DS → NDS |
| D-EL3 | Project licence for the ports (reuse your GPL-3.0 Silent Hill Wii U GX2/GamePad code, or keep MPL/permissive and write fresh) | If you're fine with GPL-3.0 for the ports, reuse it: it saves weeks. mGBA's MPL-2.0 is compatible with GPL-3.0 |
| D-EL4 | Old 3DS support via a gpSP backend (GPL-2.0) | No, unless you specifically want it |
| D-EL5 | Ask Gericom about GBARunner licensing now (for the NDS PET) | Yes, early. It's the NDS blocker |
