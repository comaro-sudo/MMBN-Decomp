# MMBN4 Overhaul

A definitive edition of Mega Man Battle Network 4 (Red Sun / Blue Moon) for every port: every Soul, every chip, every boss in one save ([plan/DEFINITIVE-EDITION.md](plan/DEFINITIVE-EDITION.md)). Solo project. Nothing from the ROMs is in this repository: everything is built from the player's own dumps.

| Folder | Contents |
|---|---|
| [plan/](plan/README.md) | The plan. Sub-plans: [decomp/](plan/decomp/README.md), [emulation-layer/](plan/emulation-layer/README.md) |
| [docs/](docs/) | `design/` (design inputs), `reference/` (repos, [toolchain](docs/reference/toolchain.md)), `hardware/`, `re/` (research notes) |
| [patch/](patch/) | Phase 1 overhaul as armips patches: `python patch/build.py [--check\|--hello] [rs\|bm]` |
| [decomp/](decomp/README.md) | Byte-matching disassembly (WSL: `make -C decomp compare`) |
| [ports/](ports/pc/README.md) | Emulation-layer ports: `pc/` (Windows ARM64/x64) today |
| `test/` | Shared test assets: `saves/`, `lua/` |
| `roms/` | Your own ROM dumps (gitignored, see [roms/README.md](roms/README.md)) |
| `tools/`, `ext/`, `build/` | Third-party tools, cloned reference repos, all build output (gitignored) |
