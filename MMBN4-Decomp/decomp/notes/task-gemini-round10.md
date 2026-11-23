# Gemini round 10: seven tasks (assigned by the owner, written by Claude, 2026-11-23)

Round 9 was excellent: the Duo Soul framework and the Duo rematch are now owner decisions in `docs/pending/duo-soul.md` and `docs/completion/bosses.md`.

**Claude is working in `patch/`** (the Duo and Grand Prix Power chip-art package, the Duo grant and the Duo rematch). So:
- **don't edit anything under `patch/`**, nor `docs/` or `TODO.md`;
- `decomp/` is yours as usual (tasks 2 and 3 edit it). Keep both ROMs byte-identical (`make -B compare` for rs and bm) after any asm change;
- you may run `python patch/build.py` and the PC port, with outputs **outside the repo**;
- for the design tasks, write only the new notes named below; put proposed names in a section at the end;
- stage only your own files by path, and don't push. Write one handoff at the end.

Do them in order. Each design must give **exact RS and BM values** Claude can drop into `patch/` as they are.

## 1. In-game regression test of the current patch (`decomp/notes/patch-test-results-3.md`)

Build the current patch and test in the PC port, with evidence outside the repo:
- **Lobby after Claude's fixes (`11d2a39`):**
  - the operators show the **right sprites**;
  - each one **appears only once its Navi's Soul flag** (`0x0017`–`0x0022`) is set, and is hidden before;
  - the trash can is the junk-pile prop, and still warps to JunkMan;
  - a rematch still starts.
- **No regressions:** the status screen's two pages, the Z-Saver Mystery Data, the Free Tournament (7 Soul Navis, no trades), the Frequency Tuner, the Bass both-chips drop, the random drops, the Souls completion star.

## 2. Merge the pending names into the decomp (`decomp/symbols/names.txt`, then a full regeneration)

Merge the proposed names from your round 8–9 notes (`lobby-fixes.md`, `sp-plus-research.md`, `duo-gpp-research.md`, `duo-soul-research.md`, `duo-rematch-research.md`, and any others not yet in `names.txt`).
- Only `[A-Za-z0-9_]` in names.
- Run a full regeneration (RS, BM, `merge_bm.py` per segment), then both compares. Commit `names.txt` and `asm/`.

## 3. Decomp: carve the decoded record tables as structured data (`decomp/config/`, then regenerate)

Several tables are now fully understood. Carve them as **records with named fields** (your `data_tables` `fmt = "record"`), not as hex words:
- `BN4_SHOP_STOCK_INIT` (8-byte stock entries, `docs/re/shops.md`) and `BN4_SHOP_RECORDS`;
- `BN4_NUMBER_TRADER_TABLE` (32 × 12-byte entries);
- the Soul tables (`BN4_SOUL_KINDS`, `BN4_SOUL_FLAGS`, the 16-entry form tables from `duo-soul-research.md`);
- any other table whose layout one of your notes decoded.

Keep both ROMs byte-identical, and record the coverage before and after.

## 4. Friendly matches: the exact patch design (`decomp/notes/friendly-matches-design.md`)

The owner's design (`decomp/notes/canon-navis.md` §6–§7, `docs/pending/free-tournament-secret-chips.md`):
- 3 friendly matches per tournament;
- all before the final, 2–3 before the canon fight; Dex after the AquaMan fight, Raika before Chaud;
- a win grants that Navi's Soul (`FC 0C <idx>`, already in the patch);
- a loss can be retried;
- the matches run at the normal story tier.

Give, for each of the 9 matches:
- the **venue map** and its NPC list (slots used/free; does it need relocating?);
- the **story-phase flags** that open and close the window (before or after the canon fight, before the final), per tournament;
- the operator's position and sprite;
- the battle record to start (plain battle, as the lobby does);
- how the talk script connects to a **compiled dialogue archive**. Claude's pipeline: `patch/src/text/new/<name>.tpl` → `.incbin <NAME>_BIN`, opened through the lobby's `OpenMapDialogue` hook (talk index `≥ 0xF0`) or a new one; see `plan/text-insertion.md`.
- Also: does a win need a separate "friendly done" flag, so the NPC disappears after a win?

## 5. SP+: the exact patch design (`decomp/notes/sp-plus-design.md`)

From your `sp-plus-research.md` (three hooks plus the SP+ flag at `0x020015EE`), and the owner's decisions in `docs/pending/boss-double-hp.md`:
- **Lobby:** SP+ is the 5th step after an S rank against SP. How it fits the lobby's 2-bit tier counter (4 states today: V1, V2, V3, SP); a third bit, or a separate "SP+ unlocked" flag per Navi? Which free flags?
- **The 8 shared Navis** (TopMan, ShadeMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan): their roaming SP ghosts become SP+ after an S rank. Where their roaming encounter is chosen (`BN4_STORY_OFFSET` `+0x10` tables, RE-06), and how a battle from that table would set the SP+ flag.
- **Duo and Bass:** their SP+ step, in the Meteor Comp and Black Earth 2 rematches.
- **The reward:** "a high amount of Zenny or BugFrags". Propose a concrete reward word (e.g. 30,000Z) and where it's granted (`BattleResultsGrantReward` hook).
- The exact hook code points and register usage, in RS and BM.

## 6. Duo Soul: the exact patch design (`decomp/notes/duo-soul-design.md`)

The owner's decisions (`docs/pending/duo-soul.md`): a **MetalSoul recolour**, granted by **beating Duo V1**. Propose defaults for:
- **the Duo palette:** 16 BGR555 colours mapped onto MetalSoul's palette slots (dark violet armour, crimson crest, gold/cyan highlights), with MetalSoul's palette address and the slot meanings;
- **the charge shot** (`BN4_FORM_LOADOUTS[13]`): which existing weapon id gives a Duo-like attack (Duo's meteor fist or laser objects, `kind == 3`);
- **the passive** (`BN4_FORM_ENTERS[13]`): a Duo-themed effect, and how other Souls' passives are written;
- **the unite chips:** which chips get unite class `0x0D` (Duo-themed: `MetrKnuk`, `AntiDmg`, the Meteors chips…), and the `chip[+7]` edits;
- **the 3rd status page:** how the two-page patch (`wp_d4_status_two_pages.asm`) extends to 13 (page offsets 0/6/12);
- **the grant:** the exact code at the post-Duo hook (`sub_080C4780`), in RS and BM, to set flag `0x0023` and `WriteChipKey(0x16D)`;
- **the Soul's icon:** whether `0x16D`'s record has an icon pointer, and its format.

## 7. Chip names and descriptions: the text design (`decomp/notes/chip-text-design.md`)

New text is needed for **Duo** and **Grand Prix Power** (names; descriptions: Duo's is "Fist of Justice: MetKnukl!"), and for the **DuoSoul** pseudo-chip if it lacks a name.
- Where chip **names** live (your Chip Names Bank 1: RS `0x08748C14` / BM `0x08748708`, with `FF 00 00 E5` / `FF 00 01 E5` for `0x137` / `0x138`) and chip **descriptions** (which archive, which entries).
- Whether they're TextPet archives in the US index (`tools/textpet/indexes/mmbn4rs-us.tpi`, `mmbn4bm-us.tpi`) and their archive offsets.
- **Open TextPet question** (`plan/text-insertion.md`): when a patch `.tpl` changes **one script** of an existing archive, does TextPet keep the archive's other scripts, or must the `.tpl` contain all of them? Test it on a scratch copy of the ROM (outside the repo). If it needs all of them, propose how to commit only our lines (e.g. a build-time merge that extracts the archive, patches our scripts in, and inserts it).
- Give the exact `.tpl` content (our lines only) for the three names and descriptions.
