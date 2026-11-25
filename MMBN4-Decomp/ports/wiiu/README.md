# MMBN4 Violet Eclipse — Wii U Port (WU-1 / WU-2 / WU-3)

Cross-compiled with **devkitPPC + WUT + wiiu-sdl2** on top of `ports/common/` and a static GBA-only **mGBA 0.10.5** core (`libmgba.a`).

## Building (from WSL Ubuntu)

Requires `devkitPPC`, `wut`, `wiiu-sdl2` in `/opt/devkitpro`, and `cmake` in WSL (`sudo apt install cmake` if not yet installed):

```bash
cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp
python3 ports/wiiu/build_wiiu.py
```

Output: `build/wiiu/mmbn4.rpx` and `build/wiiu/mmbn4.wuhb`.

## SD Card Layout (Aroma)

Copy `mmbn4.wuhb` and your legally dumped/patched `.gba` ROM onto the Wii U SD card:

```text
sd:/wiiu/apps/mmbn4/
  mmbn4.wuhb
  bn4_rs_overhaul.gba      (or bn4_bm_overhaul.gba / bn4_rs_us.gba / bn4_bm_us.gba)
  saves/                   (created automatically; mGBA-compatible .sav shared with PC)
```

## Running the Real-Hardware Benchmark (Gate G-WU)

1. Optionally copy a savestate (e.g. a battle state) to `sd:/wiiu/apps/mmbn4/bench.ss`.
2. Either hold **L + R** while launching the app, or create an empty `sd:/wiiu/apps/mmbn4/bench.txt`.
3. The app runs 600 unthrottled frames via `ports/common/bench.c` (`OSGetSystemTime`) and writes `mean_ms / p90_ms / p99_ms / max_ms / fps` to `sd:/wiiu/apps/mmbn4/bench.log`.
   - **Gate G-WU:** `< 12.0 ms` mean single-threaded (`PASS`), or `< 16.0 ms` with a core thread.
