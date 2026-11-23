# Mega Man Battle Network 4: Violet Eclipse

A definitive edition of Mega Man Battle Network 4 that unites Red Sun and Blue Moon, for every port: every Soul, every chip, every boss in one save ([plan/DEFINITIVE-EDITION.md](plan/DEFINITIVE-EDITION.md)). Solo project. Nothing from the ROMs is in this repository: everything is built from the player's own dumps.

This folder is the BN4 part of [MMBN-Decomp](../README.md), a collection of Battle Network decomps. Run the commands below from this folder.

| Folder | Contents |
|---|---|
| [`../AGENTS.md`](../AGENTS.md) | Rules for the two AI agents (Claude, Gemini), at the repo root: layout, who owns what, git rules, handoffs |
| `TODO.md` | Short backlog (Gemini's) |
| [plan/](plan/README.md) | The plan. Sub-plans: [decomp/](plan/decomp/README.md), [emulation-layer/](plan/emulation-layer/README.md) |
| [docs/](docs/) | `design/` (design inputs), `reference/` (repos, [toolchain](docs/reference/toolchain.md)), `hardware/`, [`re/`](docs/re/save-and-flags.md) (reverse-engineering notes), `handoff-*.md` |
| [patch/](patch/) | Phase 1 overhaul as armips patches: `python patch/build.py [--check\|--hello] [rs\|bm]` |
| [decomp/](decomp/README.md) | Byte-matching disassembly, Gemini's area (WSL: `make -C decomp compare`) |
| [ports/](ports/pc/README.md) | Emulation-layer ports: `pc/` (Windows ARM64/x64) today; its `--test --dump` mode feeds `docs/re/` |
| `test/` | Shared test assets: `saves/`, `lua/` |
| `roms/` | Your own ROM dumps (gitignored, see [roms/README.md](roms/README.md)) |
| `ref/` | Third-party guides and fan maps used as reference (gitignored, see [ref/README.md](ref/README.md)) |
| `tools/`, `ext/`, `build/` | Third-party tools, cloned reference repos, all build output (gitignored) |
