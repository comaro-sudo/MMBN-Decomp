# 03 — Stage C: Functional C, Run Inside the PC Port

**Goal:** readable C for the subsystems the project needs, proven to behave exactly like the original. It runs in the real game from day one, because the PC port (embedded mGBA) calls the C function instead of the assembly.

## C1. How a C function replaces an assembly function ("strangler" model)

```
game code (ARM interpreter) ── bl Foo ──► BKPT hook at Foo's first instruction
                                             │
                                             ▼
                               host: hle_Foo(ctx)  ← C, reads/writes GBA memory
                                             │
                               result written back into r0..r7 + CPSR flags
                                             ▼
                               HOOK_RETURN (bx lr) ─► caller continues in the interpreter
```

- Uses the hook mechanism planned as PC-2 ([../10-pc-port.md](../10-pc-port.md)): a `BKPT` written over the first instruction in the core's ROM copy, as Cyberworld Endless does (`docs/engine/emulation/hooks.md`).
- **Memory:** C gets typed pointers straight into mGBA's EWRAM (`0x02000000`, 256 KB) and IWRAM (`0x03000000`, 32 KB) blocks (`getMemoryBlock`). ROM is read-only. IO, VRAM and OAM writes go through the core's bus functions so the PPU sees them.
- **The toolkit:** `struct Toolkit* tk = gba_ptr(ctx->r[10])`. Field names come from bn6f's `oToolkit_*`.
- **Toggle per function:** a table `{ address, hle_fn, enabled }` loaded from a config file. Any function can switch back to its assembly instantly. This is how bugs get bisected.

## C2. The register contract (the hard part)

Hand-written functions don't follow the C ABI. Each one needs a **contract**, inferred by a tool (`tools/decomp/contract.py`) from the disassembly:

| Item | How it's inferred |
|---|---|
| Inputs | Registers read before written in the function (r0–r7, r10 implicit) |
| Outputs | Registers that **callers** read after the `bl` before writing them |
| Preserved | From the push/pop set (often `r1-r7`). C must leave these untouched in `ctx` |
| **Flags out** | Callers that branch on flags right after the `bl` (`beq`/`bne` with no `cmp`). The C function returns e.g. `ctx->Z` explicitly |
| Side effects | Memory written (traced in DC-5 tests) |

The generated contract becomes the C prototype comment and the hook glue. When a caller relies on something the contract misses, the differential tests below catch it.

## C3. Order: bottom-up first

- **Leaf functions first** (they call nothing). The C never needs to call back into assembly.
- Then functions whose callees are all already in C.
- Calling *into* still-assembly code from C ("nested emulation": run the interpreter until it returns to a sentinel address) is a **spike in DC-6**, not a day-one requirement.

## C4. Differential testing (the proof)

The equivalent of "matching" for functional C. Modelled on Cyberworld Endless's `xdiff`:

1. **Recorded sessions:** a savestate plus an input log (the PC port's test mode already does scripted inputs and headless runs).
2. Run twice, headless and unthrottled:
   - **A:** all assembly.
   - **B:** with the C functions enabled.
3. Compare **EWRAM + IWRAM + VRAM + OAM + palette every frame** (or at every hooked return, for precise blame). First divergence → report the frame, address and the last C function that ran.
4. A **corpus** of sessions per subsystem (battles of each Navi, menus, field walks, tournament brackets), grown over time. Must pass before a function's `enabled` default flips to true.
5. **Mutation check** (also from CE): deliberately break a C function and confirm the corpus catches it. This shows the tests have teeth.

RNG: both runs start from the same state, and the C uses the game's RNG (`BN4_RNG_PRIMARY`) the same way, so runs stay in lockstep.

## C5. What to decompile, in order (driven by goals)

| Priority | Subsystem | Why (which goal needs real code changes) |
|---|---|---|
| 1 | **Utility leaves** (memcpy-likes, math, RNG, flag get/set, LZ77) | Builds the tooling and contract machinery on easy cases |
| 2 | **Event flags and save data** | Phase 1 (Souls, Mystery Data persistence) plus PET features |
| 3 | **Battle core**: object runner, chip use, damage, Custom Screen logic | Duo rework, chip balance, touch Custom Screen driving the real logic |
| 4 | **Enemy and Navi AI** (Duo first) | WP-B8 and boss changes beyond data tweaks |
| 5 | **Field/overworld camera and scrolling** | 16:9 widescreen (3DS 400 px, Wii U, PC) |
| 6 | **Link/SIO** | Netplay with rollback (PC/Wii U/3DS) instead of emulated link cable |
| 7 | **Rendering setup** (BG/OBJ layer use per scene) | 3DS per-layer stereo depth |
| — | Menus, text engine, map scripts | Only when a feature needs them. Hooks and data edits cover most |

## C6. Beyond: no interpreter (optional gate G-C)

When C covers everything that runs per frame, the ARM interpreter could be dropped. Two things remain:

- **The GBA PPU/APU:** still emulated in software (mGBA's renderer) unless replaced per platform (NDS 2D engines, 3DS GPU quads). That is a port task, not a decomp task.
- **Licensing:** a native build ships Capcom's logic as C. Options E/C builds that load the user's ROM avoid shipping code. Decide at the gate.

Realistically this gate may never be needed: the hybrid already gives readable, moddable C where it matters.
