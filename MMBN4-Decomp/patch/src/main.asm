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
.include "common/wp_d3_lobby_rematches.asm"  ; Violet Eclipse: 11 lobby operators + JunkMan trash can rematches
.include "common/wp_b2_frequency_tuner.asm"  ; WP-B2: Frequency Tuner (V1/V2/V3/Auto virus tier override)
.include "common/wp_b4_gmd_tier.asm"         ; WP-B4: GMD contents follow the Frequency Tuner
.include "common/wp_d2_ereader_chips.asm"    ; Duo / GP Power text from the archives
.include "common/wp_d3_battle_exit_clear.asm" ; clear per-battle bytes on every battle exit
.include "common/wp_d4_grant_soul.asm"       ; Violet Eclipse: script command FC 0C <idx> grants any Soul
.include "common/wp_d2_special_chip_art.asm" ; Duo / GP Power chip art (private builds; build.py --public leaves it out)
.include "common/wp_d2_duo_story_grant.asm"  ; beating Duo in the story gives Duo D
.include "common/wp_d3_duo_rematch.asm"     ; Duo rematches in Meteor Comp 1 (post-game), V1 -> alpha -> beta -> omega
.include "common/wp_d2_gpp_md.asm"           ; Grand Prix Power G in a Blue Mystery Data, ACDC Area 2
.include "common/wp_d4_duo_soul.asm"         ; Duo Soul (13th): unite, MetalSoul sprite, palette, passive, charge shot, icon
.include "common/wp_d3_sp_plus.asm"          ; WP-D3: SP+ boss difficulty (4,095 HP, ATK x1.5, Speed x1.25, 80/20 reward)
.include "common/wp_b1_friendly_matches.asm" ; WP-B1: 9 friendly tournament matches (3 per tournament, FC 0E)
.include "common/wp_a3_key_item_prices.asm"  ; WP-A3: key-item/upgrade shop prices at 40% (Option 2)
.include "common/wp_b6_waiting_room_guard.asm" ; WP-B6 Guard A: WaitingRoomReceiveTradedNavi slot bounds check (1..7)

.close
