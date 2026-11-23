# DC-2: conservative short callback-table discovery

Date: 2026-11-22
Scope: US Red Sun (`rs`) and Blue Moon (`bm`)

## Why this pass exists

The original pointer-table pass accepted only runs of four or more adjacent odd ROM pointers. That is a good broad heuristic, but it misses the common two- and three-entry callback/state tables used by the engine. A single pointer-shaped word is not sufficient evidence: code-region data and script data contain many false positives.

`tools/survey.py` now has `short_prologue_pointer_tables()`. It accepts a short run only when every entry meets all of these conditions:

1. The word is an odd pointer into the surveyed ROM code region.
2. The run is exactly two or three words long; longer runs remain the existing broad pointer-table case.
3. Each target starts with Thumb `push {..., lr}` (`0xB5xx`).
4. Each target is currently neither decoded code nor a PC-relative literal pool word.

The last two conditions make these function-entry seeds rather than a generic data-pointer classifier. Targets that are already decoded do not improve code coverage and are intentionally not relabelled by this pass.

## Survey result

| Version | Short tables | New starts | Total starts | Decoded code bytes |
|---|---:|---:|---:|---:|
| RS | 838 | 1,981 | 10,416 | 502,688 (42.6% of code region) |
| BM | 894 | 2,100 | 10,465 | 506,440 (42.9% of code region) |

The totals are survey results, not a claim that every resulting routine is named or understood. The disassembler still retains undecoded data and ambiguous bytes as `incrom`.

## Regenerated output and ownership

The pass was applied to `code_04`, `code_05`, `code_06`, and `code_09` in both source views. The significant ownership change was in `code_09`:

| Segment | RS before | RS after | BM before | BM after |
|---|---:|---:|---:|---:|
| `code_09` | 24.9% | 41.1% | 24.2% | 41.2% |

Overall owned bytes are now 438,604 / 1,179,456 (37.2%) for RS and 433,196 / 1,179,476 (36.7%) for BM.

## Reproducing the pass

Run the survey before regenerating an affected segment:

```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && ~/.venvs/mmbn4-decomp/bin/python tools/survey.py rs && ~/.venvs/mmbn4-decomp/bin/python tools/survey.py bm"
```

Then regenerate only the segment under investigation, for example:

```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && ~/.venvs/mmbn4-decomp/bin/python tools/disasm.py rs code_09"
```

The generator retries with annotated raw instruction encodings when GNU `as` cannot reproduce Capstone's syntax. Finish every batch with both forced comparisons:

```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
```

## Deliberate limits

- Do not reduce this to a one-entry pointer heuristic.
- Do not admit targets that merely decode plausibly; require the prologue and uncovered-byte checks above.
- This does not solve Thumb PC-writing jump tables, `bx rX` dispatch, or code copied to RAM. Those remain separate DC-2 tasks; see [dc2-dispatch-worklist.md](dc2-dispatch-worklist.md).
