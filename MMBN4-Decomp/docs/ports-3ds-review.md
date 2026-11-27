# New 3DS port: static review (2026-11-27)

Scope: `ports/3ds/` (`src/main.c`, `src/ui_draw_c2d.[ch]`, `CMakeLists.txt`, `build_3ds.py`) and the shared `ports/common/`. Nothing was compiled: devkitARM, libctru and citro2d are not installed on this machine.

**How it was checked.** I read every file and compared the 3DS calls with the public sources on GitHub (`master` of `devkitPro/libctru`, `devkitPro/citro2d`, `devkitPro/citro3d`, fetched 2026-11-27): `3ds/gfx.h`, `3ds/services/hid.h`, `3ds/os.h`, `c2d/base.h`, `c2d/text.h`, `c3d/renderqueue.h`, and `citro3d/source/renderqueue.c`, `citro2d/source/base.c`. The mGBA calls in `ports/common/emu_mgba.c` were checked from memory of the 0.10 API, not against the headers.

## Result

No missing includes, wrong types or wrong signatures were found in `ports/3ds/`. Two logic errors would have shown up as soon as the port ran; both are fixed. The rest are gaps and risks.

## Fixed in `ports/3ds/src/main.c`

1. **High: the top screen would never be presented.** The GBA frame is written straight into `gfxGetFramebuffer(GFX_TOP, ...)`, then `gfxFlushBuffers()` is called, but nothing swaps the top buffers. citro3d only calls `gfxScreenSwapBuffers` for screens that a render target is linked to (`needSwapTop` / `needSwapBot` in `renderqueue.c`), and `C2D_CreateScreenTarget(GFX_BOTTOM, ...)` links only the bottom one. Added `gfxScreenSwapBuffers(GFX_TOP, false)` after `gfxFlushBuffers()`. (`gfxSwapBuffers()` would also swap the bottom screen behind citro3d's back, so it is not used.) Not tested on hardware.
2. **High: the GBA audio was never read.** `emu_open()` sets up mGBA's blip buffers (4096 samples), but `main.c` never called `emu_read_audio()`. The PC and Wii U mains read it every frame, and `bench_run()` does too. Without reading, the blip buffers fill after a few frames, and blip_buf only guards this with an `assert` that release builds drop, so this can write past the buffer. Added a per-frame drain into a scratch buffer. There is still no sound: an `ndsp` output is future work (the 32768 Hz rate is already what `ndsp` would want). The README now says so.

## Open findings in `ports/3ds/` (not fixed: not clear errors, or need hardware)

| Sev | Finding |
|---|---|
| Medium | No audio output (see above). |
| Medium | `n3ds_now_ns()` uses `osGetTime()`, which has 1 ms resolution. For the G-3DS gate (mean below 14 ms) the mean is fine over 600 frames, but p90, p99 and max are quantised to whole milliseconds. `svcGetSystemTick()` / `SYSCLOCK_ARM11` is finer. Careful: `ticks * 1000000000` overflows u64 after about 68 s of uptime, so split it into seconds and remainder. |
| Medium | The main-thread stack size is not set. `bench_run()` puts a 16 KB `int16_t samples[4096*2]` on the stack and `qsort` runs there too. I could not confirm the default stack size of a 3dsx from the public headers. If it is 32 KB this is tight. Fix: make the array `static` in `bench.c` (common) or raise the stack with the libctru `__stacksize__` global. |
| Low | `main()` returns 1 or 2 with no message when no ROM is found or the ROM fails to load; the user just sees the app close. A `consoleInit()` message or an error screen would help. |
| Low | `message` (the last PET action) is never cleared on the 3DS (the Wii U port clears it after 2 s). |
| Low | `PetStatus.fps` is a constant `59.7` and `fast_forward` is always false. |
| Low | `c2d_clip()` is empty. Harmless today because `pet_ui.c` never calls `clip`. If clipping is needed, use `C3D_SetScissor` (the bottom screen is rotated, so the rectangle must be rotated too). |
| Low | `mkdir(SAVES_DIR)` only works if `sdmc:/3ds/mmbn4` already exists. It does when the app is launched from there. |
| Info | `-D__3DS__` in `CMakeLists.txt` is also predefined by devkitARM. The values are identical (1), so no warning is expected. |
| Info | The `KEY_*` names, `hidKeysDown/Held`, `hidTouchRead`, `touchPosition.px/py`, `KEY_TOUCH`, `osSetSpeedupEnable`, `aptMainLoop`, and the `C2D_*` / `C3D_*` signatures used in `main.c` and `ui_draw_c2d.c` all match the current public headers. `C2D_DrawText(..., C2D_WithColor, x, y, z, sx, sy, color)` matches the variadic contract. The framebuffer offset `(x*240 + (239-y))*3` and the BGR byte order are correct for the default `GSP_BGR8_OES` top screen, given mGBA's XBGR8888 output. |
| Info | The text buffer holds 2048 glyphs per frame. The busiest tab (Status) uses roughly 400, so it fits. |

## Could not verify

- `$DEVKITPRO/cmake/3DS.cmake` (the raw file in the `devkitarm-cmake` repository was not reachable). So I could not confirm: the `ctr_generate_smdh(<out> NAME ... DESCRIPTION ... AUTHOR ...)` and `ctr_create_3dsx(<target> SMDH <file>)` signatures, that `citro2d citro3d ctru` resolve as plain library names with the toolchain's link directories, and that the libctru include path is added automatically. They follow the usual devkitPro usage but are unconfirmed.
- Whether mGBA 0.10.5 builds as `MINIMAL_CORE` for ARM11 with the flags in `build_3ds.py`. The same flags work for the Wii U (PPC) build, which is a good sign only.
- The mGBA API calls in `emu_mgba.c` (checked from memory, not against `ext/mgba/include`; the PC and Wii U builds compile them, so they are fine in practice).
- Run-time behaviour: speed on hardware (gate G-3DS), the stack size, touch calibration, text size at scale 0.38, and draw order (everything is drawn at z = 0; I did not confirm that citro2d's default depth test puts later calls on top).
- Old 3DS: `osSetSpeedupEnable` is a no-op there, and the G-3DS gate assumes 804 MHz. The memory use of the mGBA core on an Old 3DS is unknown.

## Findings in `ports/common/` (not changed: shared with the PC and Wii U ports)

| Sev | Finding |
|---|---|
| Low | `pet_ui.c`, status tab footer: the help text ("Arrows move  X=A  Z=B  A/S=L/R", "Tab=fast  F5/F7=state  F1=PET") is the PC keyboard layout. On the 3DS and Wii U it is wrong. It needs a per-port string or a layout flag. |
| Low | `pet_model.c`: `emu_read32(game->ram_save_shift) & 0x1FEu`, while `game.h` documents `ram_save_shift` as a `u8`. If it really is a byte, bit 8 of the mask picks up bit 0 of the next byte. Worth confirming against the decomp (`docs/re/save-and-flags.md`). Same read in `pet_model_set_tuner()`. |
| Low | `pet_model.c`: `cid < 389` hard-codes the chip count. It is right for vanilla BN4 only; the Violet Eclipse patch adds chips, which the folder tab then shows as `--`. |
| Low | `emu_mgba.c`: `emu_open()` leaks `emu->fb` and `emu` on the `core->init` failure path. It does not matter in practice (the app exits). |
| Info | `bench.c`: 16 KB stack buffer (see the stack note above). |
| Info | No missing includes or type problems found in `emu_mgba.c`, `game.c`, `pet_model.c`, `pet_ui.c`, `bench.c`, `gba_mem.h`. `ui_draw.h` is plain C99 with no platform dependency. The hard-coded layout offsets used by `pet_ui_click()` (Tuner buttons, Custom grid) match what the draw functions compute. |
