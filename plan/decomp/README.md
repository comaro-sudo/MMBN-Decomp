# Decompilation Plan — MMBN4

Plans the decompilation of Mega Man Battle Network 4 (US Red Sun first, Blue Moon as a variant). It replaces the option list in [../05-phase3-decompilation.md](../05-phase3-decompilation.md) with a concrete plan. Written 2026-11-21.

| File | Contents |
|---|---|
| [01-findings.md](01-findings.md) | What we know: hand-written assembly, the r10 "toolkit", existing projects, sizes |
| [02-disassembly.md](02-disassembly.md) | Stage A: a byte-matching, buildable disassembly of US Red Sun, then Blue Moon |
| [03-functional-c.md](03-functional-c.md) | Stage C: C reimplementation verified by differential testing, run *inside* the PC port one function at a time |
| [04-tooling-and-workflow.md](04-tooling-and-workflow.md) | Toolchain (WSL), tools to write, naming workflow, verification, repo rules |
| [05-milestones-and-risks.md](05-milestones-and-risks.md) | Milestones DC-0 to DC-7, gates, effort, risks, first actions |

## The plan in five lines

1. **A matching C decompilation (pokeemerald style) is not possible for most of BN4.** The engine is hand-written Thumb assembly (a global "toolkit" pointer in r10, results returned in CPU flags, non-standard register saving), so no compiler reproduces it. Only small parts (the m4a sound library, if present) are compiled C.
2. **Stage A: a byte-matching disassembly**, the way bn6f did for BN6. It is the source of truth for names, structures and boundaries, and it rebuilds both US ROMs byte for byte. No ROM bytes are ever committed.
3. **Stage B: documentation.** Name functions, structs, RAM and flags. This directly feeds the Phase 1 overhaul and the PC port's hooks.
4. **Stage C: functional C.** Rewrite subsystems in readable C that *behaves* the same, verified by running the original and the C side by side on the same inputs and comparing memory. The PC port runs C functions in place of their assembly (via hooks), so the game is always playable while coverage grows.
5. **Only decompile what the goals need.** The port features that need deep changes (battle logic, field camera for widescreen, link/SIO for netplay) come first. Features that hooks already reach (PET UI, touch Custom Screen) don't need decompiled code.

## Where it fits

```
Phase 1 ROM patches (armips) ──────────────► keep working on built ROMs, later moved into the disassembly
PC port (option E, embedded mGBA) ─────────► hosts Stage C: C functions replace asm one by one
Stage A disassembly ──► Stage B names ──► Stage C functional C ──► (optional) no-interpreter native builds
```

## Decisions to confirm

| # | Decision | Recommendation |
|---|---|---|
| DD1 | Base ROM | **US Red Sun (B4WE)**. We own it, and Cyberworld Endless's symbol map is for it. US Blue Moon is a build variant. JP stays reference-only. |
| DD2 | Fork `dism-exe/exe4rs` or start fresh | **Start fresh**, using exe4rs (JP, IDA-generated, ROM chunks in git, dormant since 2018) and bn6f only as references for boundaries, macros and naming. |
| DD3 | Matching C or functional C | **Functional C** with differential testing (reasons in [01](01-findings.md)). |
| DD4 | Assembler for the disassembly | **GNU as / ld (arm-none-eabi)**, as bn6f does. It lets C link in later and reuses bn6f's macros. Phase 1 stays armips. |
| DD5 | Repo visibility | Private while solo. Public would need the same "no ROM bytes" rules plus a licence choice. |
| DD6 | Stage C target | Run C inside the PC port first. A no-interpreter native build (no ARM emulation) is a later, optional gate. |
