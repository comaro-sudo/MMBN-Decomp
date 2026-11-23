# Gemini round 12: seven tasks (assigned by the owner, written by Claude, 2026-11-23)

Round 11 was excellent: its test found five real bugs. Claude fixed four in `4f5ec2b` and added your battle-exit clear (`wp_d3_battle_exit_clear.asm`):
- Duo / GP Power text: the e-Reader threshold is raised to `0x1000`, not `0x139`. With `0x139`, ids above `0x138` would index the 2-entry table out of range.
- Trash can and Duo's signal: now use `list[7] 0x08`.
- Duo rematch: b1 is now `0`, and a won Duo rematch pays the Duo Soul + Duo D from the lobby results hook.
- Before that, `69176cd` added **WP-B4** (GMD contents follow the Frequency Tuner, `wp_b4_gmd_tier.asm`).

**Claude is next building in `patch/`:** SP+ (from your `sp-plus-design-2.md`), the Duo palette (`form-palette-hook.md`), the Duo passive (`duo-soul-passive.md`) and the friendly matches (`friendly-matches-design-2.md`). So:
- **don't edit `patch/`, `docs/` or `TODO.md`**;
- `decomp/` is yours (keep both ROMs byte-identical);
- test outputs go outside the repo;
- stage by path, don't push;
- one handoff at the end.

Every finding gives exact RS and BM values.

## 1. Re-test the round-11 fixes (`decomp/notes/patch-test-results-5.md`)
Build the current patch (`4f5ec2b` or later) and check:
- **Text:** Duo and GP Power names and descriptions in the Folder, Pack and Library. Also check that a Soul's name (e.g. in the Custom Screen) didn't change.
- **Sprites:** the trash can and Duo's signal draw (crystal prop).
- **Duo rematch:**
  - the results screen appears after a win;
  - an S rank raises `0x0E5A`/`0x0E5B` (V1 → α → β → Ω);
  - a win sets `0x0023` and `0x0E72` and puts Duo D in the pack once;
  - β/Ω drop Duo D.
- **Battle-exit clear:** flee or lose a lobby rematch, and `0x020015EC`–`0x020015EF` read 0 afterwards. Then win a random battle with an S rank: no tier change.
- **WP-B4:** on a first playthrough, set the Tuner to Tier 3, jack in, open a GMD: is the reward from the Super Hard block of `re07-mystery-data.md`? And Auto still uses the playthrough.

## 2. Duo Soul: why MegaMan doesn't transform (`decomp/notes/duo-soul-transform.md`)
Round 11 found that the unite is consumed and `BN4_FORM_TURN_START` stores 13 in `[r7,#0x10]`, but no transform happens.
- Trace the consumer, starting from the Soul-change cutscene object in `code_08` (`sub_080EBC20` calls `BN4_FORM_STATE` `0x0800E05C`), and from whatever spawns it.
- Find every check or table that rejects or mis-handles form 13: bounds `cmp #12`, 13-entry tables, the cutscene's per-Soul graphics/text/sound.
- For each: the exact address in RS and BM, the bytes, and the fix (a bound to change, or a table entry to borrow MetalSoul's).

## 3. Duo story grant: does the final battle reach the deletion state? (`decomp/notes/duo-story-grant-test.md`)
With a save just before the final battle (PT1: Duo V1, story battle 93), win it (1-HP trick) and check:
- whether `0x0023`, `0x0E72` and Duo D come;
- if not, which Duo state actually runs at the deletion in the story battle (break on `0x080C4780` / `0x080C479C`, `r5[0x0A]`).
- Propose the exact hook that fires once when the story's Duo is beaten. RS and BM.

## 4. More veneer space within `bl` range (`decomp/notes/veneer-space.md`)
SP+ and the Duo palette need at least **4** more 8-byte, word-aligned veneers. The only one left is RS `0x080C4790` / BM `0x080C479C`. They must lie within ±4 MiB of `0x08003194`, `0x08006802`, `0x08013A5A` and `0x080107E8`.
- Find dead or unreferenced code or data bytes in `code_00`–`code_07`: unreachable functions, unused tables, padding. For each:
  - the RS and BM address and size;
  - the proof it's unreferenced (no `bl`/`b`, no pointer to it in either ROM);
  - confirm it's not in a range `--check` re-assembles (the chip table).
- Ten candidates would be ideal.

## 5. Better props for the trash can and Duo's signal (`decomp/notes/prop-sprites.md`)
A contact sheet (outside the repo) of every uncompressed `list[7]` prop (`0x06`–`0x09`, `0x12`, `0x20`, `0x45`, `0x4A`, `0x53`, `0x5C`, `0x5D`, `0x67`, `0x6B`, `0x6D`, and any more) and the uncompressed `list[6]` entries. Pick:
- a junk / trash-pile look for the ACDC Town trash can;
- something Duo-like or comet-like for the signal in Meteor Comp 1.

For each: the exact NPC script opcode bytes (`0x25 id 0x1C` or `0x17 id`) and a valid animation. Also, is there a way to draw a compressed sprite as an NPC (another opcode)?

## 6. WP-B6: Waiting Room save protection, research (`decomp/notes/waiting-room-protection.md`)
Plan (`plan/03-phase1-rom-overhaul.md` WP-B6): link transfers through the Waiting Room can overwrite Soul data.
- Find the link-receive path: what it writes into the save (Soul flags `0x0017`–`0x0022`, Navi records at `0x020014F0`, chip keys).
- Find the exact writes that can remove a Soul the player already has, or change one.
- Propose a guard: OR-merge instead of overwrite, or skip the Soul fields. Give exact code points.
- Note: with Violet Eclipse every Soul is earnable, so the Waiting Room is optional. Say what it is still used for.

## 7. WP-A3 key-item prices: the facts for an owner decision (`decomp/notes/key-item-prices.md`)
The plan lists key-item prices as an open decision.
- List every key item or upgrade that is **sold** (HP Memory, SubMemory, RegUp, ExpMemory, NaviCust programs…): shop, price in each version, and the playthrough or story gate.
- Say which are missable or unreasonably priced for a one-save completion.
- Give 2–3 pricing options with the exact shop-record bytes to change. Facts only: the owner decides.
