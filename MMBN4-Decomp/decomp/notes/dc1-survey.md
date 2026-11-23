# DC-1 survey (2026-11-22)

Tools: `tools/survey.py` (recursive descent, Capstone 5.0.6) and `tools/fingerprint.py` (masked byte fingerprints). Run in WSL with the venv `~/.venvs/mmbn4-decomp`.

## DC-0

`make compare` builds US Red Sun from a whole-ROM `.incbin` and matches (SHA-1 `a97e96a7…`). The pipeline is GNU as 2.42 / ld / objcopy (`binutils-arm-none-eabi`, Ubuntu 24.04 aarch64).

## Layout of the ROM (US Red Sun)

| Region | Contents | Evidence |
|---|---|---|
| `0x08000000`–`0x0811FFFF` (1.125 MB, 14%) | **All code**, with data tables interleaved | Every valid function start is below `0x08120000`. The m4a engine sits at the end (`0x08112848`) |
| `0x08120000`–`0x087FFFFF` (6.9 MB, 86%) | Data: graphics, maps, text, music, tables | Only 16 stray "code" seeds above, all invalid decodes (data that happens to look like code pointers) |

## Functions (RS)

- **About 7,170 candidate function starts.** 6,899 decode cleanly. Sources: `bl` targets 2,353, pointer tables 4,626 (662 tables of at least 4 Thumb pointers), CE symbols 158, literal pointers 29.
- The entry point alone reaches only 4 functions. The engine dispatches through registers: **1,309 `bx rX`** and **678 jump-table / PC writes** are not followed yet. That is DC-2's job (jump-table and dispatch resolution).
- Decoded so far: **27.6% of the code region** as instructions, 2.0% as literal pools. The other 70% is unreached code (behind dispatch), data tables inside the code region, and padding.
- 394 seeds decode as invalid. They cluster in `0x0805`–`0x0808xxxx` and `0x0810`–`0x0811xxxx`, which are data tables inside the code region, or false pointer-table seeds. DC-2 must classify these.
- 137 of the 159 Cyberworld Endless "code" symbols are reached as function starts. The rest are mid-function addresses (hook points, stores) or data mislabelled as code.
- 38 literal pointers into RAM code (IWRAM/EWRAM routines copied at runtime; bn6f's `iwram_code.s` pattern).

## Nintendo SDK libraries (compiled C → candidates for *matching* C)

| Library | Found | Notes |
|---|---|---|
| **m4a (MusicPlayer2000 / "Sappy")** | Yes: the standard `m4aSongNumStart` family signature at RS `0x08112848`, `…874`, `…8C0`, `…914`, `…948` (BM: +0x14) | The stock Nintendo version (bn6f's Capcom-modified form didn't match). pokeemerald's matching `m4a.c` is the reference: a likely **matching-C island** with agbcc |
| **SRAM library** | `SRAM_V113` strings at `0x0820A168`, `0x08211838` | agb_sram, also in pret projects |
| librfu (Wireless Adapter) | No | BN4 predates it (BN6 has it) |

## Red Sun ↔ Blue Moon

`symbols/map_rs_bm.txt`: **4,464 functions matched uniquely (79.2% of code bytes)**, all in **five constant-shift regions**:

| RS range | BM = RS + |
|---|---|
| `0x0800025C`–`0x08020CBE` | +0x0 |
| `0x08020D6C`–`0x0803501E` | +0x4 |
| `0x08035088`–`0x0804F202` | +0x8 |
| `0x0804F21C`–`0x08109E2E` | +0xC |
| `0x0810A1B4`–`0x08113CB2` | +0x14 |

Independent check: BN4-WoodMan-Fix's addresses (RS `0x080B1142`, BM `0x080B114E`) differ by +0xC, the delta of their region.

**Consequences:**

- DC-4 (the BM variant) should be small: a few `.if VERSION_BM` blocks at the four insertion points, plus differing data.
- Red Sun symbols, CE's included, can be ported to Blue Moon by region (decision D2: develop on Red Sun, port mechanically).
- 203 RS-only hashes remain. These are the functions that actually differ (version content) or that weren't sized consistently. Review them in DC-4.

## Next (rest of DC-1, then DC-2)

1. Map exe4rs (JP Rev 1) function boundaries onto US RS. Its labels carry addresses, so match by normalized instruction text.
2. Name m4a and SDK functions from pokeemerald's m4a symbols (byte patterns).
3. Segment config v1: code region vs data region; the m4a block; IWRAM-copied routines.
4. DC-2 `disasm.py`: jump-table and `bx rX` dispatch resolution, data-in-code classification, GNU-as output. Then carve the first segment (`0x08000000`–`0x0800025C`, crt0/header) and keep `make compare` matching.
