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
; next free slot:             0x087FA0A8

PLAY_SFX            equ 0x08000560      ; PlaySoundEffect(r0); same in RS and BM
VENEER_INIT_PAGE    equ 0x08000DB4      ; the old BN4_STAR_SOUL_FLAGS (14 bytes, unreferenced since WP-D4 star)

; GetPositiveSignedRNG: r0 = random 0..0x7FFFFFFF (uses r0-r3); same address in RS and BM
GET_RNG             equ 0x08001B16

; Address of a field of a chip entry
.macro chip_field_org, id, field
    .org CHIP_TABLE + (id) * CHIP_ENTRY_SIZE + (field)
.endmacro
