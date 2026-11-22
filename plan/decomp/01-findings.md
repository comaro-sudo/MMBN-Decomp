# 01 — Findings (2026-11-21)

## 1. BN4's code is hand-written assembly

The evidence:

| Observation | Where | What it means |
|---|---|---|
| A global "toolkit" pointer lives in **r10** across the whole program. `mov r7, r10; ldr r7, [r7,#0x40]` loads the game state | exe4rs `asm00.s` (every page); bn6f's IDA pseudo-C shows `v3; // r10` and `oToolkit_GameStatePtr` | No C compiler reserves r10 as a program-wide global. This is a hand-written convention. |
| About **2,500** `mov rX, r10` instructions in US Red Sun | Byte scan of `roms/bn4_rs_us.gba` | The convention is everywhere, not in a few routines. |
| Functions save **r1–r7** (`push {r1-r7,lr}`), not the AAPCS callee-saved set | exe4rs, e.g. `f500_8000558` | Non-standard calling contract. Callers rely on r1–r7 surviving. |
| Functions **return results in CPU flags**: the caller does `bl f` then `beq` | BN4-WoodMan-Fix source (`bl addr+96h` / `beq addr+60h`) | C can't express this. A reimplementation must reproduce the flag result explicitly. |
| bn6f (BN6, same engine family) has **no C in its build** | `ext/bn6f/Makefile`; its `docs/decomp/*.c` are IDA pseudo-C notes, not built | The most advanced project in the series stayed assembly. |
| Cyberworld Endless's assessment: "bn6f is a matching disassembly of Falzar, almost all of it assembly" | CE `docs/engine/emulation/why-an-emulator.md` | Same conclusion from an active project. |

**Consequence:** a *matching* decompilation (C that compiles byte-identical, as zeldaret/oot or pokeemerald do) is not achievable for the engine, because no compiler output can match hand-written assembly. The achievable path is:

- a **matching disassembly** (exact, rebuildable, nameable), and
- a **functional C reimplementation**, proven equal by testing behaviour, not bytes.

Possible exception: Nintendo's m4a ("Sappy") sound library was compiled C (agbcc) in many GBA games. Check whether BN4 uses it (step DC-1). If so, that part *can* match, and other GBA decomps (pokeemerald's m4a) already have it.

## 2. Existing work to reuse

| Project | State | Use for us |
|---|---|---|
| **dism-exe/exe4rs** | JP *Tournament Red Sun* disassembly, IDA-generated, about 326k lines of asm in 32 files interleaved with **16 MB of ROM chunks committed to git** (`bin/`). Mostly unnamed (`f500_…`). Last commit 2018-07-27 ("remove fake code, discover new code"). Clone: `ext/exe4rs` | Function **boundaries** and code/data split for JP RS, mapped to US RS by fingerprinting. **Not** a base: wrong region (we have no JP RS), ROM bytes in git, dormant |
| **dism-exe/bn6f** | BN6 Falzar, matching, active (2026-08), GNU as + Makefile, constants, structs, IDA-pseudo-C notes. Clone: `ext/bn6f` | **Names and structs** for shared engine code (toolkit offsets, game state, objects, battle), build layout, macros, `fdiff`/compare tools |
| **dism-exe/dism-exe-notes** | Research notes for the series | Read for BN4 specifics |
| **Cyberworld Endless** | ~700 BN4 RS symbols (MIT): RAM, tables, battle, tournament, objects, rewards. `docs/rom/bn4.md` includes "how much of bn6f's code it shares" | Seed labels. Its `xdiff` lockstep comparison tool is the model for our differential tests |
| **nettai** | `docs/engine/` battle flow, custom screen, chips (BN4–6) | Names and semantics for battle code |
| **TextPet** | Script and text formats for BN4 US | Text and script extraction in the disassembly |
| **mmbn-chip-tables** | BN4 chip table, verified byte-identical | First data table to "own" in the disassembly |

## 3. Size

- ROM: 8 MB. Thumb `push {…, lr}` heuristics count **about 12,500** function-like starts in dense regions. That is an overcount: data produces false positives. The real number is likely **5,000–8,000 functions**.
- For reference, exe4rs reached about 326k lines of disassembled text and still kept much of the ROM as binary chunks.

## 4. The user's existing decomp experience

The Silent Hill workspace (`C:\proyectos\Silent-Hill-Decomp`) already runs a matching decomp in WSL with:

- per-region checksum builds,
- rename maps (`old new` lines, one file per overlay),
- symbol sync scripts,
- a PC port built from the decomp.

Reuse the same habits:

- build in WSL, driven from PowerShell (Git Bash mangles `/mnt/c` paths),
- rename maps under `naming/`,
- "every commit still matches" verification.
