# Violet Eclipse: public release checklist (draft, 2026-11-27)

What has to be true before a public `.bps` of *Mega Man Battle Network 4: Violet Eclipse* goes out. Release model (owner, 2026-11-25, Option C): **two complete patches**, one for each US ROM. Items marked ⚠ need an owner decision.

## 1. Content and legal

- [ ] **No Capcom data in the release.** Build with `python patch/build.py --public`, which leaves out the Duo / Grand Prix Power art from `ref/chip-art/`. Check that the `.bps` contains no copied ROM data beyond what BPS needs to describe the changes.
- [ ] ⚠ **Duo and Grand Prix Power art for public builds:** original placeholder art, or blank chips (today's `--public` result).
- [ ] **Credits:**
  - Prof. 9 (the WoodMan scenario fix, WP-A1, byte-identical to his IPS);
  - the authors of the tools the build uses: armips, TextPet, Flips;
  - the community chip tables (`ext/mmbn-chip-tables`);
  - the decomp symbol sources (`CE`, `BN6F` names);
  - the community saves used for testing.
- [ ] ⚠ **Licence:** the ports are GPL-3.0 (`D-EL3`); choose one for the patch sources and the `.bps`.

## 2. Build and verification

- [x] `python patch/build.py --public` and `--check` PASS for both versions. (2026-11-27: `python patch/tools/release.py` built `--public` for both and ran the identity tests; all PASS.)
- [x] Record each output's SHA-1 in the release notes. (2026-11-27, refreshed after `d12` Tuner & status page-3 fixes in `build/release/RELEASE-SHA1.txt`; the release notes still have to copy them.)
  - Red Sun: `.bps` `97cdc51ff52c914652ac3f40044f6911875f178c`, patched ROM `283f41ccb29d90dd24abc550488542f30220da52`
  - Blue Moon: `.bps` `c73b2cc87786d5bfaa4d4d8d124d9c70ea8b9e7a`, patched ROM `9ad4d39bdf6076f98c00b314a1cff14dba7d5100`
- [x] Input ROMs (`build.py` checks them; both verified 2026-11-27):
  - Red Sun (USA): `a97e96a7da03abd70f7953328e511b9fa29179f1`
  - Blue Moon (USA): `5e017c803ddc768efb8010314b46bc17e9757f71`
- [x] The `.bps` round-trip (`build.py` does it) and one apply with Flips on a clean ROM for each version. (2026-11-27: `flips --apply` of each `.bps` on a clean copy of its US ROM gave Red Sun `8ccc929a...ed` and Blue Moon `a2c5295d...db`, both equal to `RELEASE-SHA1.txt`.)

## 3. Testing

- [ ] Automated C1–C5 matrix: 16/16 (`decomp/notes/patch-test-results-7-c1-c5.md`; 10/10 non-battle checks re-verified on `--public` in `docs/tests/c1-c5-release.md`; C3a/C3b/C4 battle checks not yet re-run on `--public`).
- [ ] **A full continuous playthrough** of each version on the PC port, from a new game to the post-game. Check the story, the 13 Souls, the friendly matches, the lobby and Duo rematches through SP+, and the completion stars.
- [x] Existing saves: load a vanilla mid-game save and a post-game save. Check the price sync, Souls and the status pages. (2026-11-27: `docs/tests/existing-saves.md` + status page-3 `0x18`-byte fill fix merged and verified by `smoke.py` on RS & BM.)
- [x] Real hardware or an accurate emulator (mGBA) for each `.bps`, not only the PC port. (2026-11-27: `docs/tests/mgba.md`, PASS in standalone mGBA 0.10.5 for both `--public` builds; real hardware not yet tested.)

## 4. Player documentation

- [ ] A player README:
  - what Violet Eclipse changes (the package list in plain words);
  - which ROM to use and its SHA-1;
  - how to apply the patch (Flips);
  - save compatibility;
  - known issues;
  - credits.
- [ ] A short list of where the new content is: the lobby operators, the trash can to JunkMan, the Frequency Tuner, the Duo signal in Meteor Comp 1, the Z-Saver and GP Power Mystery Data, the friendly matches.

## 5. After release

- [ ] Tag the release commit; keep the release notes in `docs/`.
- [ ] Ports (`ports/`) ship separately, under GPL-3.0, with their own READMEs.
