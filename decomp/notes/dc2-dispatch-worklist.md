# DC-2 dispatch worklist

Date: 2026-11-22
Status: short callback-table discovery is complete; indirect control flow is the main remaining source of undecoded code.

## Current survey blockers

| Blocker reported by `tools/survey.py` | RS | BM | Meaning |
|---|---:|---:|---|
| `jump_table_or_pc_write` | 11 | 6 | `mov`, `add`, or `ldr` writes PC; may be a switch table or computed branch. |
| `bx_reg` | 101 | 102 | A register branch that is not the recognized `mov lr, pc; bx rX` call pattern. |
| `bx_call` | 1,717 | 1,712 | Register-indirect call that returns to the following instruction; its callee remains unresolved. |
| `invalid` | 398 | 369 | A pointer seed reaches bytes that do not form the supported ARMv4T instruction class; often data in a code region. |

These are path counts, not necessarily distinct functions. The figures come from the refreshed survey after short callback-table discovery. Normal `mov pc, lr` returns are explicitly excluded: they end a path but are not unresolved dispatches.

## Order of investigation

1. Work a bounded gap in `code_05` (3.6% RS / 3.3% BM ownership). Identify one concrete PC-write or `bx` dispatch site and its table before adding a generic heuristic.
2. Repeat for `code_04` (11.7% / 11.4%) and `code_06` (19.9% / 19.9%).
3. Treat table bytes as data unless their entries and all targets can be verified. Keep opaque portions as `incrom`.
4. Once a pattern is verified in both versions, implement the narrowest resolver in `tools/survey.py` and `tools/disasm.py`, refresh symbols, and regenerate only the affected segment(s).

## Acceptance checklist for a new resolver

- The controlling instruction sequence is identified at a specific ROM address and the table bounds are proven.
- Every proposed target is inside the surveyed code region and decodes from a valid instruction boundary.
- The resolver cannot turn an unbounded data range into code merely because it contains pointer-like values.
- Both `make -B compare VERSION=rs` and `make -B compare VERSION=bm` pass.
- Record the pattern, evidence, addresses, and coverage effect in a dedicated note in this directory before broadening the heuristic.

## Non-goals for this stage

- Naming functions or translating assembly to C.
- Treating RAM-resident routine pointers as ROM function starts.
- Merging RS/BM source solely by address delta; use `symbols/map_rs_bm.txt` and exact matched bodies first.
