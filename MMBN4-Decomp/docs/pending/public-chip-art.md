# Duo and Grand Prix Power: picture for `--public` builds (pending owner decision)

Written 2026-11-27. Facts and options only; the owner decides. Related: `docs/release-checklist.md` §1, `docs/completion/chips.md` §3, `patch/src/common/wp_d2_special_chip_art.asm`.

## Situation

- The picture, palette and icon of Duo (`0x137`) and Grand Prix Power (`0x138`) are in no US or JP ROM. Their records point to EWRAM event-download buffers (`0x020008A0..`), filled only by e-Reader / download events (`decomp/notes/duo-gpp-research.md`).
- Private builds point the records at Capcom's official pictures from the gitignored `ref/chip-art/`, converted by `patch/tools/chipart.py`. `--public` skips that (`CHIP_ART = 0`), so the records keep the EWRAM pointers.
- **Today's `--public` result, seen in-game 2026-11-27 (PC port, Red Sun US): no crash, no garbage; the chips have no picture and no icon.** Details in "Baseline: what `--public` shows today" below. Blue Moon was not run (same code, same pointers).
- Format to produce (from `chipart.py`): picture 56 x 48, 7 x 6 tiles, 4bpp, 0x540 bytes; palette 16 x BGR555, 0x20 bytes, index 0 = transparent; icon 16 x 16, 4bpp, 0x80 bytes, using the shared icon palette (`BN4_CHIP_ICON_PAL`). The patch reserves `FS_CHIP_ART` (`0x087FAA70`, 0xBC0 used of 0xC00).

## Baseline: what `--public` shows today (checked 2026-11-27)

**Setup.** `python patch/build.py --public` (Red Sun overhaul ROM), run in the PC port (`mmbn4.exe build/bn4_rs_overhaul.gba --test N --input ... --shot out.bmp --save run.sav`, ARM64 build) with a save made from `ref/saves/mega-man-battle-network-4.22565.sps` (via `savetool.py --convert`) plus a small script (in the session temp folder, not committed) that: sets Library flags `0x16C0+0x137` and `0x16C0+0x138`; sets the pack count byte of both chips to 1; puts Duo (`0x137`, code D) and GP Power (`0x138`, code G) in Folder 1 slots 6 and 7; and sets the two chip-key bytes the gate needs (`+0x37CC+id` = `EWRAM[0x670+id] ^ 0x64`, `+0x39D0+id` = `EWRAM[0x19A0+id] ^ 0x38`; the values are read from an `--dump` of EWRAM after a normal load) and fixes the save checksum. Gotcha for anyone repeating this: **without the chip-key bytes the whole Folder list draws blank** (the list also went blank for a normal chip put in a folder with a code the pack does not own), so a hand-edited save needs them. A blank Folder list is therefore not a Duo/GP Power symptom.

**What each screen showed** (screenshots only in the session temp folder, none committed):

| Screen | Duo (`0x137`) | Grand Prix Power (`0x138`) |
|---|---|---|
| Folder list (Chip Folder, Folder Edit) | name `Duo`, code D, `99 MB`; **icon: an empty flat frame** (no glyph) | name `GP Power`, code G, `94 MB`; **icon: empty flat frame** |
| Folder Edit card (cursor on the chip) | **picture area solid black** (all-zero picture, transparent index 0 on the black card), code D, ATK 200, text `Fist of Justice: MetKnuk!` | **picture area solid black**, code G, ATK 200, text `Speedy trio attack!` |
| Library (Standard, MegaChip, GigaChip, Secret tabs, all paged) | **not listed** | **not listed** |
| Pack side of Folder Edit | with Duo and GP Power in the folder they use up the single copy, so no pack row; with them in the pack only (hand-written pack bytes) no row appeared either, so the pack view of a legitimately obtained chip is **not established** | same |

- **No crash, no garbage tiles, no freeze** in any of these (the cursor moved normally, the other rows kept drawing, the folder menu opened). **The EWRAM buffers are not simply zero:** an EWRAM dump in the overworld after loading (before opening any menu) shows the records' pointers `0x02000E00`/`0x020008A0` (Duo) and `0x020013E0`/`0x02000E80` (GP Power) holding scratch patterns (`0x11...`, `0x99...`), and only `0x02000DE0`/`0x020013C0` zero. Those areas are shared scratch memory, so what a picture shows depends on what the game last wrote there. In the Folder and Folder Edit screens the result was still a black picture and an empty icon frame, but this is luck of the scratch contents, not a guarantee; it is a reason to prefer pointing the records at zeroed free space over leaving them as they are.
- **Library:** the two chips are not shown in any tab. The Library tab counts did not change with the Library flags set (the screens are pixel-identical to the same save without the two flags: 60/60 Mega, 5/5 Giga, 56/56 Secret). Their records have bit `0x40` of byte `+9` clear, the bit the Library uses to list a chip (`savetool.py` uses the same test). So the Library is not a place where the missing art shows; it only matters in the Folder, Folder Edit and (not checked) battle Custom screen and chip-reward screens.
- **Not checked:** the battle Custom screen and the results-screen reward picture (the chip image is loaded by kind there), Blue Moon, and the case where an e-Reader event fills the EWRAM buffers (not reachable without the e-Reader).

**What this means for option C.** Doing nothing (as-is) showed names, codes, ATK and text correctly, a black picture and an empty icon frame, and no crash, on the screens checked. But the pointers target shared EWRAM scratch that is non-zero in places, so other screens (Custom, rewards) could show leftover tiles; pointing the three pointers at zeroed free space gives the same look as seen here with no such dependence.

## Option A: original generated emblem (prototype exists)

- **Rule:** fits fully. The art is drawn by a script from nothing: no Capcom pixels, and the `.bps` carries only our own new bytes. The only ROM read at build time is the icon frame and palette, which `chipart.py` already reads for icons (frame stays in the player's ROM; the `.bps` stores only the finished icon, which is new data made from our emblem).
- **Looks like:** two original emblems, a flat 16-colour pixel-art style. Duo: two interlocked blue/red rings around a gold diamond. Grand Prix Power: a gold star on a red-and-blue roundel with a ribbon. They don't try to imitate the official art. Preview: `emblems_with_icons.png` (see "Previews").
- **Work:** done to prototype level: `patch/tools/chipart_emblem.py` (about 120 lines, Pillow only, deterministic, writes the same `<name>_image/pal/icon.bin` as `chipart.py`). Left: wire it into `build.py` as the `--public` branch of `chip_art()` (the `.asm` already takes the six `.bin` files through the same `-strequ` names, so the `.asm` needs no change); polish the art (the owner may want other motifs); seen in-game once. Small. The 16 x 16 icon is the weak point: the shared icon palette is dark, so the downscaled emblem reads as a blob (the same limit applies to the official art's icons). A hand-drawn 12 x 12 icon would be better and is about an hour of pixel work.
- **Risk:** low. Emblem ideas are ours; avoid copying the official Duo/GP Power composition when polishing.

## Option B: recolour an existing BN4 chip picture, taken from the player's ROM at build time

- **Rule:** fits if done as "repoint, don't copy". The record's picture pointer is set to an existing picture in the player's ROM (a donor chip, e.g. Z-Saver `0x0DA`, whose record `chipart.py` already reads), and only a **new 32-byte palette** is written (donor palette read from the ROM, hue-shifted by the build script). The image tiles stay in the player's ROM, so the `.bps` carries no Capcom picture bytes, only a changed palette and pointers. Copying the tiles into the free space would make the `.bps` encode them as a copy from the source ROM (BPS can do that), but repointing is cleaner and shorter. The icon can likewise point to the donor's icon (same tiles, same frame). Caveat: a recoloured Capcom picture is still a derivative of Capcom art that exists in the player's ROM; it adds no new Capcom data to the release, but it is the option with the most "is this a derivative work" argument. The owner's call.
- **Looks like:** a known chip's picture in other colours. A Duo and Grand Prix Power that show another chip's artwork, tinted, can be confusing in the Library and Folder (two chips, same picture). Two different donors are needed, so Duo and GP Power don't match each other. Preview of a hue-shifted Z-Saver was made in the temp folder only (`donor_recolour_zsaver_NOT_FOR_COMMIT.png`, derived from the ROM, not committed).
- **Work:** pick donors (a fist-like or "special" chip for Duo; a trophy-like one for GP Power) and a palette transform; add a `--public` branch to `chip_art()` that reads the donor records, and an `.asm` variant that writes only palettes and pointers (the current macro takes pointer words, so it can be reused with ROM addresses; the build must emit them as `-definelabel`s per version since donor addresses differ in RS/BM, or read them from the chip table at build time). Needs per-version testing. Medium: more build plumbing than A, and the result is less distinctive.
- **Risk:** donor pictures live at different addresses in RS and BM (the chip table offset is the same, `0x197EC`, but the pictures are read through the record's pointer so this works with a runtime read). If a later patch package moves or reuses the donor chip, the picture changes with it.

## Option C: blank (today's `--public` behaviour, if confirmed)

- **Rule:** fits trivially: nothing added.
- **Looks like:** the records keep their EWRAM pointers. If those buffers are zero, the chip has no picture and no icon (empty frame). If they are not zero at some point (e-Reader events, save-load of leftovers), they could show garbage. This is the case to check first. A safer blank is to point the three pointers at a small all-zero picture, palette and icon in free space (0xBC0 bytes of zeros; no Capcom data). That removes the dependence on EWRAM and costs the same as option A's plumbing, minus the art.
- **Work:** none (as-is) or small (pointers to zeroed free space).
- **Risk:** players see a chip with no picture; the Duo chip is the reward for Duo-Beta/Omega, so it is a visible gap in a headline feature.

## Combination

A (emblems) for `--public`, with the private build still using the official art, is the lowest-risk way to ship a finished-looking release. B and C add little over A once the generator exists. C is the fallback if the owner wants no new art at all.

## Previews

Outside the repo, in the session temp folder only: `emblems_with_icons.png` (the two emblems at 6x with the icons rendered through the RS icon palette), `duo_emblem.png`, `gpp_emblem.png`, and the NOT_FOR_COMMIT donor recolour. No ROM-derived image is committed.

## Prototype

`python patch/tools/chipart_emblem.py --preview <dir>` writes the two emblem PNGs (no ROM). `python patch/tools/chipart_emblem.py <rom.gba> <icon_pal_addr> <out_dir>` writes the six `.bin` files in `chipart.py`'s format. Not wired into `build.py`; byte layout matches (0x540 / 0x20 / 0x80). Not tested in an emulator.
