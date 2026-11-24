# Veneer Space Within `bl` Range (`code_00`, Round 12 Task 4, 2026-11-23)

All 12 candidates below sit in **`code_00`** (`0x08000A24`–`0x0800BA50`), so:
- **RS == BM** (exact same address and identical bytes in both US ROMs — no per-version veneer defines needed!).
- **Within ±64 KiB** of all four SP+/palette hook sites (`0x08003194`, `0x08006802`, `0x08013A5A`, `0x080107E8`) and `BattleExitCleanup` (`0x08006E20`), well inside Thumb `bl`'s ±4 MiB range.
- **Outside `--check` re-assembly** (far before the chip table `0x080197EC`–`0x0801DA80`).
- **Binary-verified unreferenced in BOTH RS and BM** (`scratch/r12/verify_veneers.py`):
  1. Zero occurrences of the symbol in all `decomp/asm/*.s` files (only its own `.global` and label).
  2. Preceding instruction is a clean function terminator (`pop {..., pc}`, `mov pc, lr`, `bx`, `b`, or `.word` pool) — no fall-through.
  3. Zero internal `_080XXXXX` labels referenced from outside the function.
  4. Zero decoded Thumb `bl` instructions anywhere in `0x08000000`–`0x08150000` in **either** RS or BM target any halfword inside `[addr, addr + size)`.
  5. Zero 4-byte pointers anywhere in the entire 8 MiB RS or BM ROM equal `a` or `a | 1` for any address `a` inside `[addr, addr + size)`.

## Recommended Candidates (35 × 8-Byte Word-Aligned Veneers)

| # | Symbol (`code_00`) | RS & BM Start | Total Size | Word-Aligned Start | Usable Size | 8-Byte Veneer Slots | Notes |
|---|---|---|---|---|---|---|---|
| 1 | `sub_0800A144` | `0x0800A144` | 54 B | **`0x0800A144`** | 54 B | **6** (`A144`, `A14C`, `A154`, `A15C`, `A164`, `A16C`) | **Best single block** — covers all 4 SP+/palette veneers + `VENEER_BATTLE_EXIT` (moved off Bass's `0x080C4788`) with 1 slot to spare! |
| 2 | `sub_08001784` | `0x08001784` | 36 B | **`0x08001784`** | 36 B | **4** (`1784`, `178C`, `1794`, `179C`) | Unused battle-coord helper |
| 3 | `sub_080032D4` | `0x080032D4` | 28 B | **`0x080032D4`** | 28 B | **3** (`32D4`, `32DC`, `32E4`) | Unused object helper right near `0x08003194` |
| 4 | `sub_080017DE` | `0x080017DE` | 26 B | **`0x080017E0`** | 24 B | **3** (`17E0`, `17E8`, `17F0`) | Unused bounding-box check |
| 5 | `sub_08002572` | `0x08002572` | 26 B | **`0x08002574`** | 24 B | **3** (`2574`, `257C`, `2584`) | Unused loop helper |
| 6 | `sub_080045C0` | `0x080045C0` | 24 B | **`0x080045C0`** | 24 B | **3** (`45C0`, `45C8`, `45D0`) | Unused init helper |
| 7 | `sub_08009DE6` | `0x08009DE6` | 26 B | **`0x08009DE8`** | 24 B | **3** (`9DE8`, `9DF0`, `9DF8`) | Unused panel helper |
| 8 | `sub_08000A24` | `0x08000A24` | 20 B | **`0x08000A24`** | 20 B | **2** (`0A24`, `0A2C`) | Unused DMA/timer getter |
| 9 | `sub_0800176E` | `0x0800176E` | 22 B | **`0x08001770`** | 20 B | **2** (`1770`, `1778`) | Unused flag helper |
| 10 | `sub_080017C8` | `0x080017C8` | 22 B | **`0x080017C8`** | 22 B | **2** (`17C8`, `17D0`) | Unused math helper |
| 11 | `sub_0800BA50` | `0x0800BA50` | 22 B | **`0x0800BA50`** | 22 B | **2** (`BA50`, `BA58`) | Unused player-request helper |
| 12 | `sub_0800306A` | `0x0800306A` | 18 B | **`0x0800306C`** | 16 B | **2** (`306C`, `3074`) | Unused sprite flag setter |

## Suggested Allocation (All Inside `sub_0800A144`, RS == BM)
- `0x0800A144` — `VENEER_BATTLE_EXIT` (moved off Bass's `0x080C4788` / `0x080C4794`, see `duo-story-grant-test.md`)
- `0x0800A14C` — `VENEER_DUO_PALETTE` (from `0x080107E8`)
- `0x0800A154` — `VENEER_SP_SPEED` (from `0x08003194`)
- `0x0800A15C` — `VENEER_SP_HP` (from `0x08006802`)
- `0x0800A164` — `VENEER_SP_ATK` (from `0x08013A5A`)
- `0x0800A16C` — 1 spare 8-byte slot (plus 29 more verified slots in candidates #2–#12).
