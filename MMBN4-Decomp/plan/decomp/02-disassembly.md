# 02 — Stage A: Matching Disassembly

**Goal:** `make compare` rebuilds `bn4_rs_us.gba` (SHA-1 `a97e96a7…`) byte for byte from assembly plus data, and later `bn4_bm_us.gba` (`5e017c80…`) from the same source with a version switch. Every commit keeps matching.

## A1. Start from "everything is data", then carve

Don't generate everything at once and debug a broken build. Grow it one piece at a time:

1. **DC-0 skeleton:** `rom.s` = `.incbin "baserom.gba"` for the whole ROM, linker script at `0x08000000`. Build, compare: match. This proves the pipeline.
2. Split the ROM into **segments** (a YAML config): header, crt0/start, code regions, data tables, compressed graphics, text archives, music, maps. Each is still `.incbin "baserom.gba", off, size`. Still matching.
3. **Carve code** segment by segment: replace an incbin range with generated assembly. Match → commit. Mismatch → fix the generator (usually a literal pool, a jump table, or ARM/Thumb mode), not the output by hand.
4. Repeat until all code is assembly. Data stays incbin until it's worth owning (A4).

ROM bytes are only ever read from `baserom.gba` (a symlink to `roms/bn4_rs_us.gba`) at build time. **Nothing from the ROM is committed:** no `bin/` chunks like exe4rs.

## A2. The disassembler (tool to write: `tools/decomp/disasm.py`)

It's a recursive-descent Thumb/ARM disassembler (Capstone) that emits GNU as syntax. Seeds and rules:

| Need | How |
|---|---|
| Entry points | Header entry, the IRQ handler, **known function lists**: CE symbols (RS), exe4rs boundaries mapped to US (A3), bn6f-matched functions (A3), every `bl` target found |
| ARM vs Thumb | Follow `bx` targets (bit 0), `.arm` IWRAM routines (copied to `0x03000000`; disassemble at their run address with `.org`/load-address handling as bn6f's `iwram_code.s` does) |
| Literal pools | `ldr rX, [pc, #n]` targets become `.word` data with labels. Pointer-looking words into the ROM range become symbolic labels |
| Jump tables | BN idioms: `lsl/add/ldr pc`, `mov pc, rX` after a table load, and **object/state tables** (e.g. `BN4_T1_TYPES`: function pointer arrays). Detect pointer arrays to Thumb code and label them |
| Toolkit accesses | Render `ldr r7, [r10-derived, #0x40]` with bn6f's `oToolkit_*` names when the offset is known |
| Unknown regions | Stay `.incbin` and are listed in a coverage report |

Output layout mirrors bn6f: `asm/<segment>.s`, `include/` macros (`thumb_func_start`, `thumb_local_start`, …), `constants/`, `data/`.

## A3. Function fingerprinting (tool: `tools/decomp/fingerprint.py`)

This matches functions across **BN6 (bn6f names) ↔ JP RS (exe4rs boundaries) ↔ US RS ↔ US BM**.

- Fingerprint = the function's halfwords with **relocations masked**: `bl` offsets, literal-pool contents, and PC-relative immediates. Hash it, and also keep a fuzzy n-gram signature for near matches.
- Exact matches carry names directly. Fuzzy matches go to a review list.
- Expected: very high RS↔BM overlap (same engine, version-specific content), substantial BN4↔BN6 overlap (CE's `bn4.md` quantifies the shared code; read it first).
- The same tool **ports Phase 1 offsets and CE's RS symbols to Blue Moon** (D2).

## A4. Data and assets ("owning" data)

Order (each converts an incbin into readable source while staying byte-matching):

1. **Chip table:** already proven with `mmbn-chip-tables` (identity test passes).
2. Encounter tables, shop tables, NaviCust parts, reward tables (Phase 1 needs these).
3. **Text and scripts** via TextPet: extract → insert must be byte-identical. If TextPet's insertion isn't byte-stable, keep the binary and store the extracted text only for reading.
4. **Graphics:** LZ77-compressed tiles and palettes. Extract to PNG at build time *from the user's ROM* (never committed), recompress with a compressor that matches the original byte for byte (bn6f's `lz_assets.mk` / `gbagfx` show how). If recompression can't match, keep the incbin plus extract-only.
5. Music (m4a/Sappy song data), maps: last, and only if a goal needs them.

## A5. Blue Moon as a variant

- After RS matches: fingerprint BM against RS. Identical functions share source. Differing ones go behind `.if VERSION_BM` blocks, and data lives in per-version files.
- `make compare VERSION=rs` and `make compare VERSION=bm` must both match. Same pattern as the Silent Hill USA/EUR/JAP builds.

## A6. Exit criteria (gate G-A)

- Both US ROMs build byte-identical from one tree.
- 100% of code is assembly (no code left inside incbins). Coverage report shows code/data/unknown percentages.
- Phase 1 patches (armips, applied on the built ROM) still apply and pass their checks.
