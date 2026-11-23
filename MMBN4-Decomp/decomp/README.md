# MMBN4 disassembly

A byte-matching, buildable disassembly of *Mega Man Battle Network 4* (USA): Red Sun (B4WE) and Blue Moon (B4BE). The plan is in [../plan/decomp/](../plan/decomp/README.md).

**No ROM data lives in this tree.** Everything that isn't disassembled yet is `.incbin`'d at build time from your own dump in `../roms/`.

## Build (WSL)

Run from PowerShell (Git Bash rewrites `/mnt/c` paths):

```powershell
wsl.exe -d Ubuntu-24.04 -- make -C /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp compare
wsl.exe -d Ubuntu-24.04 -- make -C /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp compare VERSION=bm
```

Needs `binutils-arm-none-eabi`, `make`, `sha1sum`. Tools under `tools/` use the venv `~/.venvs/mmbn4-decomp` (Capstone 5.0.6).

## Status

| Milestone | State |
|---|---|
| DC-0 skeleton | ✅ RS matches from a whole-ROM incbin (2026-11-22) |
| DC-1 survey | In progress: [notes/dc1-survey.md](notes/dc1-survey.md). About 7.2k functions, code region `0x08000000`–`0x0811FFFF`, stock m4a present, RS↔BM = 5 shift regions |
| DC-2 code carved | In progress: all 11 code regions have generated assembly views for RS and BM. Unclassified ranges remain `incrom`; run `python3 tools/coverage.py rs` or `bm` for the exact source coverage. |
| DC-4 Blue Moon variant | ✅ Both ROMs build byte-identical from **one shared tree** (`asm/`, 2026-11-22). Symbols are named after RS addresses. The ~1,050 version differences are `.if VERSION_BM` blocks, written by `tools/merge_bm.py`. Next: reduce the blocks that come from mid-segment shifts ([notes/task-gemini-dc4-version-differences.md](notes/task-gemini-dc4-version-differences.md)). |

## Layout

| Path | Contents |
|---|---|
| `build/<version>/rom.s` | Generated top-level layout for the selected version (`rom.s` mirrors the default RS layout) |
| `ld_script.ld` | ROM at `0x08000000` |
| `config/` | Segment split per version |
| `symbols/` | Symbol tables (name, address, kind, source) |
| `asm/`, `data/`, `include/` | Shared RS/BM code (`.if VERSION_BM` blocks where they differ), owned data, macros |

## Regenerating a segment (DC-4 workflow)

`disasm.py` writes one version at a time: `asm/<seg>.s` for RS and `asm/bm/<seg>.s` for BM. Fold them back into one shared file:

```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc 'cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && P=~/.venvs/mmbn4-decomp/bin/python && $P tools/disasm.py rs code_05 && $P tools/disasm.py bm code_05 && $P tools/merge_bm.py code_05 && make -B compare VERSION=rs && make -B compare VERSION=bm'
```

Always regenerate RS first. `merge_bm.py` refuses to run on a file that's already shared. Until the merge runs, `split.py` assembles the BM view under RS names (`build/bm/norm/`), so `disasm.py bm`'s self-check still works.
| `tools/` | Disassembler, fingerprinting, coverage, symbol import |
| `notes/` | Survey results, DC-2 discovery notes, and continuation worklists |
