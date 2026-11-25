# RE Note: `code_04`–`code_05` Map-Init & Cutscene Script Bytecode

Observed while inspecting the largest `incrom` regions in `code_04` (`0x080544DC`, 10,384B; `0x08056F74`, 8,624B) through `code_05`.

## Entry Point Chain

1. `code_02` `0x0802AFD0..0x0802B1BC` (carved in `a57a907`) is the map-group pointer table.
2. `code_02` `0x0802B1BC..0x0802BCCC` (354 × 8B `uw` records, `0xFFFFFFFF`-terminated) maps each `(0xFFFFFF00 | map_group)` entry to a map-init script header in `code_04`–`code_05` (e.g. `0x08054530` for group `0x00`, `0x08056F98` for group `0x01`).

## Script Structure (e.g. `0x080544DC` / `0x08054530`, `0x08056F74` / `0x08056F98`)

1. **Prefix table** (e.g. `0x080544DC..0x08054530`): array of ROM pointers to 8-byte per-map descriptor records (terminated by `0xFFFFFFFF` padding).
2. **Map-slot header** (`0x08054530`, 16 bytes): per-submap flags/indices (`02 03 03 03 03 03 03 0B 0B 0B 0B 0B 0B 0B 00 00`).
3. **Story-progress dispatch table** (unaligned variable-length opcodes):
   - `0x02 <lo:u8> <hi:u8> <target:u32>` (7 bytes): if `lo <= story_progress <= hi`, jump to `target`.
     - Example at `0x08054540`: `[0x00..0x0F] -> 0x08054561`, `[0x10..0x1F] -> 0x080545C8`, `[0x40..0x4F] -> 0x080545F9`, `[0x57..0x57] -> 0x0805461F`.
   - `0x01 <target:u32>` (5 bytes): unconditional / fallback jump to `target` (e.g. `0x0805463D`).
4. **Action bytecode bodies** (at each `target` address): unaligned variable-length cutscene/map-setup commands (e.g. `0x26 <u16 flag> ...`). Because the dispatch opcodes are 5 and 7 bytes wide, ROM pointers inside `code_04`–`code_05` are frequently unaligned — which is why only the 4-byte-aligned pointer tables (`28` tables, `+2,724B` in `5c46201`) were carved by `segments_*.toml`, while the bytecode bodies remain `incrom` until a dedicated bytecode disassembler macro set is added.
