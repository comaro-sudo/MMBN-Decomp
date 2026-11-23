# MMBN-Decomp

A collection of decompilation projects for the *Mega Man Battle Network* series. Each game has its own self-contained folder: its decomp, plans, docs, ROM patch and ports. Nothing from the ROMs is in this repository. Everything is built from the player's own dumps, which go in each game's gitignored `roms/` folder.

| Folder | Game | Status |
|---|---|---|
| [MMBN4-Decomp/](MMBN4-Decomp/README.md) | Battle Network 4 (Red Sun / Blue Moon): *Violet Eclipse* definitive edition | Active |
| `MMBN1-Decomp/` | Battle Network | Planned |
| `MMBN2-Decomp/` | Battle Network 2 | Planned |
| `MMBN3-Decomp/` | Battle Network 3 (White / Blue) | Planned |
| `MMBN5-Decomp/` | Battle Network 5 (Team ProtoMan / Team Colonel) | Planned |
| `MMBN6-Decomp/` | Battle Network 6 (Cybeast Gregar / Falzar) | Planned |

Work order: BN4 first, then BN1, 2, 3, 5 and 6.

| File | Contents |
|---|---|
| [AGENTS.md](AGENTS.md) | Rules for the two AI agents (Claude, Gemini): repo layout, who owns what, git rules, handoffs |
| `CLAUDE.md`, `GEMINI.md` | Per-agent entry points (both import `AGENTS.md`) |

Run a game's commands from inside its folder, for example `cd MMBN4-Decomp` then `python patch/build.py --check`.
