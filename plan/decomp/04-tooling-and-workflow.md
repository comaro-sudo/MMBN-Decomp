# 04 — Tooling & Workflow

## 1. Repository layout (inside this repo, under `decomp/`)

```
decomp/
├─ Makefile                 # make [VERSION=rs|bm] compare
├─ ld_script.ld
├─ baserom_rs.gba -> ../roms/bn4_rs_us.gba   (symlinks, gitignored)
├─ baserom_bm.gba -> ../roms/bn4_bm_us.gba
├─ config/segments_rs.yaml  # segment split (code/data/gfx/text/...)
├─ config/segments_bm.yaml
├─ asm/                     # generated, then hand-edited for names/comments
├─ data/                    # owned data tables (.s or generated from .json/.csv)
├─ include/  constants/     # macros, toolkit/struct offsets (seeded from bn6f)
├─ symbols/                 # symbols_rs.txt, symbols_bm.txt (name, addr, size, kind)
├─ naming/                  # rename maps "old new", one file per subsystem
├─ src/                     # Stage C functional C (shared with ports/pc via a library)
└─ tools/                   # disasm.py, fingerprint.py, contract.py, coverage.py, symsync.py
```

`.gitignore` additions: `decomp/baserom*.gba`, `decomp/build/`, and extracted assets (`decomp/assets/`).

## 2. Toolchain (WSL Ubuntu-24.04, aarch64, already present)

| Tool | Install | Use |
|---|---|---|
| `binutils-arm-none-eabi`, `gcc-arm-none-eabi` | `apt` (aarch64 packages exist) | Assemble and link the disassembly. GCC for Stage C test builds of shared code |
| Python 3 + `capstone` | `pip` in a venv | Disassembler, fingerprinting, contracts |
| `make`, `sha1sum` | present | Build and compare |
| Ghidra + GhidraGBA (Windows, ARM64 JDK 17) | M1 task | Interactive analysis. Import the symbol files both ways (script) |
| mGBA (Windows) + the PC port's `--test` mode | present | Dynamic tracing, differential tests |

As in the Silent Hill workspace: **drive WSL from PowerShell** (`wsl.exe -d Ubuntu-24.04 -- bash …`), not from Git Bash, which rewrites `/mnt/c/...` paths.

Note: devkitARM is *not* needed for the disassembly. Ubuntu's `arm-none-eabi` binutils are enough (bn6f only needs `as`/`ld`/`objcopy`).

## 3. Tools to write (in order)

| Tool | Purpose | Milestone |
|---|---|---|
| `coverage.py` | % of ROM that is code-as-asm / owned data / incbin / unknown, per segment. The progress number | DC-0 |
| `fingerprint.py` | Masked function hashes. Map names RS↔BM↔JP(exe4rs)↔BN6(bn6f). Port CE's RS symbols to BM | DC-1 |
| `disasm.py` | Recursive Thumb/ARM disassembler → GNU as, with jump tables, literal pools and IWRAM code | DC-2 |
| `symsync.py` | One symbol source → `.s` labels, mGBA `.sym`, Ghidra import, the PC port's `game.c` tables | DC-3 |
| `rename.py` | Apply `naming/*.map` across asm and symbols safely (whole-word, then rebuild and compare) | DC-3 |
| `contract.py` | Per-function register/flags contract for Stage C | DC-5 |
| `xdiff` (in ports/pc) | Lockstep A/B runs and memory diff per frame | DC-5 |

## 4. Daily loop

```
pick a segment or subsystem
 → generate / edit / rename
 → make compare VERSION=rs && make compare VERSION=bm   (must match)
 → coverage.py
 → commit (one logical change, matching)
```

Same habits as the Silent Hill decomp:

- rename maps are reviewed text files,
- a symbol sync after renames,
- a full rebuild of all versions before committing.

## 5. Sharing with the rest of the project

- `symbols/symbols_rs.txt` is the **single source of truth** for addresses. The Phase 1 `patch/src/{rs,bm}/version.asm` labels, the PC port's `game.c` and the mGBA `.sym` files are generated from it.
- Once the disassembly matches, Phase 1 changes can move from "armips patch on the ROM" to "edit the source". Keep armips patches until G-A passes, then decide (that's a Phase 1 decision; both work).

## 6. Rules

- Never commit ROM bytes (no `bin/` chunks, no extracted graphics, audio or text dumps). Everything is extracted from `baserom` at build time.
- Every commit matches both versions (after DC-4 adds BM).
- Generated files are clearly marked. Hand edits go into the asm *after* generation is stable, or into the generator's override files, never both.
