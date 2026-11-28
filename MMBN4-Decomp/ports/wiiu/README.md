# MMBN4 Violet Eclipse — Wii U Port (WU-1 .. WU-6)

Cross-compiled with **devkitPPC + WUT + wiiu-sdl2** on top of `ports/common/` and a static GBA-only **mGBA 0.10.5** core (`libmgba.a`).

**Status** (see `TODO.md`): built and verified in Cemu (ARM64): TV 6x centred, GamePad touch PET (4 tabs), Pro Controller, `ZR` toggles Off-TV, original icon and splash art, and a 600-frame `bench.log`. The Gate G-WU run on a real console is still pending.

## Building (from WSL Ubuntu)

Requires `devkitPPC`, `wut`, `wiiu-sdl2` in `/opt/devkitpro` (or `$DEVKITPRO`), `ext/mgba` (tag 0.10.5) and `cmake` in WSL (`sudo apt install cmake` if not yet installed):

```bash
cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp
python3 ports/wiiu/build_wiiu.py
```

Output: `build/wiiu/mmbn4.rpx` and `build/wiiu/mmbn4.wuhb`. The first run also builds a static GBA-only `build/mgba-wiiu/libmgba.a`. From PowerShell: `wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp && python3 ports/wiiu/build_wiiu.py"`. To test on a PC, open the `.wuhb` in Cemu (`Cemu.exe -g build/wiiu/mmbn4.wuhb`).

To make a release zip without ROMs, run `python3 ports/wiiu/package_wiiu.py` after building. It writes `build/mmbn4-wiiu.zip` with the `.wuhb`, `NOTICE.txt` and a `LICENSES/` folder (mGBA MPL-2.0 with its source pointer, plus the licences of the mGBA bundled libraries that are linked: inih and blip-buf), read from the gitignored `ext/mgba`. The devkitPro libraries' licences are not confirmed yet (`docs/credits.md` row 14), so check them before a public Wii U release.

## SD Card Layout (Aroma)

Copy `mmbn4.wuhb` and your legally dumped/patched `.gba` ROM onto the Wii U SD card:

```text
sd:/wiiu/apps/mmbn4/
  mmbn4.wuhb
  bn4_rs_overhaul.gba      (or bn4_bm_overhaul.gba / bn4_rs_us.gba / bn4_bm_us.gba)
  saves/                   (mGBA-compatible .sav, same format as the PC port)
```

## Running the Real-Hardware Benchmark (Gate G-WU)

1. Optionally copy a savestate (e.g. a battle state) to `sd:/wiiu/apps/mmbn4/bench.ss`.
2. Either hold **L + R** while launching the app, or create an empty `sd:/wiiu/apps/mmbn4/bench.txt`.
3. The app runs 600 unthrottled frames via `ports/common/bench.c` (`OSGetSystemTime`) and writes `mean_ms / p90_ms / p99_ms / max_ms / fps` to `sd:/wiiu/apps/mmbn4/bench.log`.
   - **Gate G-WU:** `< 12.0 ms` mean single-threaded (`PASS`), or `< 16.0 ms` with a core thread.
