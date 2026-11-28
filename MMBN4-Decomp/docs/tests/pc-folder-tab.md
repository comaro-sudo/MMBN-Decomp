# PC port: ARM64 build and PET Folder tab (2026-11-27, batch 3 task 4)

**Result: PASS.**
- The ARM64 PC port builds from scratch (`python ports/pc/build_pc.py`).
- The PET Folder tab shows the right attack and element colour for every chip checked, including Duo (`0x137`) and Grand Prix Power (`0x138`).

**Setup.** The Red Sun overhaul ROM (`python patch/build.py`), windowed, with `--quit-after N --pet-tab 1 --pet-shot`. Two saves were crafted from `ref/saves/...22565.sps`, with Folder 1 rewritten (`id | code << 9`) and the checksum recomputed. The PET reads the save block straight from EWRAM, so no chip keys were needed.

**Checked:**
- **Duo and GP Power:** both show ATK 200, null element.
- **One chip of each element:** fire, aqua, elec, wood and none, plus the last table entry `#388`. All ATK values match the ROM chip table (`+0x1A`), and the element shows as the row colour.
- **Chip `#387`:** shows `--`, because its ATK in the ROM is 0.
- **Empty slots:** show `[A] --`, because code 0 prints as A. Cosmetic.

**By design:** the Folder tab shows `slot:#id[code] ATK`, never chip names, and gives the element only as a colour (`ports/common/pet_ui.c`, `draw_folder_tab`).

**Not checked:**
- the "16–30" page (it needs a mouse click);
- Blue Moon;
- loading a state with F7 in the windowed port: the tester's temporary input-script hack crashed, while the same state and input run fine headless (`--test`).

**Side effect:** the tester replaced the port's live save, `%APPDATA%\MMBN4Overhaul\PC\B4WE.sav`, during the test. It was restored afterwards and the SHA-256 is unchanged.
