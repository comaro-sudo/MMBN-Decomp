# MMBN4 disassembly

A byte-matching, buildable disassembly of *Mega Man Battle Network 4: Red Sun* (USA, B4WE), with Blue Moon (B4BE) to follow as a variant. The plan is in [../plan/decomp/](../plan/decomp/README.md).

**No ROM data lives in this tree.** Everything that isn't disassembled yet is `.incbin`'d at build time from your own dump in `../roms/`.

## Build (WSL)

Run from PowerShell (Git Bash rewrites `/mnt/c` paths):

```powershell
wsl.exe -d Ubuntu-24.04 -- make -C /mnt/c/proyectos/MMBN4/decomp compare
```

Needs `binutils-arm-none-eabi`, `make`, `sha1sum`. Tools under `tools/` use the venv `~/.venvs/mmbn4-decomp` (Capstone 5.0.6).

## Status

| Milestone | State |
|---|---|
| DC-0 skeleton | ✅ RS matches from a whole-ROM incbin (2026-11-22) |
| DC-1 survey | In progress: [notes/dc1-survey.md](notes/dc1-survey.md). About 7.2k functions, code region `0x08000000`–`0x0811FFFF`, stock m4a present, RS↔BM = 5 shift regions |
| DC-2 code carved | Next |

## Layout

| Path | Contents |
|---|---|
| `rom.s` | Top-level layout: segments in ROM order |
| `ld_script.ld` | ROM at `0x08000000` |
| `config/` | Segment split per version |
| `symbols/` | Symbol tables (name, address, kind, source) |
| `asm/`, `data/`, `include/` | Carved code, owned data, macros |
| `tools/` | Disassembler, fingerprinting, coverage, symbol import |
| `notes/` | Survey results (DC-1) |
