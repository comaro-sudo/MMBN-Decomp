# mGBA test of both release builds (2026-11-27)

Release checklist section 3, item "Real hardware or an accurate emulator (mGBA) for each `.bps`, not only the PC port". This tests the patched ROMs in the standalone mGBA emulator, not the PC port.

**Result: PASS for both versions** on what was checked (boot, new game, an existing save, the status pages, an in-game save). Real hardware was not tested. See "Not verified".

## Setup

- **Emulator:** mGBA 0.10.5, `mGBA.exe` (Qt, win64 build from `tools/mgba/`, see `docs/reference/toolchain.md`). It ran on Windows 11 ARM64 through x64 emulation. A copy of the folder was used, so the shared `tools/mgba/` stayed untouched (it has a `portable.ini`, so mGBA writes its config next to the exe).
- **Settings:** no BIOS file (mGBA's built-in HLE BIOS), default options, 3x scale. Only `savegamePath` and `screenshotPath` were overridden with `-C`.
- **ROMs:** built with `python patch/build.py --public` in a clean worktree. Both patched ROMs match the checklist section 2:
  - Red Sun: `8ccc929a05411169c4ece2f056474bcc727f13ed` (`.bps` round-trip OK, 21383 bytes differ from the US ROM)
  - Blue Moon: `a2c5295d86fdecf18003f6ccbe018982d9de3edb` (`.bps` round-trip OK, 21391 bytes differ)
- **Saves:** community saves from `ref/saves/` (SharkPort `.sps`), converted to 32 KB `.sav` with `patch/tools/savetool.py --convert`, one copy per test, named after the ROM so mGBA picks it up:
  - Red Sun: `mega-man-battle-network-4.22565.sps`: playthrough 5, cleared, 11 Souls (all but Aqua).
  - Blue Moon: `mega-man-battle-network-4.4891.sps`: playthrough 4, cleared, 9 Souls (Roll, Search, Fire, Proto, Number, Metal, Junk, Aqua, Wood).
- **How it was driven:** the 0.10.5 binary has no command-line `--script` option (scripting is only in the GUI, Tools > Scripting). It was driven from PowerShell instead: the window is brought to the foreground, key presses are posted to it as `WM_KEYDOWN`/`WM_KEYUP` (mGBA's default keys: X = A button, Z = B, A = L, S = R, Enter = Start, arrows), and frames are captured with mGBA's own screenshot key (F12), which stores the emulated 240x160 frame. Each result below was read from those frames. Screenshots and saves stayed in a temp folder and are not committed.

## Red Sun (US) patched ROM

| Check | Result |
|---|---|
| Boots | PASS. Opening cinematic ("In the year 200X"), then the title screen, "RED SUN". |
| New game | PASS. NEW GAME starts, the intro dialogue runs through the lab scene, and the player gets control in Lan's room. |
| Existing save loads | PASS. Title shows "CONTINUE 5" and the Std / Mega / Giga / P.A. completion stars. Continue puts Lan in his room. The PET menu shows HP 1250/1250, 158070 Zenny, 3 BugFrags. |
| Status screen pages (WP-D4) | PASS. MegaMan > status. Page 1 shows 6 Souls, **R** moves to page 2 (the rest, with Aqua as the empty slot), **R** again gives page 3, **R** again wraps to page 1, **L** from page 1 goes back to page 3. Vanilla ignores R, so this shows the patch code running. |
| Save from the PET menu | PASS. "Data exists. Overwrite?" > Yes > "Save complete!", no crash. `savetool.py` reads the new `.sav` as a valid Red Sun save with the same 11 Souls, Zenny and Library counts (only the game time moved, 56:50 to 56:52). |

**Observation (known issue, not an mGBA difference):** page 3 is blank (an empty frame) with 11 Souls, because it should only exist once the 13th (Duo) Soul is owned. This is findings section 3 (`docs/pending/findings-2026-11-27.md`) and task 2 of the batch; it is reproduced here on a second emulator.

## Blue Moon (US) patched ROM

| Check | Result |
|---|---|
| Boots | PASS. Title screen, "BLUE MOON". |
| New game | PASS. NEW GAME starts, the intro dialogue plays in the lab and the player gets control in Lan's room. |
| Existing save loads | PASS. Title shows "CONTINUE 4" with the completion stars; since the save is post-game, mGBA shows the game's own "From save point / Start game 5" prompt, and "From save point" loads at Water Comp. The PET menu shows HP 1455/1455, 5190 Zenny, 10 BugFrags. |
| Status screen pages (WP-D4) | PASS. Page 1 and page 2 show the Souls (9 owned), a third **R** is page 3, blank as in Red Sun, and **R** again wraps to page 1. |
| Save from the PET menu | PASS. "Save complete!", no crash. `savetool.py` reads the new `.sav` as a valid Blue Moon save with the same 9 Souls, Zenny and Library counts. |

## Not verified

- **Real GBA hardware** and a second accurate emulator (for example a different mGBA version or the BIOS file). Only mGBA 0.10.5 with the HLE BIOS ran here.
- **Save-type detection on a clean install:** mGBA picked the 32 KB SRAM save from the converted `.sav` and also created its own for the new game. A real cartridge's Flash/SRAM behaviour (the save size of the original game) was not tested.
- **Only one patched feature per version** (the status pages) plus the save. The shops, lobby operators, Frequency Tuner, friendly matches, Duo and Grand Prix Power chips and the Z-Saver / GP Power Mystery Data were not played in mGBA. They are covered on the PC port and by the other test reports.
- **A continuous playthrough** was not done; the new game was only followed up to the first moment of player control.
- **Page-1 and page-2 icon identity per Soul** was judged by eye from 240x160 frames, not by reading memory.
- mGBA cheats, debugger and logging (to catch bad memory accesses or "game boy bug" messages) were not used, so a silent fault that does not change the screen would not show.
