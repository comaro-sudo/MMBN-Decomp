# Duo and Grand Prix Power: picture for `--public` builds (pending owner decision)

Written 2026-11-27. Facts and options only; the owner decides. Related: `docs/release-checklist.md` §1, `docs/completion/chips.md` §3, `patch/src/common/wp_d2_special_chip_art.asm`.

## Situation

- The picture, palette and icon of Duo (`0x137`) and Grand Prix Power (`0x138`) are in no US or JP ROM. Their records point to EWRAM event-download buffers (`0x020008A0..`), filled only by e-Reader / download events (`decomp/notes/duo-gpp-research.md`).
- Private builds point the records at Capcom's official pictures from the gitignored `ref/chip-art/`, converted by `patch/tools/chipart.py`. `--public` skips that (`CHIP_ART = 0`), so the records keep the EWRAM pointers.
- **Today's `--public` result is unverified in-game.** The buffers are probably zero at boot (an empty, transparent picture), but nobody has looked in the Folder / battle screens. Whatever the choice, check that case once in mGBA: it is the baseline.
- Format to produce (from `chipart.py`): picture 56 x 48, 7 x 6 tiles, 4bpp, 0x540 bytes; palette 16 x BGR555, 0x20 bytes, index 0 = transparent; icon 16 x 16, 4bpp, 0x80 bytes, using the shared icon palette (`BN4_CHIP_ICON_PAL`). The patch reserves `FS_CHIP_ART` (`0x087FAA70`, 0xBC0 used of 0xC00).

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
