# Handoff: Claude, Violet Eclipse patches (updated 2026-11-23, late)

Claude's current handoff for `patch/` and the Violet Eclipse design. Paths are inside `MMBN4-Decomp/` unless they start with `/`. The ordered backlog is `TODO.md` (Gemini's file); the status of every package is in `patch/README.md`. Read both first.

## Goal & Current Status

**Goal:** *Mega Man Battle Network 4: Violet Eclipse* (`plan/DEFINITIVE-EDITION.md`): every Soul, chip and boss in one save, as a ROM patch first (`patch/`), then on the ports.

**Repo:** `main` is **32 commits ahead of `origin/main`** (GitHub `comaro-sudo/MMBN-Decomp`). The owner hasn't asked for a push; ask before pushing.

**This session's commits (Claude, all `python patch/build.py` + `--check` PASS, bytes checked in both builds):**

| Commit | What | In-game |
|---|---|---|
| `933f853` | Duo and Grand Prix Power **chip art** (`wp_d2_special_chip_art.asm`; `build.py` runs `tools/chipart.py` on the local `ref/chip-art/` PNGs; `--public` leaves it out) | Not tested |
| `c3a787f` | Lobby trash can drawn with animation 0 (Gemini round 10 found the white-circle bug) | Visual re-check pending |
| `999ec92` | **Beating Duo in the story gives Duo D** (`wp_d2_duo_story_grant.asm`: Duo's defeat handler `sub_080C4780` thunked to free space; grants once, while flag `0x006F` is still clear) | Not tested (needs a save before Duo) |
| `24786cd` | **Grand Prix Power Blue Mystery Data** in ACDC Area 2 (`wp_d2_gpp_md.asm`, flag `0x0D0B`) | Not tested |
| `a16ea98` | **Duo rematches** in Meteor Comp 1 `(82,00)` (`wp_d3_duo_rematch.asm`, `FC 0D 00`; Duo → α → β → Ω on S ranks; flags `0x0E5A`–`0x0E5B`) | Not tested |

**Gemini round 10** (`decomp/notes/task-gemini-round10.md`) is running. Task 1 is done (`decomp/notes/patch-test-results-3.md`, uncommitted, Gemini's): lobby gates, sprites, warp and rematch PASS; status two pages, Free Tournament, Frequency Tuner, Z-Saver PASS. Tasks 2–7 (names merge + regeneration, record carving, friendly-match / SP+ / Duo Soul / chip-text designs) were in progress: `decomp/asm/` and `decomp/symbols/names.txt` had uncommitted changes. **Don't touch them.**

## Key Decisions & Constraints

- Owner decisions are in `docs/pending/*.md` and `docs/completion/*.md`. Today's: Duo Soul = MetalSoul recolour granted by beating Duo V1 (`docs/pending/duo-soul.md`); Duo rematches in the Meteor Comp, climbing versions, replaying the final battle as a plain battle (`docs/completion/bosses.md`).
- **Chip art is Capcom art, local only** (`ref/` is gitignored). Never commit the PNGs or converted bins. A public release builds with `--public` (placeholder art still to decide).
- armips gotchas met today: labels are **case-insensitive** (a label `duo_defeat_state` clashed with the equ `DUO_DEFEAT_STATE`); `.macro` can't be defined inside a false `.if`; `.incbin` names are resolved even inside a false `.if` (`build.py` points them at an empty `build/chipart/<v>/none.bin`).
- Lobby dialogue archive (in `wp_d3_lobby_rematches.asm`): entries `0x00`–`0x0F` = talk index `0xF0`–`0xFF`. `0x0D` is now Duo's prompt; the shared "No" target moved to `0x0F`.
- Lobby marker byte `0x020015EF` = `0x80 | id`; id 0–11 Navis, **13 = Duo** (so `0x0E40 + id*2` gives Duo's flags `0x0E5A`). `0x0E5C` is reserved for Duo's SP+ step; `0x020015EE` for the SP+ flag (Gemini).
- Free space: next free slot **`0x087FB9C8`** (plus a 0x60 hole at `0x087FB670`); code/data must stay below `FS_TEXT` `0x087FC000`.
- Rules: stage by explicit path; never commit ROM bytes; LF line endings (the Edit tool left CRLF in `defines.asm` once: run `sed -i 's/\r$//'` if git warns).

## Modified & Relevant Files

- `patch/build.py`: `VERSIONS[..]["icon_pal"]`, `chip_art(key, public)`, `--public`.
- `patch/src/common/`: new `wp_d2_special_chip_art.asm`, `wp_d2_duo_story_grant.asm`, `wp_d3_duo_rematch.asm`; `wp_d3_lobby_rematches.asm` (FC 0D entry, Duo prompt, "No" → `0x0F`, Duo skips the secret-chip award, trash-can anim 0); `defines.asm` (`CHIP_GPP`, `FS_CHIP_ART`, `FS_DUO_GRANT`, `FS_DUO_REMATCH`).
- `patch/src/{rs,bm}/version.asm`: `DUO_DEFEAT_STATE` (RS `0x080C4780` / BM `0x080C478C`), `NPC_SLOT/LIST/COUNT_METEOR1`.
- `patch/README.md`, `docs/completion/chips.md`, `docs/completion/bosses.md`: updated.
- Uncommitted, not Claude's: `decomp/asm/*`, `decomp/symbols/names.txt`, `decomp/asm/bm/`, `decomp/notes/patch-test-results-3.md` (Gemini round 10).

## Open Issues / Blockers

- **Stale rematch marker (pre-existing, slightly wider now):** `0x020015EF` is cleared only by a *won* battle's results. After a lost or fled rematch (the Duo rematch allows running), the next ordinary win could climb that Navi's or Duo's tier on an S rank and (for Navis) award a secret chip. Needs a test (does a game over / reload clear EWRAM there?) and probably a clear at battle start or exit. Good Gemini task.
- In-game tests pending: chip art (Library, folder, pack), Duo story grant, Duo rematch (spot (212, 112), sprite `list[6] 0x63`, battle start from a static record, S-rank climb, results), trash-can sprite.
- Duo and Grand Prix Power **names/descriptions** are still blank: waits for Gemini's task 7 (`decomp/notes/chip-text-design.md`) and the TextPet one-script question.
- Owner questions open: RS/BM version choice (§7 q1), SP+ reward amount, key-item prices, Duo Soul colours/charge/passive/unite chips (Claude proposes defaults from Gemini's task 6).

## Immediate Next Steps

0. **WP-B4 built** (GMD contents follow the Frequency Tuner, `wp_b4_gmd_tier.asm`): add it to the next Gemini test round (set the Tuner to Tier 3 on a first playthrough, jack in, open a GMD, compare with the Super Hard table in `decomp/notes/re07-mystery-data.md`).

1. **Later commits (after the table above):** `8ba3c18` chip names/descriptions (TextPet `--patch`; sources in `patch/src/scripts/text/`, the only place `.gitignore` allows `*.tpl`); `146b2a8` **Duo Soul** + grant on any Duo version (`0x006F` is only set for α/β/Ω, `r5[4]` = version); `653233d` status page 3. Gemini round 10 is done and committed (`95c6e92`, `92538c0`, `adc7fdd`).
2. **Gemini round 11 is written** (`decomp/notes/task-gemini-round11.md`): an in-game test of everything above, the battle-exit clear point, the form palette hook, a verified Duo passive, the SP+ gaps, friendly matches renumbered to `FC 0E`, names. Give it to Gemini when the owner says so.
3. **SP+** (Gemini's `sp-plus-design.md`): don't build until round 11 task 2 gives a verified battle-exit clear, otherwise a lost/fled SP+ battle leaves the next battle SP+. Planned hook sites: HP at `0x08006802` (replace `ldrh r1,[r7]; adds r2,r1,#0`), attack after `bl sub_08013410` at `0x08013A5A`, veneers in the 20 dead bytes after Duo's thunk (RS `0x080C4788` / BM `0x080C4794`). Reward `0xC032` (50 BugFrags) unless the owner picks Zenny. Duo SP+ flag `0x0E5C`.
4. **Friendly matches:** after round 11 task 6 (`FC 0E`, talk indices `0xE0`+).
5. Duo palette and passive: after round 11 tasks 3–4.
6. Owner decisions (2026-11-23): GP Power's description approved; SP+ reward random: 80% 15,000 Zenny, 20% 50 BugFrags; pushed to `origin/main`. Gemini round 11 assigned.
