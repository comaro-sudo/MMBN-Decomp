# MMBN-Decomp

A collection of decompilation projects for the *Mega Man Battle Network* series. Each game has its own self-contained folder: its decomp, plans, docs, ROM patch and ports. Nothing from the ROMs is in this repository. Everything is built from the player's own dumps, which go in each game's gitignored `roms/` folder.

Status as of 2026-11-27. Work order: BN4 first, then BN1, 2, 3, 5 and 6.

## Status at a glance

| Project | What | Status |
|---|---|---|
| [MMBN4-Decomp/](MMBN4-Decomp/README.md) | Battle Network 4 (Red Sun / Blue Moon): *Violet Eclipse* | **Active.** ROM patch done and tested, PC and Wii U ports working, New 3DS port written but not compiled, decomp in progress |
| [MMBN1-Decomp/](MMBN1-Decomp/README.md) | Battle Network | Planned next. A bootstrap plan exists; no code, no tools |
| `MMBN2-Decomp/` | Battle Network 2 | Planned (folder not created yet) |
| `MMBN3-Decomp/` | Battle Network 3 (White / Blue) | Planned (folder not created yet) |
| `MMBN5-Decomp/` | Battle Network 5 (Team ProtoMan / Team Colonel) | Planned (folder not created yet) |
| `MMBN6-Decomp/` | Battle Network 6 (Cybeast Gregar / Falzar) | Planned (folder not created yet) |
| [MMBN-Principia-Trilogy/](MMBN-Principia-Trilogy/README.md) | BN1, BN2 and BN3 merged into one game on the BN3 engine, ports only | Design outline only; needs the BN1-3 decomps first |

## BN4: Violet Eclipse

A definitive edition of BN4 that unites Red Sun and Blue Moon: every Soul, chip and boss in one save ([plan](MMBN4-Decomp/plan/DEFINITIVE-EDITION.md)).

| Part | State | Where |
|---|---|---|
| **ROM patch** | Done: all packages pass in-game on both Red Sun and Blue Moon (Duo Soul, Free Tournament with the 12 Soul Navis, rematch ladders up to SP+, economy and drop changes, Mystery Data additions). Releases are built with `--public`, which leaves out Capcom chip art | [patch/README.md](MMBN4-Decomp/patch/README.md), [player README draft](MMBN4-Decomp/docs/player-readme-draft.md) |
| **PC port** | Done through PC-6: an emulation-layer port (embedded mGBA, SDL3), ARM64 and x64 builds | [ports/pc/](MMBN4-Decomp/ports/pc/README.md) |
| **Wii U port** | WU-1 to WU-6 verified in Cemu | [ports/wiiu/](MMBN4-Decomp/ports/wiiu/) |
| **New 3DS port** | Source written, not compiled yet (needs devkitPro) | [ports/3ds/](MMBN4-Decomp/ports/3ds/) |
| **Decomp** | **84.3% of the code region is owned as disassembly** (as of 2026-11-27); Red Sun and Blue Moon both rebuild byte-identical. **Only 63 C functions are matching decompilation.** The other C under `decomp/src/modern/` is portable re-implementation that isn't traced to ROM routines, so it counts as port scaffolding, not decomp progress | [decomp/README.md](MMBN4-Decomp/decomp/README.md), [decomp plan](MMBN4-Decomp/plan/decomp/README.md) |

Backlog: [MMBN4-Decomp/TODO.md](MMBN4-Decomp/TODO.md). Current handoff: [docs/handoff-violet-eclipse.md](MMBN4-Decomp/docs/handoff-violet-eclipse.md). Owner decisions waiting: [docs/pending/](MMBN4-Decomp/docs/pending/).

## Planned games

- **BN1:** [MMBN1-Decomp/](MMBN1-Decomp/README.md) holds the bootstrap plan (ROM identification, community resources, what BN4's toolchain reuses, first milestones).
- **BN2, BN3, BN5, BN6:** folders are created when their turn comes.
- **Principia Trilogy:** [MMBN-Principia-Trilogy/](MMBN-Principia-Trilogy/README.md) is the owner's idea of one continuous game from BN1-3. It depends on those three decomps, so it starts after them.

## Repository files

| File | Contents |
|---|---|
| [AGENTS.md](AGENTS.md) | Rules for the AI agents (Claude, Gemini, Codex): repo layout, who owns what, git rules, handoffs |
| `CLAUDE.md`, `GEMINI.md` | Per-agent entry points (both import `AGENTS.md`) |

Run a game's commands from inside its folder, for example `cd MMBN4-Decomp` then `python patch/build.py --check`.
