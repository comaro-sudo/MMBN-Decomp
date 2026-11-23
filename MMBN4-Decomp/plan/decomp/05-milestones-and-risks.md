# 05 — Milestones, Gates, Risks

Effort estimates are for one person part-time with AI assistance. Treat them as order-of-magnitude.

## Milestones

| ID | Milestone | Done when | Effort |
|---|---|---|---|
| **DC-0** | Skeleton | `decomp/` builds RS from a whole-ROM incbin and `make compare` matches. `coverage.py` reports 0% code. WSL toolchain installed | 1–2 days |
| **DC-1** | Survey and mapping | m4a library present or not (compiled C?). `fingerprint.py` maps exe4rs (JP) and bn6f functions onto US RS, with match %. CE symbols ported to BM. Segment config v1 | 1–2 weeks |
| **DC-2** | Code carved | `disasm.py` emits all code segments as asm and RS still matches. Unknown regions listed | 1–2 months |
| **DC-3** | Names v1 | Symbols for the RE targets in [../02-reverse-engineering.md](../02-reverse-engineering.md) (save layout, flags, tournament, encounters, rewards, Souls) named in asm. Symbol sync feeds Phase 1 and the PC port | 1–2 months (overlaps DC-2) |
| **DC-4** | Blue Moon variant | BM builds byte-identical from the same tree behind `VERSION=bm` | 2–4 weeks |
| **— Gate G-A —** | | Both match, code 100% asm. Decide: continue to Stage C now, or pause and use the names for Phase 1 | |
| **DC-5** | Hybrid C infrastructure | PC-2 hooks (BKPT) working. `contract.py`. HLE table with per-function toggle. xdiff lockstep tests with a first session corpus. First 20 leaf functions in C, all passing | 1–2 months |
| **DC-6** | First subsystem in C | Event flags + save data in C, corpus green. Spike: C calling back into asm (nested emulation) | 2–3 months |
| **DC-7+** | Subsystems by priority | Battle core → AI (Duo) → field camera → link → rendering setup ([03 §C5](03-functional-c.md)) | months each |
| **— Gate G-C —** | | Optional: no-interpreter native builds (see [03 §C6](03-functional-c.md)) | |

Rough total to G-A: **3–5 months.** Stage C is open-ended and goal-driven.

## Gates and how they connect to the rest of the plan

- G-A replaces "G3" in [../09-milestones.md](../09-milestones.md) for the native ports: the ports keep running on the embedded core (option E) and gain C subsystems over time. NDS native (no emulator, 67 MHz ARM9) is the one target that needs most of Stage C plus native PPU mapping, so it stays last.
- Phase 1 gets better names from DC-3 even if Stage C never starts.

## Risks

| ID | Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|---|
| DR1 | The disassembler mis-classifies data as code (or the reverse) and the build stops matching | High early | Low | Matching every commit catches it immediately. Carve one segment at a time. Keep unknowns as incbin |
| DR2 | IWRAM/ARM code copied at runtime is hard to place | Med | Med | bn6f's `iwram_code.s` pattern. Dynamic trace in mGBA to find copy sources |
| DR3 | TextPet or LZ77 re-insertion isn't byte-stable | Med | Low | Keep incbin plus extract-only for reading. Owning assets isn't required for G-A |
| DR4 | Register contracts miss a caller's hidden dependency | High | Med | Differential tests per frame. Per-function toggle. Infer from *all* call sites |
| DR5 | Nested emulation (C → asm calls) is too complex in mGBA | Med | Med | Bottom-up order avoids needing it early. Fallback: convert callees first |
| DR6 | Scope creep: decompiling for its own sake | High | High (solo, R11) | C5's goal-driven priority list. Every Stage C item names the feature it unblocks |
| DR7 | Legal exposure if published | Low (private) | High | No ROM bytes, build from the user's ROM, decide publicity at DD5 |
| DR8 | Effort competes with Phase 1 and PC port milestones | High | Med | DC-0..DC-3 *are* Phase 1 RE work (same targets). Schedule them as the M1 RE baseline |

## First actions

1. Confirm DD1–DD6 ([README](README.md)).
2. WSL: `sudo apt install binutils-arm-none-eabi gcc-arm-none-eabi`, plus a Python venv with `capstone`.
3. DC-0: `decomp/Makefile` + linker script + whole-ROM incbin → `make compare` matches RS.
4. Read CE's `docs/rom/bn4.md` "how much of bn6f's code it shares" and `dism-exe-notes` for BN4.
5. DC-1: check for m4a, run the first fingerprint pass (US RS vs exe4rs JP function list, vs bn6f).
