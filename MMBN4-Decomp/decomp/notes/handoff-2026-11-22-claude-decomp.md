# Handoff: Claude's decomp session, 2026-11-22 evening (Claude → Gemini)

## Goal & Current Status

- **Goal:** the owner asked Claude to "also decomp" (recorded in `AGENTS.md`), working directly in `decomp/` while Gemini isn't running.
- **Verified and committed** (working tree clean after these commits; both `make -B compare` runs OK, 0 warnings):
  - `7a6e03d`: **DC-2.**
    - A BL-density scan of every `incrom` range over 64 bytes found only 14 BLs into known functions in the whole code region, so the remaining `incrom` is data.
    - The 14 sat in two dead 86-byte blocks after an unconditional `b` in `sub_08037BE4` and its twin (RS `0x08037C58` and `0x08037D30`, BM +8). Nothing branches to or points at them.
    - They're decoded through `entries` in both `segments_*.toml` (`code_03`).
  - `231ac1c`: **DC-3, 126 text-script command handlers named** (`ScriptCmd_*`, `ScriptGroup_*`).
    - New tool: `tools/script_cmds.py [rs|bm] [--names]`. It walks the engine's main table (RS `0x0804E900`, opcodes `E5`–`FF`) and each group's sub-table, and pairs them with TextPet's command names (`tools/textpet/plugins/mmbn4.ini`, local).
    - All 98 handlers that appear in `map_rs_bm.txt` agree with the BM addresses the tool derived.
    - Only `code_03`/`code_04` annotations changed.

## Key Decisions & Constraints

- **Shared handlers** (one routine for a family of commands) get the first command's name; the description lists all of them.
- **`E8`** has no TextPet name, so it's `ScriptCmd_E8`.
- **Gotcha:** a symbol with characters outside `[A-Za-z0-9_]` in `names.txt` breaks the build. The failure shows up in `disasm.py`'s self-check as "build failed outside known instructions" for every segment. I hit this with `ScriptCmd_?` and fixed it.
- **Gotcha:** regenerating all 11 segments takes over 10 minutes. Run it in the background, and never interrupt it halfway, which leaves mixed files. If that happens, restore `decomp/asm/` from git and rerun.

## Modified & Relevant Files

- `decomp/config/segments_rs.toml`, `segments_bm.toml`: `code_03` `entries`.
- `decomp/symbols/names.txt`: a new section of 126 rows at the end, "Text-script command handlers".
- `decomp/tools/script_cmds.py`: new.
- `decomp/asm/code_03.s`, `code_04.s` and their `bm/` copies: regenerated.
- `AGENTS.md`: Claude may work in `decomp/`.

## Open Issues / Blockers

- None in the build.
- The other segments weren't regenerated, because nothing calls the handlers with `bl`. A full regeneration later will only add annotations if anything references them.

## Immediate Next Steps

1. (Either agent) DC-4: start sharing identical RS/BM functions. `map_rs_bm.txt` has 7,772 pairs.
2. (Claude, RE) the story Soul grant, the Free Tournament (`ScriptCmd_callFreeTournament` = RS `0x08050500` leads to it), shops (`ScriptCmd_startShop` = RS `0x0805025C` leads to RE-09), and battles (`ScriptCmd_startRandomBattle` = RS `0x08050228` leads to RE-06).
