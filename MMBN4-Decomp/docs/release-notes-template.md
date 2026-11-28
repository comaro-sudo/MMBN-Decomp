# Violet Eclipse: release notes template and version numbering (draft, 2026-11-27)

Two parts: a template to copy for each release, and numbering options for the owner. The owner decides the scheme; none is picked here. Release model (Option C): two complete patches, Red Sun and Blue Moon, released together. Ports ship separately (see "Ports" at the end).

## Part 1: template

Copy the block below into the release page or `docs/release-notes-<version>.md`. Fill every `<...>`. SHA-1s come from `build/release/RELEASE-SHA1.txt` (written by `python patch/tools/release.py`). Remove lines that do not apply, but keep the headings.

````markdown
# Mega Man Battle Network 4: Violet Eclipse <version>

Released: <YYYY-MM-DD>
Release commit: <git short id> (tag `<tag>`)
Builds: public (`--public`), so the Duo and Grand Prix Power chips have names and descriptions but no picture. <Change this line if the chip-art decision changes.>

## Files

| File | For | SHA-1 |
|---|---|---|
| `bn4_rs_overhaul.bps` | Red Sun (USA) | `<sha1 of the .bps>` |
| `bn4_bm_overhaul.bps` | Blue Moon (USA) | `<sha1 of the .bps>` |

## Input ROMs (the patch refuses anything else)

| Version | ROM | SHA-1 |
|---|---|---|
| Red Sun | Mega Man Battle Network 4: Red Sun (USA) | `a97e96a7da03abd70f7953328e511b9fa29179f1` |
| Blue Moon | Mega Man Battle Network 4: Blue Moon (USA) | `5e017c803ddc768efb8010314b46bc17e9757f71` |

Expected SHA-1 of the patched result, so players can check their output:

| Version | Patched ROM SHA-1 |
|---|---|
| Red Sun | `<sha1>` |
| Blue Moon | `<sha1>` |

(This is a hash only; no ROM data is shipped.)

## What changed

Use the "Player sees" wording from `patch/README.md`. List only the packages touched since the last release; for a first release list all of them.

### Economy
- <e.g. Battle Zenny x1.5; HP Memory, Zenny NaviCust programs, Unlocker and LocEnemy at about 40% of the old price>

### Souls
- <12 Souls in both versions; Duo Soul; status pages; completion star>

### Tournaments
- <friendly matches; fixed tournament Navis; Free Tournament>

### Rematches and SP+
- <Higsby's lobby operators; trash can to JunkMan; Duo rematches; SP+>

### Chips and Mystery Data
- <random version drops; Bass both chips; Z-Saver; Grand Prix Power; Green Mystery Data follow the Tuner; Duo and GP Power text>

### Quality of life and fixes
- <Frequency Tuner; WoodMan fix; Waiting Room guard>

## Known issues

- <issue, how to notice it, workaround if any>
- Public builds: no picture for the Duo and Grand Prix Power chips.

## Upgrading and saves

- Saves from the same version (Red Sun to Red Sun) load. Shop prices sync on load.
- Back up the `.sav` first.
- <Does this release change anything a save from the previous release depends on (flags, free-space layout)? "No" or "Yes: what, and what the player must do".>
- <Moving a save between Red Sun and Blue Moon: not supported / state the rule.>
- <A save made on the patched game and opened in the original ROM: state what happens, or "untested".>

## Verification

- `python patch/build.py --public` and `--check`: <PASS/FAIL, both versions>
- Automated matrix: <n/n>
- Full playthrough: <Red Sun: who/when; Blue Moon: who/when>
- Emulators / hardware: <mGBA version; real hardware or "not tested">

## Credits and licence

See `docs/credits.md`. Patch licence: <licence; owner decision pending>. *Mega Man Battle Network* is a trademark of Capcom; this is an unofficial fan patch and contains no Capcom data.
````

### Steps for whoever fills it in

1. Build: `python patch/tools/release.py` (public build, checks, `RELEASE-SHA1.txt`, zip). Copy hashes from its output, not by hand.
2. Apply each `.bps` with Flips on a clean ROM and compare the result's SHA-1 with the table.
3. Go through `docs/release-checklist.md`; anything unchecked goes under "Known issues" or blocks the release.
4. Tag the commit and keep the notes in `docs/`.

## Part 2: version numbering options (owner decision)

The patch is two files that release together, so one number for both is simplest. Whatever is chosen, put it in the zip name (`release.py` names it `violet-eclipse-<date>.zip` today) and in the notes title. Showing the version inside the ROM (title screen or header) is a separate question and would need a patch package; today there is none.

### Option 1: semantic version (1.0.0)

`MAJOR.MINOR.PATCH`; first public release `1.0.0`.
- MAJOR: a change that makes old saves unsafe or restructures the game.
- MINOR: new content (a boss tier, a new chip location).
- PATCH: fixes only.
- Implies: a save-compatibility promise per MAJOR version, so "Upgrading and saves" must be filled in with care each time. Familiar to ROM-hack players. `1.0.0` says "complete", which fits "every Soul, chip and boss" but is a strong claim for a first public build; starting at `0.9.0` avoids it while the full playthrough test is still open.

### Option 2: date-based (2026.12.01, or 2026.12 plus a letter)

`YYYY.MM.DD`, or `YYYY.MM` with a letter for a second release in a month.
- Implies: the number makes no compatibility promise, so the notes must always say whether saves carry over. Trivial to produce: `release.py` already stamps the zip with the date. Players cannot tell a big release from a small one by the number. Suits few, irregular releases.

### Option 3: plain counter (v1, v2, v3; fixes as v1.1)

- Implies: easiest to read for non-technical players, and common in fan patches. The number carries no machine meaning, so nothing in it says what is safe. The owner must set a rule for when a change is `v1.1` and when it is `v2`. Same upgrade notes needed as the others.

### Questions that apply to all three

- Do Red Sun and Blue Moon always share a version, or may one be fixed alone? Sharing is simpler: a fix for one version bumps both, and the other is shipped unchanged with a note.
- Do the ports (PC, Wii U, 3DS) follow the patch number or have their own? They ship separately under GPL-3.0 and are at different stages. Either use the number of the patch they were tested with, or number them independently with a "tested with patch <version>" line.
- Pre-release tags (`-rc1`, `-beta`) for the first public build, if the full playthrough is not finished.

## Ports

Port zips are separate downloads. Use a shorter block of the same notes: version, platform, zip SHA-1, `NOTICE.txt` and `LICENSES/` included, tested patch version, known issues (as of 2026-11-27: 3DS never compiled or run; Wii U real-console benchmark pending).
