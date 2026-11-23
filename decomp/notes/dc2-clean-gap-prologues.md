# DC-2: Clean Gap Prologue Discovery & Coverage Leap to 52%

Date: 2026-11-22
Scope: US Red Sun (`rs`) and Blue Moon (`bm`)
Result: Byte-matching verified across both versions. Overall source ownership surpassed 50%:
- **RS: 52.0%** (613,584 / 1,179,456 code bytes, up from 37.2% / 438,604 bytes)
- **BM: 51.9%** (611,820 / 1,179,476 code bytes, up from 36.7% / 433,196 bytes)

## Root cause analysis of undecoded code gaps

Investigation into the lowest-coverage segments (`code_05` at 3.6%, `code_04` at 11.7%, `code_06` at 19.9%) revealed why thousands of valid functions remained inside `incrom` blocks:

1. **Undecoded Gap Skip Bug:** In `tools/survey.py`, `prologue_pass()` attempted to discover unreferenced functions by looking for `push {..., lr}` (`0xB5xx`) after `0x0000` padding at the start of an undecoded gap. However, after checking the very first non-zero offset `p`, it skipped the *entire rest of the gap* (`while a < end and a not in covered: a += 2`). If a gap began with data (such as a data table, jump table, or sprite parameters), `p` was not a function prologue, causing the entire gap (sometimes tens of kilobytes) to be skipped.
2. **False Literal Seeds:** Literal-pool loads (`ldr rX, [pc, #...]`) loading odd values pointing into data or table bytes were indiscriminately seeded as code pointers (`literal_ptr`), truncating real data tables and creating phantom functions.

## Implementation: Verified Clean Gap Prologues

`tools/survey.py` was enhanced with `verify_clean_thumb_func()` and `clean_gap_prologue_pass()`:

1. **Validation criteria:**
   - Word starts with Thumb `push {..., lr}` (`0xB5xx`) on a halfword-aligned boundary.
   - Address is within the valid code region (`0x0800025C` - `0x08120000`) and not already decoded.
   - Every instruction must decode cleanly as valid ARMv4T Thumb.
   - Execution path must terminate at an authentic function return: `pop {..., pc}`, `mov pc, lr`, or `bx lr`.
   - Any invalid instruction, out-of-range branch, or runaway decode (>500 instructions) rejects the candidate.

2. **Survey results:**
   - RS: **1,061 clean gap prologues** seeded, discovering **1,094 functions** and **65,256 new decoded code bytes**. Total functions: 11,510 (11,235 clean).
   - BM: **960 clean gap prologues** seeded, discovering **991 functions** and **59,552 new decoded code bytes**. Total functions: 11,456 (11,185 clean).

## Segment Ownership Comparison

| Segment | Total Bytes | RS Before | RS After | BM Before | BM After |
|---|---:|---:|---:|---:|---:|
| `crt0` | 364 | 97.8% | 97.8% | 97.8% | 97.8% |
| `intr_table` | 48 | 0.0% | 0.0% | 0.0% | 0.0% |
| `code_00` | 64,974 | 76.8% | **84.4%** | 75.8% | **83.5%** |
| `code_01` | 68,930 | 48.3% | **53.7%** | 47.1% | **53.0%** |
| `code_02` | 82,716 | 55.9% | **72.8%** | 55.8% | **72.8%** |
| `code_03` | 106,900 | 69.7% | **75.1%** | 69.7% | **75.1%** |
| `code_04` | 139,208 | 11.7% | **13.6%** | 11.4% | **13.6%** |
| `code_05` | 144,756 | 3.6% | **6.6%** | 3.3% | **6.5%** |
| `code_06` | 113,286 | 19.9% | **28.3%** | 19.9% | **28.2%** |
| `code_07` | 130,778 | 51.8% | **90.0%** | 50.9% | **89.9%** |
| `code_08` | 130,906 | 38.1% | **89.7%** | 36.8% | **89.6%** |
| `code_09` | 106,914 | 41.1% | **43.1%** | 41.2% | **43.1%** |
| `code_10` | 89,676 | 32.1% | **43.6%** | 31.8% | **43.4%** |
| **TOTAL** | **1,179,456** | **37.2%** | **52.0%** | **36.7%** | **51.9%** |

## Verification

Both full builds forced and verified byte-for-byte:
```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
```
Both emitted `build/rs/bn4_rs.gba: OK` and `build/bm/bn4_bm.gba: OK`.
