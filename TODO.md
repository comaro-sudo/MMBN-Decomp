# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-22, afternoon (America/Mexico_City). Claude updated it at the owner's request;
the decomp sections are Gemini's (see [`AGENTS.md`](AGENTS.md)) and keep Gemini's wording.

This is the short, execution-oriented backlog. The detailed plans remain in
[`plan/`](plan/README.md); the decomp notes and handoffs are in
[`decomp/notes/`](decomp/notes/).

## Coordination (read first)

- [x] Ownership split recorded in [`AGENTS.md`](AGENTS.md): **Gemini** owns `decomp/` and this file;
  **Claude** owns `plan/`, `docs/`, `patch/`, `ports/`, `README.md`. Stage by path, never `git add -A`.
- [x] Gemini and Claude now work **one at a time**: the owner runs Gemini only when Claude's usage limit is
  reached. Each one starts by reading the other's latest handoff (`decomp/notes/handoff-*.md`,
  [`docs/handoff-re.md`](docs/handoff-re.md)).
- [x] **Gemini: commit your decomp work.** Committed all `decomp/` work (asm, `asm/bm/`, configs,
  tools, notes, `names.txt`) plus this file by explicit path after verifying both RS and BM forced
  byte-match builds.
- [x] **Gemini: fix the shared build folder.** Replaced absolute symlink in `decomp/Makefile` with
  portable relative path `ln -sf ../../../roms/$(notdir $<) $@`. Works identically in WSL and Git Bash.
  Clean builds verify byte-matching across both RS and BM.
- [x] Claude's reverse-engineering lane, approved and recorded in `AGENTS.md` (`31e92a0`). Claude reads
  Gemini's asm without editing it, writes findings in [`docs/re/`](docs/re/save-and-flags.md), and proposes
  names in `docs/re/proposed-names.txt`.
- [ ] **Gemini: merge `docs/re/proposed-names.txt`** (about 70 names) into `symbols/names.txt`, then delete
  the merged lines. Also from Claude's notes:
  - `0x0800045E` is labelled `HBlankIntr`, but it looks like boot or soft-reset init;
  - six `code_04` "functions" are data disassembled as code (`sub_0805A58C`, `sub_0805A674`,
    `sub_0805A6AE`, `sub_08061836`, `sub_08066C0A`, `sub_08068B1C`);
  - see [`docs/re/completion-and-new-game-plus.md`](docs/re/completion-and-new-game-plus.md) §4.
- [ ] Owner, later: rename `C:\proyectos\MMBN4` to `C:\proyectos\MMBN-Decomp` when no agent is running;
  then fix the absolute `/mnt/c/proyectos/MMBN4` paths in the docs.

## Current verified state

- [x] US Red Sun and Blue Moon rebuild byte-for-byte from `decomp/`. (At 04:40 the BM build was broken
  by the build-folder symlink; see Coordination.)
- [x] Both versions have generated assembly views for every surveyed code
  region; opaque/unclassified ranges still use `incrom` from the player's ROM.
- [x] Blue Moon has its own correctly addressed layout and generated source
  view in `decomp/asm/bm/`.
- [x] Source-ownership reporting is available through
  `python decomp/tools/coverage.py rs` and `bm`.

Current source ownership (re-measured 04:40): RS **52.4%** (618,326 / 1,179,456 code bytes) and
BM **52.0%** (613,330 / 1,179,476 code bytes). This measures bytes represented
by source rather than `incrom`; it does not mean every represented function is
understood or named.

## Do next: disassembly (DC-2)

- [x] Preserve the byte-matching baseline before and after each change:

  ```powershell
  wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN4/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
  ```

- [x] Discover short two/three-entry Thumb callback tables: broadened with
  `verify_clean_thumb_func()` across all targets, finding 1,162 short tables
  (2,631 targets) in RS and 1,163 short tables (2,645 targets) in BM.
- [x] Discover clean gap function prologues: scan undecoded gaps for Thumb
  `push {..., lr}` (`0xB5xx`) and preceded `push {r...}` (`0xB4xx`) that
  decode cleanly to authentic returns (`pop {pc}`, `mov pc, lr`, `bx lr`).
- [x] Discover sparse pointer tables: scan runs of odd ROM pointers interleaved
  with null pointers (`0x00000000`), recovering 11 sparse opcode dispatch
  tables (18 targets) across both RS and BM.
- [x] Integrate canonical symbol names (`symbols/names.txt`) into `tools/disasm.py`,
  annotating function declarations, branch calls (`bl`), literals, and data
  references inline across both Red Sun and Blue Moon.
- [x] Audit indirect dispatch sites in `code_05`: verified that all 9 `bx rX`
  sites across RS and BM point to 100% clean Thumb target functions (70 entries
  total, exact +0xC delta in BM). Documented remaining ~135 KB in `code_05` as
  battle background scripts (`BN4_BATTLE_BGS`), animation structures, and
  scroll parameter tables.
- [x] Improve `decomp/tools/disasm.py` recursive descent for jump tables: added `jumptables`
  configuration and tracing in `disasm.py`, resolving both major switch dispatch sites in `code_10`
  (`0x08110DA4` / `0x08113A70` RS, `0x08110DB8` / `0x08113A84` BM) and reclaiming all 428 bytes
  of `incrom` as clean, structured, symbolic assembly.
- [ ] Prioritize remaining lower-coverage segments: `code_04` (RS 13.6%, BM 13.6%),
  `code_06` (28.3% / 28.2%), `code_09` (43.1%), and `code_10` (47.3% / 43.9%).
  Notice `code_07` (90.1%) and `code_08` (89.9%) have achieved near-complete code
  carving.
- [ ] Add verified function starts to `symbols/functions_rs.txt` and
  `symbols/functions_bm.txt`; regenerate only the affected segment with the
  WSL Capstone venv:

  ```powershell
  wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN4/decomp && ~/.venvs/mmbn4-decomp/bin/python tools/disasm.py rs code_04"
  ```

- [ ] Keep unresolved bytes as `incrom`; never commit ROM bytes, extracted
  assets, or bulk binary blobs.
- [ ] Reach DC-2 exit criteria: all code is assembly, code/data boundaries are
  documented, and no code remains hidden inside an `incrom` range.

## Then: names, RE, and the Blue Moon variant

- [x] Establish unified symbolic naming database in `symbols/names.txt` (753 symbols)
  cross-referencing Cyberworld Endless (CE), `bn6f`, and `exe4rs`.
- [x] Port 100% of addressable CE symbols (685 / 685) from Red Sun to Blue Moon,
  proving EWRAM/IWRAM address symmetry and documenting the 7 piecewise ROM code
  shift deltas (`+0x0`, `+0x4`, `+0x8`, `+0xC`, `+0x18`, `+0x14`, `+0xC`, `+0x14`).
- [ ] `docs/re/` notes (Claude's lane). Done:
  - [x] save format, verified on real RS and BM saves: relocation, XOR scrambling, checksum `+0x16`/`+0x22`;
  - [x] event-flag routines; Library flags `0x16C0 + id`; the playthrough counter and its 33 readers;
  - [x] Zenny and BugFrags with their anti-cheat shadows; battle-reward format and the AddZenny hook;
  - [x] Soul flags and pseudo-chips (and a debug script that grants the other version's Souls);
  - [x] completion stars; what New Game+ keeps.
  Still to do:
  - [ ] RE-04 tournament brackets and their RNG;
  - [ ] the story's Soul grant (needs a late-game save for a runtime watch);
  - [ ] RE-06 encounters and enemy records;
  - [ ] RE-07 Mystery Data and GMD tables;
  - [ ] RE-09 shop and trader tables;
  - [ ] the remaining save slots (pack, Library details, key items);
  - [ ] free ROM space.
- [x] Promote those findings into versioned symbol tables and generate labels
  for the patch project, mGBA, and the PC port: `tools/split.py` emits `build/<v>/symbols.inc`
  exporting all 753 canonical symbols (`.global` and `.set`/`.thumb_set`), populating the ELF
  symbol table with accurate Thumb functions, RAM structures, and data labels.
- [ ] Compare exact RS/BM function pairs from `symbols/map_rs_bm.txt`; merge
  only verified-identical functions into shared source with explicit
  version-specific blocks for differences.
- [ ] Complete DC-4: one maintainable RS/BM source tree that byte-matches both
  ROMs, with every version difference accounted for.

## Definitive ROM work (after the RE baseline)

- [ ] Inventory every Soul, chip, boss, acquisition source and version exclusive from both ROMs.
  Started: `python patch/tools/inventory.py`, with the conclusions in
  [`docs/completion/`](docs/completion/README.md).
  - [x] Chips: 389 records, identical in both ROMs. One save is missing 68 items: the other version's
    18 Navi chips, 5 Giga chips and 6 Souls, the 36 secret chips, and Z-Saver, Grand Prix Power and Duo.
  - [x] Souls: all 12, with flags, pseudo-chips and what the merge needs.
  - [ ] Bosses: a preliminary Navi/tier list exists; to be replaced with the ROM's enemy records (RE-06).
  - [ ] Replace the guide- and map-based sources (in `ref/`, local) with the ROM's own GMD, Mystery Data,
    shop and trader tables (RE-07/RE-09). Check the 4 chips with no listed source and the 6 partial-code
    chips.
- [ ] Decide the definitive-ROM story/version model and implement the
  cross-version content merge.
- [ ] Ship the Phase 1 ROM-patch quick wins: WoodMan timing fix, Zenny scaling,
  key-item shop pricing, ToyRobo reduction, battle rewards, and cutscene skip.
- [ ] Implement the progression work: all Souls in one run, deterministic
  brackets, Frequency Tuner, persistent Mystery Data, no missable GMD rewards,
  save protection, boss rematches, and in-game sources for every chip.
- [ ] Add automated completion, regression, hardware, and save-compatibility
  tests before release.

## Ports and functional C (after the shared symbols are useful)

- [x] PC-1: native ARM64 Windows app on an embedded mGBA 0.10.5 core + SDL3, PET window, headless
  `--test` mode ([`ports/pc/`](ports/pc/README.md), commit `f10889f`).
- [x] Emulators for testing copied into `tools/`: Cemu (Wii U, ARM64), Dolphin, PPSSPP, Vita3K.
- [x] PC test mode for RE: `--dump` (EWRAM), `--state-in` and `--state-out` chain scripted runs
  (`cf318b4`).
- [ ] A late-game test save or state (both versions), needed for runtime RE: Soul grants, tournaments,
  battles. Owner: if you have real saves, `test/saves/` is the tracked test-save folder (saves hold no ROM data; ask before committing personal ones).
- [ ] EL-0: refactor the PC code into `ports/common` (backend interface, endian-safe reads, `--bench`,
  idle hook) ([`plan/emulation-layer/`](plan/emulation-layer/README.md)).
- [ ] PC-2: add mGBA BKPT hooks, then PET state, touch Custom Screen, and Net
  Map support.
- [ ] Refactor the PC implementation into the shared director/backend layer.
- [ ] Benchmark and bring up the Wii U, New 3DS, and NDS routes in the planned
  order; NDS remains dependent on GBARunner licensing and spare-frame-time
  validation.
- [ ] Start functional C only for goal-driven subsystems, proven against the
  original with lockstep differential tests.

## Phase 1 patch (Claude)

- [x] Pipeline: `python patch/build.py` (hash-checked ROMs, armips, BPS round-trip); chip-table identity
  test passes for RS and BM; hello-world patch (Cannon 40 -> 99) builds.
- [ ] Owner: check the hello patch in mGBA (Library shows Cannon at 99, and it does 99 in battle).
- [ ] WP-A1: integrate Prof. 9's WoodMan fix. Ready: its mechanism is understood and its routine pointer is
  confirmed in both ROMs (`docs/re/save-and-flags.md` §3).
- [ ] WP-A2 Zenny ×1.75: the hook is the battle reward's AddZenny call (`0x08021704` RS). Changes must go
  through AddZenny, because of the Zenny anti-cheat shadow.
- [ ] Remaining Tier A: story-battle rewards (find the "no reward" rule), key-item prices (RE-09), ToyRobo,
  cutscene skip ([`plan/03`](plan/03-phase1-rom-overhaul.md)).
- [ ] Violet Eclipse patches unlocked by the RE so far (see `docs/completion/souls.md`):
  - a 12-entry Soul flag table;
  - 12-Soul checks in the completion star and in `checkDoubleSouls`;
  - a load routine that accepts both checksum constants;
  - a stash/restore pair for Mystery Data in New Game+ (WP-B3).

## Owner decisions needed

- [ ] [`plan/00` §4](plan/00-decisions-and-feasibility.md): Tier C content in 1.0? Zenny/prices fixed o
  in-game options? Omega Navis new or retuned? Confirm D1-D6, D8, D9 and D2 (develop on Red Sun first).
- [ ] [`plan/DEFINITIVE-EDITION.md` §7](plan/DEFINITIVE-EDITION.md): RS/BM version choice in the merged ROM,
  stronger-Navi requirements, Dark Chips without forced Dark Karma.
- [ ] [`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md): which hardware you own
  (Wii U/Aroma, New 3DS, Old 3DS, DS/DSi); reuse the GPL-3.0 Silent Hill GX2 GamePad code? Ask Gericom
  about the GBARunner licence?
- [ ] [`plan/decomp/README.md`](plan/decomp/README.md): confirm DD1-DD6 (most already followed in practice).
- [ ] [`docs/completion/chips.md`](docs/completion/chips.md) §3: how should one save get the 36 secret chips
  (Free Tournament drawing all twelve Navis, level by tier?) and Z-Saver, Grand Prix Power and Duo (Numbe
  Trader codes, BBS, a post-game reward)?

## Before committing

- [ ] Review the existing dirty worktree; it predates the latest decomp pass.
- [ ] Run `git diff --check`.
- [ ] Run both forced byte-match builds above.
- [ ] Run `python decomp/tools/coverage.py rs` and `bm`; record meaningful
  coverage changes in the relevant note.
