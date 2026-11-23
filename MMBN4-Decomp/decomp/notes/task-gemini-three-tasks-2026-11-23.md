# Three tasks for Gemini (assigned by the owner, written by Claude, 2026-11-23)

Excellent work on RE-06 and the DC-4 polish (92 → 63 blocks). Claude's next session works only in `docs/` (the completion inventory from your RE-06) and `patch/` (the Violet Eclipse patches). So **don't edit `docs/` or `patch/`**. Everything else in `decomp/` and `TODO.md` is yours. Do the tasks in order, commit each one separately by explicit path, and keep both compares green after any asm change:

```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp/decomp && make -B compare VERSION=rs && make -B compare VERSION=bm"
```

## Task 1: carve the pointer-heavy `incrom` tables into structured data (DC-2/DC-4)

About 5,000 data pointers and about 30,000 code pointers still sit inside `incrom` tables (your `dc4-version-differences.md` §4.4–4.5), so they can't become shared symbols.

- Start with the tables you named: `BN4_MYSTERY_DATA_CW`, `BN4_ENEMY_STATS`, `BN4_REWARD_ROWS`, `BN4_STORY_BATTLES`. Then take the next largest pointer tables in `code_04`–`code_06`.
- Emit them as `.word` / `.hword` / `.byte` with `dat_`/`sub_` symbols, through `disasm.py` (a config-driven table list per segment, like `jumptables`), not by hand-editing asm. Regenerate per the workflow in `decomp/README.md`: RS, then BM, then `merge_bm.py`.
- Keep the renaming one-to-one. The `code_04` cutscene-script pointers that cross version-specific insertions (the 16 "non-trivial" ones) stay as version blocks.
- **Done when:** the chosen tables are structured in both versions, both compares are OK, and `coverage.py` and the version-block count are re-measured and recorded in a note.

## Task 2: RE-04 remainder, tournaments (read-only note)

`docs/re/tournaments.md` (Claude's, don't edit it) has the pools, the Soul Navi pairs and the entrant fill. Still open:

- **the Free Tournament** (`sub_080448C0`, reached from `ScriptCmd_callFreeTournament`, RS `0x08050500`): how its entrants and prizes are chosen;
- **the bracket draw**: where the order and pairings come from, and the RNG calls involved;
- **who writes the traded-Navi list at `0x02006F40`**: Red Sun already fields Blue Moon's Navis when they're in that list.

Write **`decomp/notes/re04-tournaments-remainder.md`**: routines with RS and BM addresses, the tables, and proposed names (merge them into `names.txt` yourself, then regenerate).

## Task 3: name the remaining save slots (read-only analysis, then names)

`docs/re/save-and-flags.md` §1 lists the toolkit slots. These are still unnamed:

| Slot | Offset in the save block | Size |
|---|---|---|
| `+0x50` | `+0x05B4` | `0xF04` |
| `+0x54` | `+0x14B8` | `0x194` |
| `+0x5C`–`+0x70` | `+0x2410`–`+0x251C` | small |
| `+0x7C` | `+0x2F34` | `0x704` |
| `+0x80` | `+0x3638` | `0x194` |

What's already known: `+0x58` is the shop stock (`docs/re/shops.md`), `+0x74` is the Mystery Data state (your RE-07), and `+0x78` holds Navi stats.

For each slot, find its readers and writers (`mov rX, sl; ldr rY, [rX, #slot]`) and say what it holds: the pack (chips owned × codes), Library details, key items, NaviCust layout, mail, BBS, the Number Trader. Use the 13 community saves (`ref/saves/`, local, through `python patch/tools/savetool.py`, read-only) to confirm what each one holds.

Write **`decomp/notes/save-slots.md`**, plus proposed names merged into `names.txt`.

## For every task

- Never commit ROM bytes, extracted assets or guide data.
- Stage by explicit path.
- Don't push.
- Write a handoff in `decomp/notes/` at the end.
