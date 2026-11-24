# Rules for AI agents working in this repo

**Claude** (Claude Code) is the main agent on this project. **Gemini** (Gemini CLI) and **Codex** (OpenAI Codex CLI) are used from time to time. All read this file: Claude through `CLAUDE.md`, Gemini through `GEMINI.md`, Codex directly (it reads `AGENTS.md` by default). The project owner decides; this file records those decisions.

## 0. Repo layout: a collection of MMBN decomps (set 2026-11-22)

This repo (`C:\proyectos\MMBN-Decomp`) collects decomps of the *Mega Man Battle Network* series. Each game has its own self-contained folder at the root:

| Folder | Game | Status |
|---|---|---|
| `MMBN4-Decomp/` | Battle Network 4 (Red Sun / Blue Moon) | Active: the first game |
| `MMBN1-Decomp/`, `MMBN2-Decomp/`, `MMBN3-Decomp/`, `MMBN5-Decomp/`, `MMBN6-Decomp/` | BN1, BN2, BN3, BN5, BN6 | Planned, in that order after BN4 |

- The repo root holds only what all the games share: this file, `CLAUDE.md`, `GEMINI.md`, `.gitignore`, `.gitattributes` and the collection `README.md`.
- A game folder holds everything about that game, including its gitignored `roms/`, `tools/`, `ext/` and `build/`. Paths inside a game folder are relative to that folder, so its docs, scripts and Makefiles work unchanged no matter where the repo sits.
- **Run a game's commands from its folder** (e.g. `cd MMBN4-Decomp` before `python patch/build.py`). In the rest of this file, `decomp/`, `docs/`, `TODO.md` etc. mean the paths inside `MMBN4-Decomp/`.

## 1. Who owns what (set 2026-11-22)

Edit only your own areas. If work needs a change in the other agent's area, **stop and tell the owner** instead of editing.

| Area (inside `MMBN4-Decomp/`) | Owner | Notes |
|---|---|---|
| `decomp/` (tools, asm, config, symbols, notes) | **Gemini** | Includes the decomp sections of `TODO.md` |
| `TODO.md` | **Gemini** | Claude may only add items under "Ports and functional C" and "Definitive ROM work" when asked |
| `plan/` (incl. `plan/decomp/`, `plan/emulation-layer/`) | **Claude** | Gemini proposes plan changes in its notes or to the owner |
| `docs/`, `README.md` | **Claude** | |
| `patch/` (Phase 1 ROM patch) | **Claude** | |
| `ports/` (PC, future Wii U/3DS/NDS) | **Claude** | |
| `roms/`, `tools/`, `ext/`, `build/` | (gitignored) | Never commit anything from them |

| Area (repo root) | Owner | Notes |
|---|---|---|
| `README.md` (collection index) | **Claude** | |
| `.gitignore`, `.gitattributes`, `AGENTS.md`, `CLAUDE.md`, `GEMINI.md` | **Owner** | Either agent may edit when the owner asks |

**Other game folders (`MMBN1-Decomp/`, `MMBN2-Decomp/`, ... ; owner, 2026-11-22):** **Claude** owns everything in them by default. Gemini and Codex work there only on a task the owner gives them, and only in the files that task covers. They record what they did in a handoff in that game's `docs/` folder, so Claude sees it on its next turn.

To swap or change ownership, the owner edits these tables.

**Temporary (owner, 2026-11-23, until Friday 2026-11-27):** Gemini handles most of the BN4 tasks and **may also edit `MMBN4-Decomp/patch/` and `MMBN4-Decomp/docs/`**. It follows Claude's handoff `MMBN4-Decomp/decomp/notes/handoff-claude-to-gemini-2026-11-23.md`, keeps `patch/README.md` current, and records its work in a `decomp/notes/handoff-*.md`. After Friday the tables above apply again.

**Claude's reverse-engineering lane (approved by the owner, 2026-11-22).** At first Claude only read `decomp/` (see the update below).
- Findings go in `docs/re/`.
- Proposed symbol names go in `docs/re/proposed-names.txt`, in the `names.txt` format. Gemini merges them into `decomp/symbols/names.txt` and deletes the merged lines.
- Claude builds only in a private directory outside `decomp/build/`.

**Update (owner, 2026-11-22 evening):** Claude may also work directly in `decomp/` ("you can also decomp"). This is safe because the agents run one at a time, never concurrently.
- Claude follows the decomp rules: both ROMs byte-identical (`make -B compare` for `rs` and `bm`), no ROM bytes committed.
- Claude records its decomp work in a `decomp/notes/` handoff, so Gemini sees it on its next turn.

## 2. Working rules (both agents)

- **Check before editing.** Run `git status --short` first. Read the latest handoff of the area you touch (`decomp/notes/handoff-*.md`, `HANDOFF.md`, or `docs/handoff-*.md`).
- **Stage by explicit path**, never `git add -A` or `git add .`. Commit only your own files. Leave the other agent's uncommitted work alone, and never reset, revert or discard it.
- **Never commit ROM data**: no ROMs, no extracted graphics/audio/text, no bulk binary blobs. Opaque bytes stay `.incbin`/`incrom` from the player's ROM at build time.
- **Decomp changes must keep both ROMs byte-identical.** Run from PowerShell (Git Bash mangles `/mnt/c` paths):

  ```powershell
  wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
  ```

  Expected: `build/rs/bn4_rs.gba: OK` and `build/bm/bn4_bm.gba: OK`.
- **Patch changes:** `python patch/build.py --check` (run from `MMBN4-Decomp/`) must pass.
- Line endings are LF (`.gitattributes`). WSL `make` and GNU `as` break on CRLF.
- The BN4 goal is in `MMBN4-Decomp/plan/DEFINITIVE-EDITION.md`: *Mega Man Battle Network 4: Violet Eclipse*, every Soul, chip and boss in one save, on every port.

## 3. Handoff protocol (both agents)

Write a handoff when your context is nearly full (Claude: the status-line hook warns at 95%), before the owner switches agents, or when asked.

1. Stop non-handoff edits.
2. Write a self-contained file in your own area: Gemini → `decomp/notes/handoff-<date>-<time>.md`; Claude → `docs/handoff-<topic>.md` (or `HANDOFF.md` at the repo root for cross-game or cross-area work).
3. Say what is **verified** versus **tentative**.
4. Use these sections:
   - **Goal & Current Status**: the task; what is done (commit ids if committed).
   - **Key Decisions & Constraints**: owner decisions, invariants, gotchas.
   - **Modified & Relevant Files**: exact paths, and what is uncommitted.
   - **Open Issues / Blockers**: failing checks, edge cases, open questions.
   - **Immediate Next Steps**: ordered and actionable, enough to resume without the conversation.
5. Tell the owner the handoff is ready.

## 4. Owner decisions

- ~~Rename the repo folder `C:\proyectos\MMBN4` → `C:\proyectos\MMBN-Decomp`~~: **done 2026-11-22**.
- ~~Turn the repo into a collection, BN4 work in `MMBN4-Decomp/`~~: **done 2026-11-22**. Order of games: BN4, then BN1, 2, 3, 5, 6.
- ~~Who owns the next game folders~~: **decided 2026-11-22**. Claude owns them by default, and Gemini and Codex work only on tasks the owner assigns (see §1). The BN4 split in §1 is unchanged.
- ~~Shared tools~~: **decided 2026-11-22**. When work on the second game starts, the third-party tools and emulators (`tools/`: armips, TextPet, flips, mGBA, SDL3, Cemu, Dolphin, PPSSPP, Vita3K) and their shared sources (`ext/`: armips, mgba, cemu) move from `MMBN4-Decomp/` to a shared `tools/` and `ext/` at the repo root. Clones that only serve one game stay in that game's `ext/`. When the move happens, point BN4's paths at the root: `MMBN4-Decomp/patch/build.py`, `ports/pc/build_pc.py`, `ports/pc/CMakeLists.txt` and `docs/reference/toolchain.md`, then re-run the BN4 checks. Until then everything stays in `MMBN4-Decomp/`. The `.gitignore` already ignores both locations.
