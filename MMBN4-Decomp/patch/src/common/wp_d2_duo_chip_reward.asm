; ============================================================
; WP-D2: the Duo Giga chip comes from beating Duo at his highest tier (owner decision 2026-11-23,
; docs/completion/chips.md section 3). Duo has no in-game source in either version (e-Reader, Japan only).
;
; Duo's drop rows (BN4_REWARD_ROWS, RE-06) give 5000 Zenny in all 20 slots, for every tier, in both
; versions. Here the two highest tiers, Duo-beta (0xE2, the playthrough-3 Duo) and Duo-omega (0xE3), drop
; the Duo chip (0x137, code D) in every slot, so any win against them gives it.
; Both tables are changed: this ROM's, and the other version's copy that the random-drop package
; (wp_d2_random_version_drops.asm) reads half the time. So this file must come after that one in main.asm.
; (T: in-game check pending that the Duo battles reach the results screen.)
; ============================================================

DUO_CHIP_REWARD equ CHIP_DUO | (3 << 9)        ; reward word: chip, code D (3)
ENEMY_DUO_BETA  equ 0xE2
ENEMY_DUO_OMEGA equ 0xE3

.macro duo_rows, table
    .org (table) + ENEMY_DUO_BETA * 0x28         ; rows 0xE2 and 0xE3 are contiguous: 2 x 20 halfwords
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
    .halfword DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD, DUO_CHIP_REWARD
.endmacro

duo_rows REWARD_ROWS                ; this ROM's table: rows 0xE2 and 0xE3 (40 halfwords, contiguous)
duo_rows FS_OTHER_REWARD_ROWS       ; the other version's copy (random drops)
