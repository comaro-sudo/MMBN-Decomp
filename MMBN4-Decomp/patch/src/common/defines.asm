; Version-agnostic constants.
; Version-specific addresses live in patch/src/bm/version.asm and patch/src/rs/version.asm.

; Chip table: entries of 0x20 data bytes + 0x0C (icon/image/palette pointers)
CHIP_ENTRY_SIZE     equ 0x2C
CHIP_OFS_DAMAGE     equ 0x1A        ; halfword

; Chip IDs (see ext/mmbn-chip-tables/bn4b_chip_table.asm)
CHIP_CANNON         equ 0x0001
CHIP_HICANNON       equ 0x0002
CHIP_MCANNON        equ 0x0003

; The four Bass Giga chips (decomp/notes/bass-rematch.md, Gemini)
CHIP_BASS           equ 301
CHIP_DELTARAY       equ 302
CHIP_BASSANLY       equ 306
CHIP_BLAKBARR       equ 308
CHIP_DUO            equ 0x137       ; Duo (Giga), code D only
CHIP_GPP            equ 0x138       ; Grand Prix Power (PrixPowr, Mega), code G only

; Give a chip: r0 id, r1 code, r2 quantity (pack + Library flag + chip key). code_01, same address in RS and BM
CHIP_GIVE           equ 0x0801913E

; The literal GetCompletionStars (0x08000D28) loads for BN4_STAR_SOUL_FLAGS; same address in RS and BM
STAR_SOUL_FLAGS_PTR equ 0x08000DB0
STAR_SOUL_COUNT_CMP equ 0x08000D56      ; cmp r5, #6 (owned Souls must equal the list's count); same in RS and BM

; ---- Free space: the unused 0xFF tail, 0x087F7400-0x08800000, same in RS and BM (docs/re/free-space.md).
; Allocate one slot per work package here, in address order, so nothing overlaps.
FREE_SPACE_START        equ 0x087F7400
FS_STAR_SOUL_FLAGS      equ 0x087F7400      ; WP-D4 star: 13 u16 = 26 bytes
FS_STAR_SOUL_FLAGS_SIZE equ 0x20
FS_OTHER_REWARD_ROWS      equ 0x087F7420    ; WP-D2 random drops: the other version's 276 x 40 drop rows
FS_OTHER_REWARD_ROWS_SIZE equ 0x2B20
FS_STATUS_SOULS           equ 0x087F9F40    ; WP-D4 status: 12-Soul flags (+0), icon anims (+0x10), icon pals (+0x20)
FS_STATUS_CODE            equ 0x087F9F70    ; WP-D4 status: InitPage + PageToggle
FS_STATUS_CODE_SIZE       equ 0xA0
FS_NPC_HIGSBY             equ 0x087FA010    ; WP-D0: Higsby's NPC list (max 16 + 0xFF)
FS_NPC_ACDC               equ 0x087FA058    ; WP-D0: ACDC Town's NPC list (max 16 + 0xFF)
FS_NPC_LIST_SIZE          equ 0x48
FS_STATUS_OAM             equ 0x087FA0A0    ; WP-D4 status: 6-byte OAM slot offsets (moved off dat_0810DABF)
FS_ZSAVER_MD              equ 0x087FA0A8    ; WP-D2 Z-Saver BMD: Undernet 1 MD list + CW NPC list + spawn script
FS_MD_UNDER1_LIST         equ 0x087FA0A8    ;   4 existing + 1 new BMD + place + cont (0x60 B)
FS_CW_NPC_UNDER1          equ 0x087FA108    ;   6 existing + 1 new spawn ptr + 11 B script (0x38 B)
FS_ZSAVER_MD_SIZE         equ 0x98
FS_FREE_TOURN_FILL        equ 0x087FA140    ; WP-D4 Free Tournament: missing-chips-first selector + case 0 spawner
FS_FREE_TOURN_SIZE        equ 0xF0
FS_LOBBY_REMATCH          equ 0x087FA230    ; WP-D3 Lobby Rematches + JunkMan Trash Can (code, NPCs, dialogue, FC table)
FS_LOBBY_REMATCH_SIZE     equ 0x700
FS_FREQ_TUNER             equ 0x087FA930    ; WP-B2 Frequency Tuner (BN4_ID_RAISE hook + FC 0B cycler + NPC)
FS_FREQ_TUNER_SIZE        equ 0x100
FS_GRANT_SOUL             equ 0x087FAA30    ; WP-D4: FC 0C <idx> grant-Soul handler
FS_GRANT_SOUL_SIZE        equ 0x40
FS_CHIP_ART               equ 0x087FAA70    ; WP-D2: Duo/GP Power pictures, palettes, icons (0xBC0; private builds only)
FS_CHIP_ART_SIZE          equ 0xC00
; 0x087FB670-0x087FB6B0 free (0x40): remainder of the first Duo grant slot
FS_DUO_REMATCH            equ 0x087FB6B0    ; WP-D3 Duo rematch (FC 0D, 4 battle records, Meteor Comp 1 NPC list + NPC)
FS_DUO_REMATCH_SIZE       equ 0x120
FS_GPP_MD                 equ 0x087FB7D0    ; WP-D2 GP Power BMD: ACDC Area 2 MD list + CW NPC list + spawn script
FS_MD_ACDC2_LIST          equ 0x087FB7D0    ;   3 existing + 1 new BMD + place + cont (0x60 B)
FS_CW_NPC_ACDC2           equ 0x087FB830    ;   13 existing + 1 new spawn ptr + 11 B script (0x48 B)
FS_GPP_MD_SIZE            equ 0xA8
FS_DUO_SOUL               equ 0x087FB878    ; WP-D4 Duo Soul: 13-entry unite tables, form-sprite redirect, charge shot
FS_DUO_SOUL_SIZE          equ 0x60
FS_DUO_GRANT              equ 0x087FB8D8    ; WP-D2/D4: Duo deletion handler (Duo Soul + Duo D + flag 0x006F)
FS_DUO_GRANT_SIZE         equ 0x90
FS_GMD_TIER               equ 0x087FB968    ; WP-B4: GMD content block follows the Frequency Tuner
FS_GMD_TIER_SIZE          equ 0x60
FS_BATTLE_EXIT            equ 0x087FB9C8    ; WP-D3: zero 0x020015EC-EF on every battle exit
FS_BATTLE_EXIT_SIZE       equ 0x18
FS_DUO_EXT                equ 0x087FB9E0    ; WP-D4 Duo Soul: face fix, Duo palette hook + 3 palettes, passive (Air/Float + Reflect)
FS_DUO_EXT_SIZE           equ 0xF0
FS_SP_PLUS                equ 0x087FBAD0    ; WP-D3 SP+: 4,095 HP, ATK x1.5, Speed x1.25, 80/20 reward, roaming SP scan
FS_SP_PLUS_SIZE           equ 0x1A0
FS_FRIENDLY               equ 0x087FBC70    ; WP-B1 Friendly Matches: FC 0E, 3 venue NPC lists, 9 NPCs, dialogue archive
FS_FRIENDLY_SIZE          equ 0x500
FS_WAITING_ROOM_GUARD     equ 0x087FC170    ; WP-B6 Guard A: WaitingRoomReceiveTradedNavi slot bounds check (1 <= r6 <= 7)
FS_WAITING_ROOM_GUARD_SIZE equ 0x30
FS_DUO_RANK               equ 0x087FC1A0    ; WP-D3 Duo rematch S-rank tier climb (0x08007692 -> V1->a->b->o->SP+)
FS_DUO_RANK_SIZE          equ 0xB0
; next free slot:             0x087FC250 (code/data packages must stay below FS_TEXT)
FS_TEXT                   equ 0x087FD000    ; TextPet --free-space for new/grown text archives (build.py reads this line)
FS_TEXT_SIZE              equ 0x3000        ; up to the ROM end, 0x08800000

WRITE_CHIP_KEY      equ 0x080063B4      ; WriteChipKey(r0 chip id): the anti-cheat key, version XOR applied; same in RS and BM

BN4_GET_CHIP        equ 0x080190D0      ; BN4_GET_CHIP(r0 chip_id) -> r0 ptr to chip entry; same in RS and BM
INIT_BATTLE_REC     equ 0x08007C90      ; sub_08007C90; same in RS and BM
SET_BATTLE_PARAMS   equ 0x08007C74      ; sub_08007C74; same in RS and BM
BATTLE_ADD_ACTOR    equ 0x08007CB0      ; BattleAddActor(slot, packed); same in RS and BM
START_OW_BATTLE     equ 0x08005544      ; sub_08005544(r0 rec, r1=1, r2=0xFF, r3=1); same in RS and BM
SET_MODE_FLAGS      equ 0x08005AFC      ; sub_08005AFC(r0, r1); same in RS and BM
WARP_BY_TABLE       equ 0x08005664      ; where_bn4_plays.warp_2(r0 table, r1 idx); same in RS and BM

PLAY_SFX            equ 0x08000560      ; PlaySoundEffect(r0); same in RS and BM
VENEER_INIT_PAGE    equ 0x08000DB4      ; the old BN4_STAR_SOUL_FLAGS (14 bytes, unreferenced since WP-D4 star)

; Contiguous 6-slot veneer pool in verified dead function sub_0800A144 (54 bytes, RS == BM; decomp/notes/veneer-space.md)
VENEER_BATTLE_EXIT  equ 0x0800A144      ; WP-D3 battle-exit clear (moved off Bass's 0x080C4788)
VENEER_SP_PLUS_1    equ 0x0800A14C      ; WP-D3 SP+ HP/roam scan (0x08006806)
VENEER_SP_PLUS_2    equ 0x0800A154      ; WP-D3 SP+ ATK x1.5 (0x08013A06)
VENEER_SP_PLUS_3    equ 0x0800A15C      ; WP-D3 SP+ Speed x1.25 (0x08003196)
VENEER_DUO_PAL      equ 0x0800A164      ; WP-D4 DuoSoul palette (0x080107E2)
VENEER_SP_REWARD    equ 0x0800A16C      ; WP-D3 SP+ 80/20 reward (SP_REWARD_HOOK)
; Second veneer pool in verified dead code_00 function sub_08001784 (36 bytes = 4 slots, RS == BM; veneer-space.md)
VENEER_DUO_RANK     equ 0x08001784      ; WP-D3 Duo rematch S-rank climb (0x08007692)

; GetPositiveSignedRNG: r0 = random 0..0x7FFFFFFF (uses r0-r3); same address in RS and BM
GET_RNG             equ 0x08001B16

; Address of a field of a chip entry
.macro chip_field_org, id, field
    .org CHIP_TABLE + (id) * CHIP_ENTRY_SIZE + (field)
.endmacro
