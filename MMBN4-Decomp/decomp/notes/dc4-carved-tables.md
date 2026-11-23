# Task 1: Structured Data-Table Carving (DC-2 / DC-4, 2026-11-22)

Added config-driven `data_tables` support to `decomp/tools/disasm.py` (`fmt = "word"`, `"hword"`, `"record"` with field specs `w`/`u`/`h`/`b`) and configured 8 high-value tables in `decomp/config/segments_rs.toml` and `decomp/config/segments_bm.toml`.

## Carved Tables

| Segment | Table | RS Range | BM Range | Format | Bytes | Pointers Symbolized | Added Version Blocks |
|---|---|---|---|---|---|---|---|
| `code_01` | `BN4_ENEMY_STATS` | `0x08011888`–`0x08012470` | `0x08011888`–`0x08012470` | `record` (`hbbbbbbu`, 254 × 12 B) | 3,048 | — | 0 |
| `code_04` | Sprite/object table | `0x080523C0`–`0x080538F8` | `0x080523CC`–`0x08053904` | `word` | 5,432 | 723 | **0** |
| `code_05` | Battle/script table | `0x080843F0`–`0x080853F8` | `0x080843FC`–`0x08085404` | `word` | 4,104 | 370 | +31 (genuine RS/BM entries) |
| `code_06` | Pointer table A | `0x080A78C8`–`0x080A7C28` | `0x080A78D4`–`0x080A7C34` | `word` | 864 | 96 | **0** |
| `code_06` | Pointer table B | `0x080A7C60`–`0x080A7EC0` | `0x080A7C6C`–`0x080A7ECC` | `word` | 608 | 96 | **0** |
| `code_06` | `BN4_MYSTERY_DATA` (RW+CW) | `0x080A9110`–`0x080ADD8C` | `0x080A911C`–`0x080ADD98` | `word` | 19,580 | 414 | **0** |
| `code_09` | `BN4_REWARD_ROWS` | `0x080F5F50`–`0x080F8A70` | `0x080F5F5C`–`0x080F8A7C` | `hword` (276 × 20) | 11,040 | — | +17 (the 64 RS/BM virus drop rows) |
| `code_09` | `BN4_STORY_BATTLES` | `0x080FA900`–`0x080FB47C` | `0x080FA90C`–`0x080FB488` | `record` (`bbbbbbbbw`, 245 × 12 B) | 2,940 | 245 | **0** |

## Coverage & Version-Block Impact (Verified Byte-Identical)

- **RS source ownership:** `598,984 / 1,179,456` (`50.8%`) → **`646,592 / 1,179,456` (`54.8%`)** (**+47,608 bytes**, **+4.0%**).
  - `code_01`: `50.1%` → `54.5%`
  - `code_04`: `18.3%` → `27.8%`
  - `code_05`: `4.6%` → `7.4%`
  - `code_06`: `28.2%` → `46.8%`
  - `code_09`: `42.4%` → `55.5%`
- **BM source ownership:** `599,008 / 1,179,476` (`50.8%`) → **`646,624 / 1,179,476` (`54.8%`)** (**+47,616 bytes**, **+4.0%**).
- **Externs (newly symbolized shared pointers):** `479` → **`2,007`** (RS) / `480` → **`2,008`** (BM) (**+1,528** shared pointer symbols).
- **Version blocks (`.if VERSION_*`):** `63` → **`111`** (+48 genuine data differences exposed in plain `.word`/`.hword` inside `code_05` and `BN4_REWARD_ROWS`; **0** version blocks added across `BN4_ENEMY_STATS`, `code_04` 723-pointer table, `code_06` Mystery Data + 192 pointers, and `BN4_STORY_BATTLES`).
- **Build verification:** `make -B compare VERSION=rs` → `build/rs/bn4_rs.gba: OK`; `make -B compare VERSION=bm` → `build/bm/bn4_bm.gba: OK`.
