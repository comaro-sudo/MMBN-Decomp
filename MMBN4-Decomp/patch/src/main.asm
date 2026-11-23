; ============================================================
; Violet Eclipse (MMBN4) — main entry point
; Assembled by build.py with:
;   -strequ ROM_IN  <original rom>   -strequ ROM_OUT <output rom>
;   -strequ VERSION_FILE patch/src/bm/version.asm | patch/src/rs/version.asm
;   -definelabel HELLO 0|1
; ============================================================

.gba
.thumb

.include VERSION_FILE           ; per-version labels (VER_BM / VER_RS, offsets)
.include "common/defines.asm"   ; version-agnostic constants

.open ROM_IN, ROM_OUT, 0x08000000

; ---- Test patches -------------------------------------------
.if HELLO
    .include "test/hello.asm"
.endif

; ---- Overhaul work packages (added per milestone) -----------
.include "common/wp_a1_woodman_fix.asm"      ; WP-A1: WoodMan scenario fix (Prof. 9)
.include "common/wp_a2_zenny_battle.asm"      ; WP-A2: battle Zenny x1.5
.include "common/wp_d4_twelve_souls.asm"     ; Violet Eclipse: all 12 Souls unite (both versions)
.include "common/wp_d3_bass_both_chips.asm"  ; Violet Eclipse: a Bass battle gives both versions' chips
.include "common/wp_d4_star_twelve_souls.asm" ; Violet Eclipse: the Souls completion star needs all 12
.include "common/wp_d2_random_version_drops.asm" ; Violet Eclipse: version-specific drops at random, 50/50
.include "common/wp_d2_duo_chip_reward.asm"  ; Violet Eclipse: Duo-beta/omega drop the Duo chip (after random drops)
.include "common/wp_d4_status_two_pages.asm" ; Violet Eclipse: status screen shows 12 Souls on two pages (L/R)
.include "common/wp_d0_npc_lists.asm"        ; infrastructure: Higsby's and ACDC Town NPC lists in free space
.include "common/wp_d2_zsaver_md.asm"        ; Violet Eclipse: Z-Saver Z Blue Mystery Data in Undernet 1
.include "common/wp_b1_tournaments.asm"      ; WP-B1: deterministic story tournaments & canon Soul Navi pairs
.include "common/wp_d4_free_tournament.asm"  ; Violet Eclipse: Free Tournament 36-secret-chip fix

.close
