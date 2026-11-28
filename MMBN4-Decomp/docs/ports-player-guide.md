# Violet Eclipse ports: player guide (draft, 2026-11-27)

For players of the PC, Wii U and 3DS ports. The ports run the game from **your own patched ROM**; none ships any Capcom data. Built from `ports/README.md`, the three port READMEs, `ports/*/src/main.c`, `ports/pc/src/config.c` and `ports/common/pet_ui.c`.

**Marks.** *(unverified)* means taken from the source code or README only, not seen running by the author of this guide. The README status lines say: PC done and release zips verified; Wii U verified in Cemu (not on a real console); 3DS **never compiled and never run**, so everything in the 3DS column is unverified.

## 1. Ports at a glance

| | PC (Windows) | Wii U | New 3DS / New 2DS XL |
|---|---|---|---|
| Status | Done; release zips verified | Verified in Cemu; real console pending | Source only, never compiled |
| Game screen | Window (integer-scaled) | TV, 6x centred | Top screen, 240x160 centred 1:1 |
| PET screen | Second window, or inside one window (Off-TV mode) | GamePad touch screen | Bottom screen, touch |
| Run file | `mmbn4.exe` | `mmbn4.wuhb` (Aroma) | `mmbn4.3dsx` |

An original 3DS or 2DS is not a target: the port needs the New 3DS 804 MHz mode.

## 2. Controls

GBA button on the left, then what you press.

| GBA | PC keyboard (default) | PC gamepad | Wii U GamePad | Wii U Pro Controller | New 3DS |
|---|---|---|---|---|---|
| D-pad | Arrow keys | D-pad, left stick (4-way) | D-pad, left stick | D-pad, left stick | D-pad |
| A | X | East button | A | A | A |
| B | Z | South button | B | B | B |
| L | A | LB | L | L | L |
| R | S | RB | R | R | R |
| Start | Enter | Start | + | + | Start |
| Select | Backspace | Back | - | - | Select |

Other keys:

| Action | PC | Wii U | 3DS |
|---|---|---|---|
| Fast forward (4x) | Hold Tab; gamepad right trigger | none found in the code (unverified) | none found in the code (unverified) |
| Save state / load state | F5 / F7 | none found in the code (unverified) | none found in the code (unverified) |
| Fullscreen | F11 | n/a | n/a |
| Show/hide the PET window | F1 | n/a (the GamePad always shows the PET) | n/a (bottom screen is always the PET) |
| Off-TV / single-window mode | F2 | ZR (GamePad or Pro Controller) | no such mode |
| Quit | Esc | Home menu / system (unverified) | Start + Select together |

Notes:
- The "Start + Select quits" rule on 3DS means those two buttons cannot be held together for any in-game purpose.
- The Wii U and 3DS "Fast forward / save state" gaps are what the source shows; they may have been left out on purpose. Treat as unverified.
- 3DS controls were never tested on hardware. The Wii U controls were checked in Cemu per the README (GamePad, Pro Controller, ZR). Wii U Wiimote and other controllers are not handled (unverified).

## 3. The PET tabs

The PET is the second screen: a panel with four tabs along the top, drawn by shared code (`ports/common/pet_ui.c`), so it looks the same on all ports. It reads the game's live memory; it does not change the game except for the Tuner and the Custom pad.

| Tab | What it shows |
|---|---|
| **Status** | Game code and version, CRC32 and frame rate (`>>` when fast-forwarding); HP bar and current area; Zenny and BugFrags; Souls owned (x / 12); the current Frequency Tuner setting; battle HP and the game's random-number state; a clickable **Tuner** bar (Auto / T1 / T2 / T3); a short key reminder for the platform. Before a save is loaded it shows only the live RAM values. |
| **Folder** | Your Folder 1 chips, 15 at a time: a button switches between chips 1-15 and 16-30. Shows chip names/attack/element read from the ROM. |
| **NetMap** | The current zone, playthrough number and Tuner setting, a small map of nodes with your position marked, and the Tuner bar again. |
| **Custom** | A "touch pad" for the Custom screen: a 5x2 grid of chip slots plus OK, A, B, L and R buttons. Touching a slot or button injects that input into the game (OK is Start). |

Using it:
- Click / touch a tab name to switch. The Tuner buttons write the same setting as the Frequency Tuner NPC in Higsby's shop, live, so it works anywhere a save is loaded.
- The PET does nothing useful with no save loaded (title screen): it says so.
- A patched ROM is expected. On an unmodified ROM the Tuner setting does not exist in the game (unverified how the PET behaves).

## 4. Off-TV and single-window modes

**PC (F2).** Normal mode is two windows: the game and a separate PET window (F1 shows or hides it). F2 switches to **Off-TV / single-window**: one window holding the game and the PET side by side, letterboxed to fit; the separate PET window is hidden and F1 does nothing. In this mode you click the PET inside the same window. F2 again goes back. The choice is saved in `mmbn4.ini` (`off_tv=1`), so the port starts in that mode next time.

**Wii U (ZR).** Normal: the game on the TV, the PET on the GamePad. ZR switches to **Off-TV**: the TV goes black and the GamePad shows the game full-size, so you can play without the TV. While Off-TV the GamePad touch does not drive the PET. ZR again goes back. The choice is not saved between runs (it starts off) (unverified: from the code, no setting is written).

**3DS.** No Off-TV mode: the top screen is always the game, the bottom screen always the PET.

## 5. Remapping keys (PC only, `mmbn4.ini`)

The PC port writes `mmbn4.ini` with the defaults on first run, in the same folder as the saves (section 6). Edit it with a text editor while the game is closed.

```
# MMBN4 Violet Eclipse - PC configuration
key_a=X
key_b=Z
key_select=Backspace
key_start=Return
key_right=Right
key_left=Left
key_up=Up
key_down=Down
key_r=S
key_l=A
key_ff=Tab
off_tv=0
```

(The values written are what SDL calls each key, so the file on your PC may show slightly different spellings of the same keys, for example `Return` for Enter; the above is the list the code produces by default.)

- Each `key_*` line takes an **SDL key name**: `X`, `Z`, `Space`, `Return`, `Tab`, `Left Shift`, `Keypad 1`, ... Names that SDL does not recognise are ignored and the default stays.
- Blank lines and lines starting with `#` or `;` are ignored.
- `off_tv` is 0 or 1.
- Only the keyboard is remappable. The gamepad buttons, and the fixed keys F1, F2, F5, F7, F11 and Esc, are not (unverified: none of these appear in the config code).
- Delete `mmbn4.ini` to get the defaults back.
- Wii U and 3DS have no remapping.

## 6. Where saves go

All ports use the same save format: a 32 KB `.sav` named after the game's code (for example `B4WE.sav` for Red Sun US), the same as an mGBA `.sav`. A patched game and the original of the same version **share** one save file, and you can copy a `.sav` between the ports and an emulator.

| Port | Save folder |
|---|---|
| PC | `%APPDATA%\MMBN4Overhaul\PC\<code>.sav` (also holds `mmbn4.ini` and the F5/F7 state file) |
| Wii U | `sd:/wiiu/apps/mmbn4/saves/<code>.sav` |
| 3DS | `sdmc:/3ds/mmbn4/saves/<code>.sav` |

The `<code>` example above (`B4WE`) is the usual US Red Sun game code; the actual name is read from your ROM's header, so check the folder for the file name (unverified for Blue Moon: expected `B4BE`).

Back up the save folder before the first run with a patched ROM. If you want a fresh game, move the `.sav` away.

## 7. Pointing the port at a patched ROM

First make the patched ROM with the patcher (see `docs/player-readme-draft.md`): apply `bn4_rs_overhaul.bps` or `bn4_bm_overhaul.bps` to your own US ROM. The port needs the `.gba`, not the `.bps`.

**PC.**
- Pass the path: `mmbn4.exe C:\path\to\bn4_rs_overhaul.gba` (you can also drag the `.gba` onto `mmbn4.exe`; unverified).
- With no argument, it looks in the working folder for `build\bn4_rs_overhaul.gba`, then `roms\bn4_rs_us.gba`. Blue Moon always needs the explicit path.
- Run from the folder you intend as the working folder; the zip builds have no ROM in them.

**Wii U.** Copy the `.gba` into `sd:/wiiu/apps/mmbn4/` next to `mmbn4.wuhb`. The app uses the first file it finds, in this order: `bn4_rs_overhaul.gba`, `bn4_bm_overhaul.gba`, `bn4_rs_us.gba`, `bn4_bm_us.gba`, `rom.gba`. So to play Blue Moon patched, **do not** leave a Red Sun file in that folder, or name the one you want `rom.gba` and remove the others (a name earlier in the list wins).

**3DS.** The same, in `sdmc:/3ds/mmbn4/`, with the same file order and the same `rom.gba` fallback.

Wrong or missing ROM: the PC port shows "No ROM found" or "Could not load the ROM"; the Wii U and 3DS builds report a missing ROM on screen/log (wording unverified).

## 8. Things to know

- **Audio on 3DS** needs the DSP firmware dump `sdmc:/3ds/dspfirm.cdc` (made with DSP1). Without it the port plays silent. Untested on hardware.
- **Frame rate target** is the GBA's 59.73 fps. If the PC port stutters, the Status tab's FPS line shows it. On the 3DS the target needs the New 3DS speed mode.
- **Benchmark mode** (Wii U / 3DS): holding L + R at launch, or an empty `bench.txt` in the app folder, runs a 600-frame speed test and writes `bench.log` instead of starting the game. Remove `bench.txt` to play normally.
- **Real hardware:** Wii U real-console speed and the whole 3DS port are not yet checked.
- The game inside is the same patched ROM: for what the patch adds and where, see the player README.

## 9. Unverified items, in one list

1. Everything for the 3DS port (never compiled or run).
2. Wii U: Home-button / quit behaviour; whether Off-TV is remembered; other controllers; error wording for a missing ROM.
3. Wii U and 3DS: no fast-forward and no save states were found in the code (could be intentional).
4. PC: drag-and-drop of a ROM onto the exe; the exact spelling SDL writes for keys in `mmbn4.ini`; the non-remappable gamepad/function keys; Blue Moon's save file name (`B4BE`).
5. PET behaviour on an unpatched ROM.
