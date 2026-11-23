# 02 — Reverse-Engineering Plan

Every Phase 1 feature depends on finding a specific routine, table or flag in the ROM. This file lists those targets so the RE work can be scheduled before the feature work.

## 1. Method

1. **Harvest existing knowledge first.** Never re-discover what is already documented. Known sources, in order of value:
   - `ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/symbols/bn4-red-sun-us.sym` (~700 symbols, our exact RS ROM, MIT) and `docs/rom/` (bn4.md, souls.md, battle.md, game-state.md, scripts-and-shops.md…).
   - `ext/nettai/docs/engine/` (battle flow, custom screen, chips, JP differences).
   - bn6f symbols (shared engine).
   - TextPet `mmbn4{rs,bm}-us.tpi` indexes.
   - chip tables (verified byte-identical).
   - The symbols load into mGBA with `load-symbols FILE`.
2. **Dynamic tracing in mGBA.** Use memory watchpoints on known RAM values (Zenny, HP, playthrough counter, flags) to find the code that reads and writes them.
3. **Static analysis in Ghidra.** Label each function found, then follow its callers and callees.
4. **Cross-version mapping.** Once an offset is found in one version (Red Sun first, if D2's revision is accepted), find the matching offset in the other by byte-pattern search. Record both.
5. **Document as you go.** One file per system in `docs/re/`, with the RAM address, the ROM offset (BM/RS), the function signature, a short pseudo-C sketch and confidence (confirmed/likely/guess).

## 2. RE targets (ordered by which features they unblock)

| ID | Target | Unblocks | Priority | Likely starting point |
|---|---|---|---|---|
| RE-01 | Save-data layout (story flags, playthrough counter, Soul flags, inventory, Zenny) | Nearly everything | P0 | Diff save files before and after known events |
| RE-02 | Event flag set/check routines and the flag ID list | Persistence, narrative, Souls | P0 | Script opcodes via TextPet; bn6f flag routines |
| RE-03 | Playthrough counter (NG+ count) and **every place it is read** | Frequency Tuner, Soul rework, NG+ | P0 | Watchpoint on the counter in RAM |
| RE-04 | Tournament system: bracket generator, opponent pool, RNG call, per-playthrough rules | Deterministic brackets, Soul rework | P0 | Breakpoint on the RNG during the bracket reveal |
| RE-05 | Double Soul grant logic (which event gives which Soul) | Soul rework | P0 | Flags set after the tournament finals |
| RE-06 | Encounter tables (per area, per tier) and the selection routine | Frequency Tuner, GMD scaling | P1 | Random-battle trigger code |
| RE-07 | Mystery Data system (BMD/PMD one-shot flags, GMD loot tables, story-reset logic) | Persistent MD | P1 | Flags set on pickup |
| RE-08 | Battle reward calculation (Zenny, BugFrag, chip drops, the "no reward" flag on scripted battles) | Economy | P1 | Code after the battle-end results screen |
| RE-09 | Shop tables and price data | Economy | P1 | Search for known prices in the ROM |
| RE-10 | Cutscene and script interpreter (skip hooks, ToyRobo sequence counter) | Skips | P1 | TextPet script dumps |
| RE-11 | Duo AI state machine and hitbox data | Duo refinements | P2 | Boss AI tables (compare with bn6f boss AI structure) |
| RE-12 | Dark Chip / Karma tracking and the max-HP penalty | Karma redemption | P2 | Watchpoint on the HP penalty |
| RE-13 | Waiting Room / link transfer buffer and its overlap with Soul flags | Save protection | P2 | Link-trade code (nettai SIO notes) |
| RE-14 | WoodMan scenario softlock | Bug fix | P0 (likely solved already by BN4-WoodMan-Fix) | Read the existing patch |
| RE-15 | Job BBS request system | Moving generic Navis to the BBS | P2 | Request list data and its handlers |
| RE-16 | Map, NPC and warp data formats | New mini-dungeons (Tier C) | P3 | bn6f map format, Cyberworld Endless |
| RE-17 | Free ROM space and pointer tables for expansion | All new code and content | P0 | Scan for 0xFF/0x00 padding |
| RE-18 | NaviCust program table (if the Frequency Tuner is a program) | Frequency Tuner | P1 | NaviCust part list |

## 3. Output format for each RE note (`docs/re/<system>.md`)

```
# <System>
Status: confirmed | partial | guess
RAM:  0x0200xxxx  (BM)  0x0200xxxx (RS)
ROM:  0x08xxxxxx  (BM)  0x08xxxxxx (RS)
Functions: name @ addr — signature — purpose
Data structures: field table
Notes / open questions
```

## 4. Exit criteria for the RE phase (M1)

- RE-01 through RE-05, RE-14 and RE-17 are confirmed in Blue Moon, and at least mapped in Red Sun.
- A shared `symbols_bm.asm` and `symbols_rs.asm` are usable from armips.
