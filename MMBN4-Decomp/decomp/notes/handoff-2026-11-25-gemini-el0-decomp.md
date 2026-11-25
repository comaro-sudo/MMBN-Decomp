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

- **Wii U `.wuhb` link:** `devkitPPC`, `wut`, and `wiiu-sdl2` are installed in WSL `/opt/devkitpro`, and `ports/wiiu/src/main.c` + `ports/common/*.c` pass `powerpc-eabi-gcc -Wall -Wextra -Werror` cleanly (`07232d6`), but `cmake` is not yet installed in WSL Ubuntu (`sudo apt install cmake` unblocks `python3 ports/wiiu/build_wiiu.py` to build `libmgba.a` and `mmbn4.wuhb`).
- **New 3DS (`ports/3ds/`):** `devkitARM` / `libctru` are not yet installed in `/opt/devkitpro`.
- **DSi:** still awaits `D-EL5` (GBARunner licence check with Gericom).

## Immediate Next Steps

1. **Wii U build & hardware benchmark:** Run `sudo apt install cmake` in WSL, then `python3 ports/wiiu/build_wiiu.py` to produce `build/wiiu/mmbn4.wuhb`; test on Cemu ARM64 and run the `L+R` / `bench.txt` real-hardware benchmark on Wii U (Gate G-WU: `< 12.0 ms` mean single-threaded).
2. **Wii U WU-4 (GX2 GamePad PET):** Add `ui_draw_gx2.c` (reusing GPL-3.0 Silent Hill GX2/GamePad code) to render `PET_LAYOUT_WIDE` (`854x480`) on the GamePad alongside 6× TV output.
3. **New 3DS (`ports/3ds/`):** Install `devkitARM` + `libctru` + `citro2d` and stand up `ports/3ds/` on top of `ports/common/`.
4. **Decomp:** Start DC-5 C (first leaf functions compiled with `arm-none-eabi-gcc` and verified byte-identical) or decode the cutscene bytecode blocks in `code_04`–`code_05`.

## Follow-up Round 2 Commits (2026-11-25)

- `8094af4` — **PC-2**: Interactive 4-tab PET (`Status`, `Folder`, `NetMap`, `Custom`), live EWRAM Folder 1 reader, area-name decoder & topology grid, live Frequency Tuner write buttons (`0x0E58..0x0E59`), and Custom Screen touch input pulse queue (`pet_ui_click` / `pet_ui_pop_keys`), visually verified across all 4 tabs (`--pet-tab 0..3 --pet-shot`).
- `07232d6` — **Wii U (`ports/wiiu/`)**: `CMakeLists.txt`, `build_wiiu.py`, `src/main.c` (WUT + SDL2 + VPAD + built-in `L+R` / `bench.txt` hardware benchmark), `README.md`; verified `-Wall -Wextra -Werror` clean with `devkitPPC`.
- **Decomp (`code_04`–`code_05`)**: Carved 28 additional 100%-pointer tables (`+2,724B`, **62.1%** total coverage, `732,312 / 1,179,456` bytes, `make -B compare` OK on both RS and BM).
