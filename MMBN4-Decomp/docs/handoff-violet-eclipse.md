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
| `a16ea98` | **Duo rematches** in Meteor Comp 1 `(82,00)` (`wp_d3_duo_rematch.asm`, `FC 0D 00`; Duo → α → β → Ω on S ranks; flags `0x0E5A`–`0x0E5B`) | Not tested |

**Gemini round 10** (`decomp/notes/task-gemini-round10.md`) is running. Task 1 is done (`decomp/notes/patch-test-results-3.md`, uncommitted, Gemini's): lobby gates, sprites, warp and rematch PASS; status two pages, Free Tournament, Frequency Tuner, Z-Saver PASS. Tasks 2–7 (names merge + regeneration, record carving, friendly-match / SP+ / Duo Soul / chip-text designs) were in progress: `decomp/asm/` and `decomp/symbols/names.txt` had uncommitted changes. **Don't touch them.**

## Key Decisions & Constraints

- Owner decisions are in `docs/pending/*.md` and `docs/completion/*.md`. Today's: Duo Soul = MetalSoul recolour granted by beating Duo V1 (`docs/pending/duo-soul.md`); Duo rematches in the Meteor Comp, climbing versions, replaying the final battle as a plain battle (`docs/completion/bosses.md`).
- **Chip art is Capcom art, local only** (`ref/` is gitignored). Never commit the PNGs or converted bins. A public release builds with `--public` (placeholder art still to decide).
- armips gotchas met today: labels are **case-insensitive** (a label `duo_defeat_state` clashed with the equ `DUO_DEFEAT_STATE`); `.macro` can't be defined inside a false `.if`; `.incbin` names are resolved even inside a false `.if` (`build.py` points them at an empty `build/chipart/<v>/none.bin`).
- Lobby dialogue archive (in `wp_d3_lobby_rematches.asm`): entries `0x00`–`0x0F` = talk index `0xF0`–`0xFF`. `0x0D` is now Duo's prompt; the shared "No" target moved to `0x0F`.
- Lobby marker byte `0x020015EF` = `0x80 | id`; id 0–11 Navis, **13 = Duo** (so `0x0E40 + id*2` gives Duo's flags `0x0E5A`). `0x0E5C` is reserved for Duo's SP+ step; `0x020015EE` for the SP+ flag (Gemini).
- Free space: next free slot **`0x087FB7D0`**; code/data must stay below `FS_TEXT` `0x087FC000`.
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
- Grand Prix Power's Blue Mystery Data in ACDC Area 2 (106, 326), flag `0x0D0B`: not built yet.
- Owner questions open: RS/BM version choice (§7 q1), SP+ reward amount, key-item prices, Duo Soul colours/charge/passive/unite chips (Claude proposes defaults from Gemini's task 6).

## Immediate Next Steps

1. When Gemini's round 10 lands: read its handoff and the four design notes; check that its regeneration compares OK.
2. Build from those designs, in order: **Duo Soul** (`duo-soul-design.md`; add the grant to `wp_d2_duo_story_grant.asm` and to Duo V1 rematch wins), **chip names/descriptions** (`chip-text-design.md`), **SP+** (`sp-plus-design.md`; Duo's step uses `0x0E5C`), **friendly matches** (`friendly-matches-design.md`, TextPet dialogue + `FC 0C`).
3. Build Grand Prix Power's Blue Mystery Data (ACDC Area 2, flag `0x0D0B`), like `wp_d2_zsaver_md.asm`.
4. Give Gemini a test round: chip art, Duo story grant, Duo rematch, trash can, and the stale-marker case above.
5. Ask the owner whether to push.
