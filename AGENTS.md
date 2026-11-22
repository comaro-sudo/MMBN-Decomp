# Rules for AI agents working in this repo

Two agents work on this project in parallel: **Claude** (Claude Code) and **Gemini** (Gemini CLI). Both read this file: Claude through `CLAUDE.md`, Gemini through `GEMINI.md`. The project owner decides; this file records those decisions.

## 1. Who owns what (set 2026-11-22)

Edit only your own areas. If work needs a change in the other agent's area, **stop and tell the owner** instead of editing.

| Area | Owner | Notes |
|---|---|---|
| `decomp/` (tools, asm, config, symbols, notes) | **Gemini** | Includes the decomp sections of `TODO.md` |
| `TODO.md` | **Gemini** | Claude may only add items under "Ports and functional C" and "Definitive ROM work" when asked |
| `plan/` (incl. `plan/decomp/`, `plan/emulation-layer/`) | **Claude** | Gemini proposes plan changes in its notes or to the owner |
| `docs/`, root `README.md` | **Claude** | |
| `patch/` (Phase 1 ROM patch) | **Claude** | |
| `ports/` (PC, future Wii U/3DS/NDS) | **Claude** | |
| `.gitignore`, `.gitattributes`, `AGENTS.md`, `CLAUDE.md`, `GEMINI.md` | **Owner** | Either agent may edit when the owner asks |
| `roms/`, `tools/`, `ext/`, `build/` | (gitignored) | Never commit anything from them |

To swap or change ownership, the owner edits this table.

## 2. Working rules (both agents)

- **Check before editing.** Run `git status --short` first. Read the latest handoff of the area you touch (`decomp/notes/handoff-*.md`, `HANDOFF.md`, or `docs/handoff-*.md`).
- **Stage by explicit path**, never `git add -A` or `git add .`. Commit only your own files. Leave the other agent's uncommitted work alone, and never reset, revert or discard it.
- **Never commit ROM data**: no ROMs, no extracted graphics/audio/text, no bulk binary blobs. Opaque bytes stay `.incbin`/`incrom` from the player's ROM at build time.
- **Decomp changes must keep both ROMs byte-identical.** Run from PowerShell (Git Bash mangles `/mnt/c` paths):

  ```powershell
  wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN4/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
  ```

  Expected: `build/rs/bn4_rs.gba: OK` and `build/bm/bn4_bm.gba: OK`.
- **Patch changes:** `python patch/build.py --check` must pass.
- Line endings are LF (`.gitattributes`). WSL `make` and GNU `as` break on CRLF.
- The project goal is in `plan/DEFINITIVE-EDITION.md`: *Mega Man Battle Network 4: Violet Eclipse*, every Soul, chip and boss in one save, on every port.

## 3. Handoff protocol (both agents)

Write a handoff when your context is nearly full (Claude: the status-line hook warns at 95%), before the owner switches agents, or when asked.

1. Stop non-handoff edits.
2. Write a self-contained file in your own area: Gemini → `decomp/notes/handoff-<date>-<time>.md`; Claude → `docs/handoff-<topic>.md` (or `HANDOFF.md` at the root for cross-area work).
3. Say what is **verified** versus **tentative**.
4. Use these sections:
   - **Goal & Current Status**: the task; what is done (commit ids if committed).
   - **Key Decisions & Constraints**: owner decisions, invariants, gotchas.
   - **Modified & Relevant Files**: exact paths, and what is uncommitted.
   - **Open Issues / Blockers**: failing checks, edge cases, open questions.
   - **Immediate Next Steps**: ordered and actionable, enough to resume without the conversation.
5. Tell the owner the handoff is ready.

## 4. Pending owner decisions

- Rename the repo folder `C:\proyectos\MMBN4` → `C:\proyectos\MMBN-Decomp`: **later**, when no agent is running. Absolute paths in the docs (e.g. `/mnt/c/proyectos/MMBN4`) must be updated then.
