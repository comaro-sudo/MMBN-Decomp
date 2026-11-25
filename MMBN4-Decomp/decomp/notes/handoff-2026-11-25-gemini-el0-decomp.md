# Handoff: EL-0 Ports Refactor & Decomp Table Carving (2026-11-25)

Author: Gemini (executing owner-directed unblocked Phase 2 EL-0 + Decomp work).

## Goal & Current Status

Completed and verified both requested unblocked workstreams:

1. **Ports — EL-0 Shared Architecture Refactor (`c934453`)** *(verified)*
   - Extracted platform-independent emulation, game RAM map, PET model/UI, and benchmark logic into `MMBN4-Decomp/ports/common/`.
   - Refactored `MMBN4-Decomp/ports/pc/` into a thin SDL3 frontend implementing `UiDraw`.
   - Added Blue Moon (`B4BE`) EWRAM map alongside Red Sun (`B4WE`), including anti-cheat save-block relocation (`0x02002130 + (u32(0x02001550) & 0x1FE)`), live HP/MaxHP bar, Zenny, BugFrags, Map `XX:YY`, PT, Double Souls (`N/12`), Frequency Tuner (`Auto/T1/T2/T3`), Battle HP, and RNG.
   - Added portable `--bench <state.ss> <N>` mode reporting mean / p90 / p99 / max ms and FPS.
   - Enabled mGBA's built-in `idleOptimization = detect` in `emu_mgba.c`.
   - **Verified** on Windows ARM64 (`python ports/pc/build_pc.py`, MSVC 19.44, 0 warnings):
     - `--test 60`: RS (`AGB-B4WE`) and BM (`AGB-B4BE`) both PASS (`audio_samples=47531`, `nonblack_pixels=2244`, `ram rng=A338244F`).
     - `--bench` (600 frames):
       - RS overworld (`rs_higsby.ss`): `mean=0.768 ms`, `p90=0.811 ms`, `p99=1.062 ms`, `max=1.237 ms` (`1302.0 fps`).
       - RS battle (`rs_fspp_bat.ss`): `mean=0.968 ms`, `p90=1.060 ms`, `p99=1.913 ms`, `max=7.018 ms` (`1032.8 fps`).
       - BM overworld (`bm_higsby.ss`): `mean=0.812 ms`, `p90=0.932 ms`, `p99=1.056 ms`, `max=1.436 ms` (`1232.0 fps`).
     - `--quit-after 30 --pet-shot --state-in`: visually inspected both RS & BM PET screenshots; live HP `780/780`, PT4, Map `00:06`, Zenny `82320`, BugFrags `12`, Souls `12/12`, Tuner `Auto`, RNG `516DB7C2` match on both versions.

2. **Decomp — Mixed & Pointer Table Carving (`a57a907`)** *(verified)*
   - Carved 7 tables across `code_00`, `code_02`, `code_03`, and `code_09` in both `segments_rs.toml` and `segments_bm.toml`, regenerated with `disasm.py`, and merged via `merge_bm.py`:
     - `code_00`: `0x0800597C..0x08005AFC` (`uuuu`, 24 × 16B DMA/VRAM descriptors, 384B)
     - `code_02`: `0x08022344..0x08022404` (pointer table, 192B), `0x0802AFD0..0x0802B1BC` (map-group script pointer table, 492B), `0x0802B1BC..0x0802BCCC` (`uw`, 354 × 8B map-group script lists, 2,832B)
     - `code_03`: `0x0804563C..0x0804620C` (`uuww`, 189 × 16B descriptor table with 2 ROM pointers, 3,024B)
     - `code_09`: `0x08103118..0x08103150` (14/14 pointers, 56B), `0x08103648..0x0810379C` (85/85 pointers, 340B)
   - **Verified** in WSL (`make -B compare VERSION=rs` and `VERSION=bm`):
     - `build/rs/bn4_rs.gba: OK`
     - `build/bm/bn4_bm.gba: OK`
     - Coverage: **61.9%** (`729,588 / 1,179,456` bytes, up from `61.2%`, **+7,320 bytes**).

## Key Decisions & Constraints

- **Note for Claude on `ports/` ownership:** `ports/` is normally Claude's area; this EL-0 refactor was executed per direct owner instruction. Every touched file is listed below and committed cleanly in `c934453`.
- **Endianness invariant:** `ports/common/gba_mem.h` provides `gba_r16/r32/w16/w32`; `pet_model.c` uses `emu_read*` (host-endian) and never casts raw EWRAM pointers.
- **BN4 save-block relocation:** `shift` at `0x02001550` is a **u32 masked with `0x1FE`** (`0x000..0x1FE`), not `u8 & 0xFF`.

## Modified & Relevant Files

- **Committed in `c934453` (EL-0):**
  - Added `MMBN4-Decomp/ports/common/`: `gba_mem.h`, `emu_backend.h`, `emu_mgba.c` (moved from `ports/pc/src/emu.c`), `game.h`, `game.c`, `ui_draw.h`, `pet_model.h`, `pet_model.c`, `pet_ui.h`, `pet_ui.c`, `bench.h`, `bench.c`.
  - Added `MMBN4-Decomp/ports/pc/src/ui_draw_sdl3.{h,c}`.
  - Updated `MMBN4-Decomp/ports/pc/CMakeLists.txt`, `ports/pc/src/main.c`, `ports/pc/src/pet.{h,c}`.
  - Removed old `MMBN4-Decomp/ports/pc/src/emu.h`, `game.{h,c}`.
- **Committed in `a57a907` (Decomp):**
  - `MMBN4-Decomp/decomp/config/segments_rs.toml`, `segments_bm.toml`
  - `MMBN4-Decomp/decomp/asm/code_00.s`, `code_02.s`, `code_03.s`, `code_09.s`
- **This commit:** `MMBN4-Decomp/TODO.md`, `MMBN4-Decomp/decomp/notes/handoff-2026-11-25-gemini-el0-decomp.md`.
- Working tree clean after commit & push.

## Open Issues / Blockers

- **DC-5 C:** `arm-none-eabi-gcc` is not yet installed in WSL Ubuntu (only `binutils-arm-none-eabi` is installed); run `sudo apt install gcc-arm-none-eabi` to unblock DC-5 C compilation.
- **New 3DS (`ports/3ds/`):** `devkitARM` / `libctru` / `citro2d` are not yet installed in `/opt/devkitpro`.
- **DSi:** still awaits `D-EL5` (GBARunner licence check with Gericom).

## Immediate Next Steps

1. **Wii U WU-4 (GX2 GamePad PET):** Port `ui_draw_gx2.c` (reusing GPL-3.0 Silent Hill GX2/GamePad code in `C:\proyectos\Silent-Hill-Decomp\sh-gc-disc\wiiu_port\src\gamepad_screen.c`) to render `PET_LAYOUT_WIDE` (`854x480`) on the GamePad alongside 6× TV output, and test in Cemu + real Wii U hardware.
2. **New 3DS (`ports/3ds/`):** Install `devkitARM` + `libctru` + `citro2d`, wire `ui_draw_c2d.c` on top of `ports/common/`, and test in Citra/Azahar + real New 3DS hardware.
3. **DC-5 C:** Install `gcc-arm-none-eabi` in WSL, compile first leaf functions with `arm-none-eabi-gcc -mthumb -O2`, and verify byte-identical RS & BM ROMs.

## Follow-up Round 2 & 3 Commits (2026-11-25)

- `8094af4` — **PC-2**: Interactive 4-tab PET (`Status`, `Folder`, `NetMap`, `Custom`), live EWRAM Folder 1 reader, area-name decoder & topology grid, live Frequency Tuner write buttons (`0x0E58..0x0E59`), and Custom Screen touch input pulse queue (`pet_ui_click` / `pet_ui_pop_keys`), visually verified across all 4 tabs (`--pet-tab 0..3 --pet-shot`).
- `288d986` — **PC-3**: Live ROM `BN4_CHIP_TABLE` (`0x080197EC`) damage & element color-coding in Folder tab + `F2` Off-TV single-window mode (game + PET side-by-side).
- `07232d6` + Wii U build commit — **Wii U (`ports/wiiu/`)**: `wiiu_toolchain.cmake`, `CMakeLists.txt`, `build_wiiu.py` (using venv `cmake 4.4.5` + `devkitPPC` + `wut` + `wiiu-sdl2`), `src/main.c`, `README.md`. Built static big-endian PPC `libmgba.a` (`MINIMAL_CORE=1`) and `build/wiiu/mmbn4.wuhb` (949 KB). **Verified in Cemu ARM64:** booted `bn4_rs_overhaul.gba` (`AGB-B4WE`), executed 600 frames, and wrote `bench.log` (`mean_ms=14.668`, `68.2 fps` in Cemu).
- `5c46201` — **Decomp (`code_04`–`code_05`)**: Carved 28 additional 100%-pointer tables (`+2,724B`, **62.1%** total coverage, `732,312 / 1,179,456` bytes, `make -B compare` OK on both RS and BM). Audited all remaining 161 `.if VERSION_BM` blocks across the 1.18 MB shared code tree (real version-specific asset/code diffs).
- `d5625d0` — **Wii U WU-4**: Dual-screen GX2 via `wiiu-sdl2` (`SDL_WINDOW_WIIU_TV_ONLY` 1920×1080 integer 6× centered + `SDL_WINDOW_WIIU_GAMEPAD_ONLY` 854×480 interactive PET via `ui_draw_sdl2.c` + built-in 5×7 font + calibrated VPAD touch + `ZR` Off-TV toggle). Built 966 KB `build/wiiu/mmbn4.wuhb` and verified booting cleanly in Cemu ARM64.
- **PC-6 (`27b37ef`)**: Added `ports/pc/src/config.{h,c}` (`mmbn4.ini` with 11 remappable keys + `off_tv` pref) and `ports/pc/package_pc.py`. Built and packaged **both** ARM64 (`mmbn4-pc-arm64.zip`, 1.35 MB) and x64 (`mmbn4-pc-x64.zip`, 1.41 MB) with zero warnings.
- **Wii U WU-6 (`a3f14ed`)**: Added Pro Controller (`KPAD` + `WPADEnableURCC`) support + `ports/wiiu/gen_assets.py` generating original Violet Eclipse `icon.png` (128×128), `tv_splash.png` (1280×720), `drc_splash.png` (854×480); built 1.1 MB `build/wiiu/mmbn4.wuhb` and verified clean boot in Cemu ARM64.
- **New 3DS (`e01a011`)**: Added `ports/3ds/` (`CMakeLists.txt`, `build_3ds.py`, `src/ui_draw_c2d.{h,c}`, `src/main.c`, `README.md`) on top of `ports/common/` (`citro2d` bottom PET + touchscreen, top GBA, 804 MHz speedup, `L+R` bench). Ready to build once devkitPro `3ds-dev` (`devkitARM` + `libctru` + `citro2d`) is installed.
- **DC-5 C + `agbcc` (`d6d09e3`, `9217f19` + follow-up)**: Built native aarch64 `pret/agbcc` into gitignored `MMBN4-Decomp/tools/agbcc/`, extended `decomp/tools/c_to_inc.py --agbcc [--O1]` + `decomp/Makefile`, and decompiled **15 C functions** across 3 files: `src/bios_swi.c` (8 BIOS SWIs), `src/m4a_cmds.c` (6 pure `agbcc -O2` `m4a` handlers: `sub_08113C10..sub_08113C74`), and `src/agb_sram.c` (`sub_0811189C` `MemCopyForward` compiled with `agbcc -O1`) — **both RS and BM verified `make -B compare` OK (100% byte-identical)**.
- **Cutscene bytecode macros (`include/cutscene_macros.inc`, `tools/disasm_cutscene.py`, `tools/emit_cutscenes.py`)**: Widened `emit_cutscenes.py` to direct-scan all of `code_04`–`code_05` and emitted **all 396 RS+BM-identical dispatch tables (6,698 bytes total)** directly into `asm/code_04.s`..`code_05.s` with zero new `.if VERSION_BM` blocks (**62.7% coverage**, `739,010 / 1,179,456` bytes, `6,600` externs, both RS & BM `OK`).
- **New 3DS blocker note**: `apt.devkitpro.org` / `pkg.devkitpro.org` return HTTP 403 (Cloudflare anti-bot gate) to CLI clients; install devkitPro `3ds-dev` interactively to build `ports/3ds/build_3ds.py`.
