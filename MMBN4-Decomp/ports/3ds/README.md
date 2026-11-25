# MMBN4 Violet Eclipse — New 3DS Port (3DS-1..3DS-3)

Targets **New 3DS / New 2DS XL** (`osSetSpeedupEnable(true)` → 804 MHz + L2 cache) on top of `ports/common/` and a static GBA-only **mGBA 0.10.5** core:
- **Top screen (400×240):** GBA game (240×160 centered, 1:1 crisp pixels).
- **Bottom screen (320×240):** Interactive 4-tab PET (`Status`, `Folder`, `NetMap`, `Custom`) via `citro2d` + touchscreen (`hidTouchRead`).

## Building (from WSL Ubuntu)

Requires devkitPro `3ds-dev` (`devkitARM`, `libctru`, `citro2d`, `citro3d`) in `/opt/devkitpro`:

```bash
cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp
python3 ports/3ds/build_3ds.py
```

Output: `build/3ds/mmbn4.3dsx`.

## SD Card Layout & Hardware Benchmark

```text
sdmc:/3ds/mmbn4/
  mmbn4.3dsx
  bn4_rs_overhaul.gba   (or bn4_bm_overhaul.gba / bn4_rs_us.gba)
  saves/                (mGBA-compatible .sav shared with PC & Wii U)
```

Hold **L + R** on launch (or create `sdmc:/3ds/mmbn4/bench.txt`) to run 600 unthrottled frames and write `sdmc:/3ds/mmbn4/bench.log` (Gate G-3DS: `< 14.0 ms` mean).
