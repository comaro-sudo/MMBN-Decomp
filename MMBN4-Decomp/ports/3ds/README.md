# MMBN4 Violet Eclipse — New 3DS Port (3DS-1..3DS-3)

Targets **New 3DS / New 2DS XL** (`osSetSpeedupEnable(true)` → 804 MHz + L2 cache) on top of `ports/common/` and a static GBA-only **mGBA 0.10.5** core:
- **Top screen (400×240):** GBA game (240×160 centered, 1:1 crisp pixels).
- **Bottom screen (320×240):** Interactive 4-tab PET (`Status`, `Folder`, `NetMap`, `Custom`) via `citro2d` + touchscreen (`hidTouchRead`).

**Status** (see `TODO.md`): the source is written on `ports/common/` but has **never been compiled**: devkitARM, libctru and citro2d are not installed here. `docs/ports-3ds-review.md` is a static review of it. Nothing has run on a 3DS or in an emulator yet, so the G-3DS gate is open.

## Building (from WSL Ubuntu)

Requires devkitPro `3ds-dev` (`devkitARM`, `libctru`, `citro2d`, `citro3d`) in `/opt/devkitpro` (or `$DEVKITPRO`; the script needs `$DEVKITPRO/cmake/3DS.cmake`) and `ext/mgba` (tag 0.10.5):

```bash
cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp
python3 ports/3ds/build_3ds.py
```

Output: `build/3ds/mmbn4.3dsx`. The first run also builds a static GBA-only `build/mgba-3ds/libmgba.a`.

## SD Card Layout & Hardware Benchmark

```text
sdmc:/3ds/mmbn4/
  mmbn4.3dsx
  bn4_rs_overhaul.gba   (or bn4_bm_overhaul.gba / bn4_rs_us.gba)
  saves/                (mGBA-compatible .sav shared with PC & Wii U)
```

Hold **L + R** on launch (or create `sdmc:/3ds/mmbn4/bench.txt`) to run 600 unthrottled frames and write `sdmc:/3ds/mmbn4/bench.log` (Gate G-3DS: `< 14.0 ms` mean).
