# RE-01 / RE-02 / RE-03: save layout, event flags, playthrough counter

Claude's reverse-engineering lane, 2026-11-22. This note is based on static reading of Gemini's generated assembly in `decomp/asm/` (at the time RS was in `decomp/asm/` and BM in `decomp/asm/bm/`; since DC-4 both share `decomp/asm/`) and on small reads of the player's ROMs. Nothing from the ROM is reproduced here beyond addresses, offsets and constants.

**Status legend.**
- **V**: verified by reading the code, in both versions where a BM address is given.
- **R**: also confirmed in the running game (§7).
- **T**: tentative, an inference that still needs a runtime check.

## 1. Where the save lives

| Item | RS | BM | Status |
|---|---|---|---|
| Saved image: EWRAM `0x02000000`, `0x73D2` bytes, to SRAM `0x0E000000` | `sub_0804885C` | `sub_08048864` | V |
| Load: SRAM to EWRAM, same size | `sub_080488EC` | `sub_080488F4` | V |
| Writes retry up to 3 times through the SRAM library's write-and-verify (`sub_08111924` / `sub_08111994` RS) | | | V |
| Second block: `0x020073E0`, `0x1C` bytes, to SRAM `0x0E007F00`: magic, then a plain byte-sum at `0x020073F4` (write `sub_080489FC`, read `sub_08048A5A`) | `0x080489FC` | `0x08048A04` | V (purpose T) |

### 1.1 The save block moves (anti-cheat relocation)

The game's main save data is one `0x3CF0`-byte block. It doesn't sit at a fixed address.

- Its base is `0x02002130 + shift`. `shift` is a u32 at `0x02001550`, an even number from `0x000` to `0x1FE` (`GetRNG() & 0x1FE`).
- `sub_08006128` picks a new shift and moves the block with a memmove in IWRAM (`0x03006FA9`, length `0x3CF0`). It then rebuilds 23 toolkit pointers, `eToolkit+0x40` to `+0x98`, as base + shift + a fixed offset from the ROM table at `0x080061AC` (same address and contents in RS and BM). **V**
- `sub_0800616A` rebuilds the same pointers without moving the block. It runs after a load. **V**
- `sub_08006128` runs after every load (`sub_080488EC`). It also runs on game-mode changes: the mode routines `sub_08004BE0`–`sub_080051C8` (map, jack-in, PET, bracket and others; 11 call sites), `sub_0803A704`, and `sub_0800045E`. That one sets up the toolkit (`sub_080060CC`) and then relocates, so it looks like the boot or soft-reset routine. `names.txt` calls it `HBlankIntr`, which looks wrong (T). So the block moves many times per session. **V**, **R**: during the intro, with no load in between, the shift went from `0x16C` to `0xBC`.

All save code reaches the data through these pointers (`mov rX, sl; ldr rY, [rX, #slot]`). Nothing uses absolute addresses, which is why a fixed-address cheat breaks after each load.

| Toolkit slot | Offset in block | Size | What it is | Status |
|---|---|---|---|---|
| `+0x40` | `+0x0000` | `0x84` | Game state: `BN4_MAP_STATES` index at `+0`, map/mode bytes `+4` and `+5` (heavily read), HP `+0x20` and `+0x22` (u16, cap 9999), **Zenny `+0x74`** (u32, cap 999999), **BugFrags `+0x78`** (u32, cap 9999) | V, R (Zenny and BugFrags) |
| `+0x44` | `+0x0084` | `0x94` | Progress: playthrough `+0x0D`, HP `+0x14` and `+0x16` (100/100 on a new game; which is current and which max is T), game time `+0x2C`, checksum `+0x34`, magic `+0x54` | V, R |
| `+0x48` | `+0x0118` | `0x3E4` | **Event flags**, `0x3E0` bytes = 7,936 flags (§2) | V |
| `+0x4C` | `+0x04FC` | `0xB8` | **Folders**: 3 × 30 u16 `chip \| code << 9` (code 26 = `*`), then 4 bytes. A new game has folder 1 = the 30 starting chips (Cannon A/B first) and folders 2–3 filled with chip 0 | R |
| `+0x50` | `+0x05B4` | `0xF04` | **Chip pack**: 320 × 12 bytes, per chip id the owned count of each of its 4 codes (`ChipPackInit` `0x080192A8`, `ChipPackCountById` `0x0801922C`) | V, R (Gemini) |
| `+0x54` | `+0x14B8` | `0x194` | **Items**, indexed by item id: key items; upgrades `0xA0` HPMemory, `0xA1` ExpMemory, `0xA2`–`0xA4` RegUP, `0xA5` SubMemory (starts at 4); **SubChips `0xB0`–`0xB5`** (capped at the SubMemory count); NaviCust programs `0xC0+` (cap 9). `InventoryAddItem` `0x0802D42C` | V, R (Gemini) |
| `+0x58` | `+0x164C` | `0xDC4` | **Shop stock** ([shops.md](shops.md)) | V |
| `+0x5C` | `+0x2410` | `0x24` | NaviCust 5×5 grid | V, R (Gemini) |
| `+0x60` | `+0x2434` | `0xCC` | NaviCust placed programs, 25 × 8 bytes | V, R (Gemini) |
| `+0x64` | `+0x2500` | `0x14` | NaviCust compiled bug flags | V, R (Gemini) |
| `+0x68` | `+0x2514` | `0x4` | Active e-Reader Mod Card id (0 = none; effect table `dat_08041E8C`, 134 entries) | V, R (Gemini) |
| `+0x6C` | `+0x2518` | `0x4` | u16 Max HP bonus (Mod Card / NaviCust) | V, R (Gemini) |
| `+0x70` | `+0x251C` | `0x10` | 7 loaded e-Reader / Mod Card slots (`0xFF` = empty) | V, R (Gemini) |
| `+0x74` | `+0x252C` | `0x804` | **Mystery Data state**, 1,024 × 2 bytes for flags `0x0D00`–`0x10FF` (RE-07) | V (Gemini) |
| `+0x78` | `+0x2D30` | `0x204` | Navi stats, 8 × `0x40`: slot 0 = MegaMan, 1–7 = the Waiting Room's traded Navis (`WaitingRoomReceiveTradedNavi` `0x0803FA90`). A new game has 100 at `+0x30`, `+0x32` and `+0x34`, and 500 at `+0x36` | V, R |
| `+0x7C` | `+0x2F34` | `0x704` | Navi AI / chip patterns, 8 × `0xE0`, same slots as `+0x78` | V, R (Gemini) |
| `+0x80` | `+0x3638` | `0x194` | Item anti-cheat keys: `[+0x80][id] = [0x02000240 + id] XOR SAVE_XOR_KEY_0` (`0x2C` RS / `0x71` BM) | V, R (Gemini) |
| `+0x84` | `+0x37CC` | `0x204` | Chip keys mirror: CE's `BN4_CHIP_GATE` compares `eToolkit[+0x84] + id` with `0x02000670 + id XOR 0x64` | T (from CE) |
| `+0x88` | `+0x39D0` | `0x204` | Chip key table: for each set Library flag `0x18C0 + i`, byte `i` = `[0x020019A0 + i] XOR 0x38` (`sub_08006594`) | V, R |
| `+0x8C` | `+0x3BD4` | `0x8` | **Zenny shadow**: Zenny XOR `[0x02000180]`. With 0 Zenny it equals the key, which looked like a ROM pointer | V, R |
| `+0x90` | `+0x3BDC` | `0x8` | **BugFrag shadow**: BugFrags XOR `[0x02001E94]` | V, R |
| `+0x94` | `+0x3BE4` | `0x104` | Byte shadows of `0x020003D0[i] XOR 0x43`, written per entry (`sub_08006510`; `sub_08006524` writes the complement; `sub_0800653A` checks). Not all entries match on a new game | V (contents T) |
| `+0x98` | `+0x3CE8` | `0x8` | Navi-stat shadow: stat `+0x36` XOR `[0x02001BA0]` (`sub_08006558`, check `sub_08006570`). Didn't match on a new game, so it's probably written only at certain events | V (timing T) |

Sizes are the gap to the next slot. The slot boundaries are **V**. Naming the unnamed slots is the next RE step; a field-usage scan is in §5.

### 1.2 Checksum and scrambling

| Step (save, `sub_0804885C`) | Detail | Status |
|---|---|---|
| 1. Refuse if flag `0x118A` is set | Returns 2 | V |
| 2. New key | `sub_08006368`: `key = ~GetRNG()`, retried until nonzero, stored at `0x02001554` | V |
| 3. Magic | 20 bytes `"ROCKMANEXE4 20031022"` (ROM `0x080489E4` RS / `0x080489EC` BM) copied to progress `+0x54` | V |
| 4. Checksum | Progress `+0x34` is zeroed, then set to the byte sum of `0x02000000`–`0x020073D1`, **plus `0x16` in Red Sun or `0x22` in Blue Moon** (`sub_08048960`) | V |
| 5. Scramble | `sub_08006338`: every byte of the image XOR `key & 0xFF`; then the key word at `0x02001554` is restored in the clear | V |
| 6. Write and verify, up to 3 tries | Success **sets** flag `0x1166` (after the write, so the file holds it clear), failure clears it and returns 1 | V, R |
| 7. Unscramble RAM again | `sub_08006350` (same XOR) | V |

Load (`sub_080488EC`) reverses it: read, XOR with the key byte found at `0x1554` (which is stored in the clear), rebuild the pointers, check the checksum (`sub_0804898E`) and the magic (`sub_080489C0`). If both pass, it sets flag `0x1166`. If either fails, it clears `0x1166` and reseeds the RNG. Finally it re-randomizes the shift.

**How to read a `.sav`** (**R**: tested on real RS and BM saves, §7):

1. `key = u8 at 0x1554`.
2. XOR every byte except `0x1554`–`0x1557` with `key`.
3. `shift = u32 at 0x1550 & 0x1FE`.
4. `block = 0x2130 + shift`.
5. The event flags are at `block + 0x118`, the progress struct at `block + 0x84`, and the checksum at `block + 0xB8`.

**Violet Eclipse impact.** A merged ROM must choose one checksum constant. To load saves from both carts, its load routine has to accept both `+0x16` and `+0x22`. The checksum is also the cheapest version tag: a save says which cart wrote it. The save protection of WP-B6 also hooks here.

## 2. Event flags (RE-02)

The flag buffer is `[eToolkit+0x48]`, i.e. block `+0x118`. Flag `n` is bit `0x80 >> (n & 7)` of byte `n >> 3`: MSB first, the same scheme as BN6. **V** (RS addresses; BM is +4 for all of these).

| RS | BM | Function | Arguments |
|---|---|---|---|
| `0x08025048` | `0x0802504C` | Set | `r0 << 8 \| r1` (falls through into the next) |
| `0x0802504C` | `0x08025050` | Set | `r0` = flag |
| `0x08025064` | `0x08025068` | Clear | `r0 << 8 \| r1` |
| `0x08025068` | `0x0802506C` | Clear | `r0` |
| `0x08025084` | `0x08025088` | Toggle | `r0` |
| `0x0802509C` | `0x080250A0` | Test | `r0 << 8 \| r1`; result in the Z flag (Z set = flag clear) |
| `0x080250A0` | `0x080250A4` | Test | `r0` |
| `0x080250B6` / `0x080250BA` | `+4` | Set range | `r0` first, `r2` count |
| `0x080250E0` / `0x080250E4` | `+4` | Clear range | |
| `0x0802510E` | `0x08025112` | Toggle range | |
| `0x08025138` | `0x0802513C` | All set? | `r0` first, `r2` count; `r0 = 1` and Z clear when every flag is set |
| `0x08024FFC` | `0x08025000` | Clear all `0x3E0` bytes | New game |
| `0x0802502C` | `0x08025030` | Clear flags `0x1100`–`0x114F` | Temporary flags (T) |
| `0x08025010` | `0x08025014` | Clear flags `0x1150`–`0x115F` | Temporary flags (T) |

Code calls these at about 186 set sites and 373 test sites; most story flags are set by event scripts instead (§4).

**Flags identified so far**

| Flag | Meaning | Status |
|---|---|---|
| `0x17`–`0x1C` | Red Sun's six Double Souls obtained (Souls 1–6) | V |
| `0x1D`–`0x22` | Blue Moon's six Double Souls obtained (Souls 7–12) | V |
| `0x31D` | Lan has picked up the PET (opening, Lan's room); the START menu stays locked until then | R |
| `0x1130`, `0x1151` | Cleared at the same moment as the PET pickup: temporary scene flags in the `0x1100`–`0x115F` ranges | R |
| `0x16C0 + id` | **Library: chip `id` obtained** (512 flags). A new game sets exactly the ten distinct chips of folder 1 (1, 4, 15, 29, 48, 49, 91, 107, 117, 149), matching the save screen's "Library S10" | R |
| `0x18C0 + i`, `0x1AC0 + i` | Two more 512-flag chip ranges checked by the chip-key routine `sub_080065E2`, all clear on a new game. Possibly Mega/Giga or per-code records | V (meaning T) |
| `0x1166` | Last save or load succeeded | V, R |
| `0x1188` | Second block (`0x0E007F00`) last written or read successfully | V |
| `0x118A` | Saving refused | V (when it is set: T) |
| `0x0D00`–`0x10FF` | **Mystery Data taken** (Blue/Purple), one flag per placement, plus a 2-byte state per flag in save slot `+0x74`. Kept across New Game+ and advanced by `MysteryDataAdvanceAll` (`sub_080ADE5C`). See `decomp/notes/re07-mystery-data.md` (Gemini) | V (RE-07) |
| `0x15C0`–`0x163F` | Logged by `sub_0802516E`: sets `f` and `f + 0x80`, and appends `f - 0x15C0` to a list at `0x02006EC0` (count `0x02001600`; cleared by `sub_080251D6`). Not Mystery Data (RE-07 corrected this guess); the debug menu and a story hook (`sub_08085EC0`) set it. Purpose unknown | T |
| `0x1200`–`0x13FF` | Logged by `sub_0802519C`: sets `f` and `f - 0x1200 + 0x1400`, and appends to 64-entry lists at `0x020017A0` (counts `0x02001500`) | T |

## 3. Playthrough counter (RE-03)

The playthrough counter is the progress struct's `+0x0D`, a u8. **V**

| RS = BM | Function | Status |
|---|---|---|
| `0x08000C44` | Raw count | V |
| `0x08000C4C` | Count capped at 2: **the tier** (1st, 2nd, 3rd+ run) | V |
| `0x08000C5A` | Increment, capped at 99 (`0x62`); its one caller is `sub_08000DE8` | V |
| `0x08000C2C` | Game time `+0x2C`, frames, capped at `0x014988F0` (99:59:59 at 60 fps) | V |

**Readers.** A scan found 33 functions that read it (RS), either through the three accessors or directly with `ldrb [progress, #0xd]`. **T**: a heuristic scan; reads through a pointer kept in a register longer may be missed.

- **Battle and enemies:** `sub_0800B036` (CE `BN4_ID_RAISE`: virus +2 per tier, Navi +1), `sub_08007DC0`, `sub_080B1144` (tier). This is the routine Prof. 9's WoodMan fix replaces (`WOODMAN_FIX` = `0x080B1142` in `patch/src/rs/version.asm`; BM `0x080B114E`). **V**: in the ROM the routine starts at `0x080B1144`, and `0x080B1142` is a 2-byte zero pad after the literal `0x1AD`. The fix (`ext/BN4-WoodMan-Fix/source/src.asm`) reassembles the routine 2 bytes earlier, into that pad, which frees one instruction slot. There it adds `strb r0, [r7, #2]` after `cmp r0, #0x17; bne`, so the counter at `[r7+2]` stops at `0x17` instead of running past it. It then rewrites the routine's pointer at `addr - 0x3E` (`0x080B1104` RS) to the new start. WP-A1 can port it as is. Confirmed in both ROMs: the pointer at `0x080B1104` (RS) is `0x080B1145`, and at `0x080B1110` (BM) is `0x080B1151`, i.e. `addr + 2 | 1` each time.
- **Tournaments (RE-04):** `sub_08044744` (CE `BN4_NEXT_OPPONENT`: `BN4_OPPONENT_BATTLES[id] + playthrough`), `sub_0804480C`.
- **Other tier readers:** `sub_0808EBC4`, `sub_0809C18C`, `sub_0809F938`, `sub_080A02A2`, `sub_080A1BBC`, `sub_080A5018`. Likely encounter or GMD tables (RE-06/RE-07); not yet looked at.
- **Raw readers:** `sub_0802A540`, `sub_080A5270`, `sub_080ADEB2`, `sub_080ADF5C`.
- **Direct readers:** `sub_080255A8`, `sub_080256F8`, `sub_0802A1FC`, `sub_0802ABEA`, `sub_0803968C`, the `sub_0803A300`–`sub_0803AE98` group (7 functions, likely one menu or system), `sub_0804F21C`, `sub_08050C54`, `sub_080A3578`.

**Violet Eclipse impact.** The Frequency Tuner (WP-B2) can redirect `sub_08000C4C` and the direct tier readers to a player-chosen tier. The real count stays for New Game+ and for scripts. These 33 sites are its whole surface; each needs a check.

## 4. Double Souls (RE-05, first half)

**V**, from `sub_0801FF84` (CE `BN4_UNITE_CHECK`, same address in both versions) and two 12-byte ROM tables:

- `BN4_SOUL_KINDS` (`0x08020008`): the chip kind each Soul unites with. It is identical in RS and BM: `04 08 0A 07 00 02 06 05 0B 09 01 03`.
- `BN4_SOUL_FLAGS` (`0x08020018`): the Soul's event flag.
  - RS: `17 18 19 1A 1B 1C FF FF FF FF FF FF`
  - BM: `FF FF FF FF FF FF 1D 1E 1F 20 21 22`

The unite check matches the sacrificed chip's kind against the table, then refuses the Soul if its flag byte is `0xFF`, if the event flag is clear, or if the Soul was already used this battle (bit `n` of `0x0203BF88`).

**So both ROMs already have code paths for all 12 Souls.** The other version's Souls are blocked only by `0xFF` entries in this table and by the missing data behind them (graphics, AI, forms). For WP-D4/Option A, the first concrete patch step is to write `1D`–`22` into Red Sun's table, then supply the Blue Moon Soul data and its grant events. Which Soul is which index (CE: Soul 1 = Roll, Soul 12 = Wood) needs a check against the in-game list.

**Who sets `0x17`–`0x22`.** No code does it with an immediate value: a search for `movs r0, #0x17..0x22` before a set call found nothing. The flags are set by event scripts, so the second half of RE-05 means finding the script opcode that sets a flag (TextPet's script commands) and the scripts that use these IDs.

## 4b. Economy (for Phase 1 Tier A)

| RS | BM | Routine | Returns |
|---|---|---|---|
| `0x0802D59A` | `0x0802D59E` | AddZenny(`r0`) | 0 ok, 1 already at the cap or tampered, 2 clamped to 999999 |
| `0x0802D5D0` | `0x0802D5D4` | SetZenny(`r0`) | |
| `0x0802D5E4` | `0x0802D5E8` | SubZenny(`r0`) | 0 ok, 1 Zenny is 0, 2 not enough (unchanged) or tampered |
| `0x0802D610` | `0x0802D614` | GetZenny | 0 if tampered |
| `0x0802D624` | `0x0802D628` | AddBugFrags(`r0`) | as AddZenny, cap 9999 |
| `0x0802D658` | `0x0802D65C` | SetBugFrags | |
| `0x0802D66C` | `0x0802D670` | SubBugFrags | as SubZenny |
| `0x0802D698` | `0x0802D69C` | GetBugFrags | |
| `0x08006498` / `0x080064B0` | same | Zenny shadow: write / check (Z set when OK) | |
| `0x080064D4` / `0x080064EC` | same | BugFrag shadow: write / check | |

Every change goes through these routines, which update the shadow, so a patch that changes Zenny must call them instead of writing `+0x74` directly. Otherwise the next check reads as tampering and the routines refuse to work.

AddZenny has four callers:
- **`sub_08021684`:** a reward handler (kind 1 = Zenny, sound `0x99`). This is the likely battle-reward path; check it against `BN4_REWARD_ROWS`.
- **`sub_080501B8` and `sub_0805081C`:** the amount is a 4-byte little-endian value read from data, probably a script command or Mystery Data.
- **`sub_08090698`:** a fixed amount from a literal.

**Battle reward path (RE-08).**

- **Reward word.** u16 `0xFFFF` = none; bits 14–15 = kind, decoded by `sub_08021A98`:
  - kind 0 = chip, id in bits 0–8 and code in bits 9–13;
  - kinds 1, 2 and 3 = Zenny, an unknown kind, and BugFrags, each with a 14-bit amount (≤16,383).
- **Format.** It matches CE's `BN4_REWARD_ROWS` (`0x080F5F50`: 20 rewards per row, rows 0–9 when MegaMan's HP is above 3/8, rows 10–19 at or below). **V**
- **Results screen.** Two small state machines share the handlers:
  - `sub_08021470`, with states at ROM `0x08021484`;
  - `sub_0802152C`, with states at ROM `0x08021540`: init `sub_0802154C`, then `sub_08021624`, then `sub_08021684`.
- **Init.** It shuffles 42 tile indices (`0x2A`) and loads the reward's picture by kind (the chip's own image, the Zenny picture, two variants for kind 2, four BugFrag sizes by amount) into VRAM `0x0600B900`. This is the results screen's tile-by-tile reveal.
- **Grant.** `sub_08021684` then grants the reward: chip → `sub_08021CA8` (sound `0x99`), Zenny → AddZenny, kind 2 → `sub_08021B92`, BugFrags → AddBugFrags.
- **Status.** **V** for the code; that this is the battle results screen is **T** (strongly suggested by the reveal), to confirm in a battle.

The Zenny ×1.75 package (WP-A2) can scale at the battle-reward caller only, so shop refunds and scripted gifts stay as they are. The battle caller is `sub_08021684` (above), so the patch hook is its `bl sub_0802D59A` at `0x08021704` (RS).

The HP Memory effect is `sub_0802D508` (T: the name comes from its body). It adds 20 HP, or 10 when flag `0xC49` is set, to game state `+0x20` and `+0x22` and to progress `+0x14`, each capped at 9999.

## 5. Tool used

The field scan of §3 and of the slot table comes from a throwaway script (read-only on `decomp/asm/`). For each `mov rA, sl; ldr rB, [rA, #slot]`, it records the `ldr`/`str` widths and offsets made through `rB`. It's in Claude's scratchpad, not the repo. If it proves useful, it can move to `decomp/tools/` through Gemini.

## 6. Next steps

1. Watch flags `0x17`–`0x1C` across a Soul event; this needs a late-game state.
2. Name the remaining save slots from their readers and from diffs across in-game actions: pack, Library, NaviCust, Zenny, BugFrags, mail, key items. On a new game, `+0x5C`–`+0x6C` are all zero, `+0x58` holds about 440 8-byte records, and `+0x70` is 14 bytes of `0xFF`. The bulk of the Zenny and chip work (Phase 1 economy) depends on this step.
3. RE-05 second half: the script flag opcode and the scripts that set `0x17`–`0x22`.
4. RE-03 completion: read the six tier readers and the `sub_0803A3xx` group.
5. RE-04: CE already names `BN4_START_TOURNAMENT`, `BN4_ADVANCE_ROUND` and `BN4_NEXT_OPPONENT`; follow the pool fill (`0x08043FE4`, `0x08044058`, `0x0804422C`, `0x0804428C`) to the RNG.

## 7. Runtime checks (2026-11-22)

These runs used the PC port's headless test mode with the new `--dump`, `--state-in` and `--state-out` options (`ports/pc/README.md`). The inputs started a new game, skipped the intro, picked up the PET in Lan's room, and saved from the PET menu. The same inputs work on both versions.

| Check | Red Sun | Blue Moon |
|---|---|---|
| All 23 toolkit save pointers = `0x02002130 + shift + BN4_SAVE_SLOT_OFFSETS[i]` | match (shift `0x16C`, later `0xBC`) | match |
| `.sav` decoded with §1.2: magic `ROCKMANEXE4 20031022` | yes (key byte `0x90`, shift `0xF0`) | yes (key byte `0x6F`, shift `0x1EC`) |
| Stored checksum = byte sum + constant | `+0x16` OK | `+0x22` OK |
| Decoded save vs RAM right after the save | 7 bytes differ, all expected: RNG `0x02001790`, play time (progress `+0x2C`), and the flag byte holding `0x1166` (set after the write) | same 7 |
| Playthrough byte (progress `+0x0D`) on a new game | 0 | 0 |

The scripts (`run.py` input runner, `savdec.py` save decoder, `fdiff.py` flag diff) are throwaway; `savdec.py`'s logic is §1.2. If a save tool is wanted, it belongs in `tools/` of the patch or port project, not in `decomp/`.
