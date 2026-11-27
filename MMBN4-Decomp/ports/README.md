# Ports

Native ports of *MMBN4: Violet Eclipse*. They embed a GBA-only mGBA 0.10.5 core and run the game from the player's own ROM (no Capcom data is in the repo). Licence: GPL-3.0 (owner decision `D-EL3`). Run every command from `MMBN4-Decomp/`.

| Folder | Target | Status (see `TODO.md`) | Build |
|---|---|---|---|
| [`common/`](common/) | Shared code: mGBA wrapper, game/RAM maps (RS and BM), PET model and 4-tab UI, benchmark | Used by all three ports | |
| [`pc/`](pc/README.md) | Windows ARM64 / x64 (SDL3) | Done (EL-0..PC-6); release zips verified | `python ports/pc/build_pc.py [--arch x64]` |
| [`wiiu/`](wiiu/README.md) | Wii U (devkitPPC + WUT + wiiu-sdl2) | WU-1..WU-6 verified in Cemu; real-console benchmark pending | `python3 ports/wiiu/build_wiiu.py` (WSL) |
| [`3ds/`](3ds/README.md) | New 3DS (devkitARM + libctru + citro2d) | Source written, never compiled; review in [`docs/ports-3ds-review.md`](../docs/ports-3ds-review.md) | `python3 ports/3ds/build_3ds.py` (WSL) |

Toolchain notes: [`docs/reference/toolchain.md`](../docs/reference/toolchain.md). Plan: [`plan/10-pc-port.md`](../plan/10-pc-port.md) and [`plan/emulation-layer/`](../plan/emulation-layer/).
