# Patch Test Results 5 (Round 12 Task 1, 2026-11-23)

Tested commit `4f5ec2b` (`bn4_rs_overhaul.gba`, SHA-1 `8a3584c683a55c9cd919d6a675fbb6321dd0891a`; `python patch/build.py --check` PASS for both RS and BM) in `mmbn4.exe`. All screenshots, savestates, and EWRAM dumps are in `scratch/r12/` outside the repo.

## Summary Table

| Item | Result | Key Evidence |
|---|---|---|
| **Duo / GP Power text** (`0x137`/`0x138`, threshold `0x1000`) | **PARTIAL PASS + 2 FINDINGS** | `GP Power` name & description `"Speedy trio attack!"` render cleanly in Folder Edit (`r12_fd_703.bmp`), and all 13 Soul names (`0x161`–`0x16D`) are byte-identical. **Finding 1:** `0x137`/`0x138` have chip-table `flags = 0x07` (bit 6 `0x40` = 0, so they are excluded from the Library) and hovering `0x137`/`0x138` in Folder Edit blanks the display to `0/30` 1 frame later (`r12_fd_704.bmp`, also reproduces on vanilla `bn4_rs_us.gba` `r12_van_down.bmp`). **Finding 2:** `chip_descs_1.tpl` compiles `e6 01 e5` without the trailing `e7 03` that vanilla chip descriptions carry (cosmetic/unrelated to the `0/30` wipe, which still occurred when `e7 03` was patched in `r12_test_e703.bmp`). |
| **Sprites (Trash Can & Duo's Signal)** | **PASS** | Both draw the glowing blue crystal (`list[7] 0x08`) cleanly with zero white circles (`r12_trashcan.bmp`, `r12_duo_signal.bmp`). |
| **Duo Rematch (V1, β, Ω)** | **FAIL (still skips results screen)** | Even with `b1 = 0x00`, killing 1-HP Duo V1 (2000 HP), Duo β (3000 HP), or Duo Ω (3500 HP) plays Duo's white-flash deletion (`r12_dkill_380.bmp`–`580.bmp`) and returns straight to Meteor Comp 1 (`r12_dkill_680.bmp`) with **no results screen**. `BattleExitCleanup` ran and cleared `0x020015EF` (`0x8D -> 0x00`), but `0x0023`, `0x0E72`, `0x0E5A`, and `0x0E5B` all remained `False` and Pack did not receive Duo D. (Root cause investigated in Task 3.) |
| **Battle-Exit Clear (`wp_d3_battle_exit_clear.asm`)** | **PASS** | Entered Dex lobby rematch (`0x020015EC..0x020015EF = 00 00 00 80`), lost at 1 HP (`r12_lob_lost_real.bmp`), returned to lobby with `0x020015EC..0x020015EF = 00 00 00 00`. Also cleared `0x8D -> 0x00` on Duo rematch exit (`r12_dkill_680.ewram`). Subsequent random battle S-rank sees `0x020015EF == 0`, so no tier change can occur. |
| **WP-B4 (GMD contents follow Frequency Tuner)** | **PASS** | On PT1 (`4440.sav`, playthrough 0), opened ACDC Area 1 GMD #1 (`0x0D02`) at `(-174, 136, 0)`: with Tuner = **Tier 3** (`0x0E58=1, 0x0E59=1`), rolled `"The mystery data was a [virus!]"` (`r12_wpb4_t3_got2.bmp` — in `re07-mystery-data.md`, **only** Super Hard has virus-trapped entries for `0x0D02`, Normal and Hard have 0); with Tuner = **Auto** (`0x0E58=0, 0x0E59=0`), rolled `"MegaMan got a BattleChip: Recov10 *!"` (`r12_wpb4_auto_name.bmp`, exact Normal-tier entry). |

## Detailed Findings & Bugs for Claude

### 1. Duo & GP Power in Folder Edit / Library (`0x137`/`0x138`)
- **Text rendering:** Confirmed `CHIP_NAME_EREADER_MIN` (`0x0802F1BC`) and `CHIP_DESC_EREADER_MIN` (`0x08106C28`) are `0x00001000`. In `r12_fd_703.bmp`, `GP Power` draws its name, icon, card art, and `"Speedy trio attack!"` cleanly. All 13 Soul names (`0x161`–`0x16D`) in the patched Bank 1 archive (`0x087FC000`) are byte-identical to vanilla `bn4_rs_us.gba`.
- **Library exclusion:** In the chip table (`0x080197EC`), compare `Z-Saver` (`0x136`) vs `Duo` (`0x137`) and `GP Power` (`0x138`):
  - `0x136` (`Z-Saver`): `lib = 2`, `flags = 0x43` (bit 6 `0x40` set = shown in Library).
  - `0x137` (`Duo`): `lib = 2`, `flags = 0x07` (bit 6 `0x40` clear = hidden from Library; bit 2 `0x04` set = e-Reader chip).
  - `0x138` (`GP Power`): `lib = 1`, `flags = 0x07` (bit 6 `0x40` clear = hidden from Library; bit 2 `0x04` set = e-Reader chip).
  - Setting Library flags `0x16C0 + 0x137` and `0x16C0 + 0x138` does not add them to the Secret Library (`r12_clean_lib_duo.bmp` still shows `5` chips) because bit 6 (`0x40`) of `flags` is 0.
- **Folder Edit `0/30` wipe (1 frame after highlighting `0x137`/`0x138`):** On `chips.sav` (slot 0 = `AreaGrab *`, slot 1 = `GP Power G`), pressing `DOWN` onto `GP Power` draws `"Speedy trio attack!"` on frame 703 (`r12_fd_703.bmp`), then on frame 704 (`r12_fd_704.bmp`) wipes the Folder Edit list to `0/30` (yellow box). This also reproduces on **vanilla `bn4_rs_us.gba`** (`r12_van_down.bmp`) and on `lob_chips.sav` (clean `lob.sav` with `Duo D` and `GP Power G` placed in Folder 1). Because it happens on vanilla US ROM as well, it is likely vanilla US e-Reader chip handling (`flags & 0x04` or chip ID `0x137`/`0x138`) in the Folder Edit code, not `wp_d2_ereader_chips.asm`.

### 2. Duo Rematch Still Skips Results Screen (`b1 = 0x00`)
- Entering Duo V1 (2000 HP), Duo β (3000 HP), and Duo Ω (3500 HP) from Duo's Signal all set `0x020015EF = 0x8D` and launch the right Duo version (`r12_d9_beta_bat.bmp` shows `Duoβ 3000`, `r12_d9_omega_bat.bmp` shows `DuoΩ 3500`).
- Killing 1-HP Duo V1 plays Duo's white-flash deletion (`r12_dkill_380.bmp`–`580.bmp`) and returns directly to Meteor Comp 1 (`r12_dkill_680.bmp`) with **no results screen**.
- `BattleExitCleanup` ran on exit and cleared `0x020015EF` (`0x8D -> 0x00`), confirming `wp_d3_battle_exit_clear.asm` works even on Duo's exit path.
- Because the results screen never ran (and `duo_defeat_hook` did not fire — see Task 3), `0x0023`, `0x0E72`, `0x0E5A`, and `0x0E5B` all stayed `False`. Duo's own deletion routine (`sub_08010D4C` or battle-record `b6`) forces the final-boss no-results exit regardless of `b1 = 0x00`.
