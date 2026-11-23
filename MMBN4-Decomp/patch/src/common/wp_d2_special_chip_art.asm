; ============================================================
; WP-D2: chip art for Duo (0x137) and Grand Prix Power (0x138).
; Their records point the picture, palette and icon at EWRAM event-download buffers (0x020008A0.., e-Reader
; only); no US or JP ROM has their art (decomp/notes/duo-gpp-research.md). The owner supplied the official
; 56 x 48 pictures, kept local in ref/chip-art/ (gitignored); build.py converts them with
; patch/tools/chipart.py (icons made from the pictures on the standard icon frame) and passes the files in.
; With --public, or without the PNGs, CHIP_ART = 0 and nothing here is assembled.
; Chip record: 0x20 data bytes, then +0x20 icon, +0x24 picture, +0x28 palette pointers (same table in RS/BM).
; ============================================================

.macro chip_art_ptrs, id, icon, image, pal
    .org CHIP_TABLE + (id) * CHIP_ENTRY_SIZE + 0x20
    .word icon, image, pal
.endmacro

.if CHIP_ART

chip_art_ptrs CHIP_DUO, FS_CHIP_ART + 0x540 * 2 + 0x20 * 2, FS_CHIP_ART, FS_CHIP_ART + 0x540 * 2
chip_art_ptrs CHIP_GPP, FS_CHIP_ART + 0x540 * 2 + 0x20 * 2 + 0x80, FS_CHIP_ART + 0x540, FS_CHIP_ART + 0x540 * 2 + 0x20

.org FS_CHIP_ART
    .incbin DUO_IMAGE_BIN           ; +0x000: Duo picture       (0x540)
    .incbin GPP_IMAGE_BIN           ; +0x540: GP Power picture  (0x540)
    .incbin DUO_PAL_BIN             ; +0xA80: Duo palette       (0x20)
    .incbin GPP_PAL_BIN             ; +0xAA0: GP Power palette  (0x20)
    .incbin DUO_ICON_BIN            ; +0xAC0: Duo icon          (0x80)
    .incbin GPP_ICON_BIN            ; +0xB40: GP Power icon     (0x80)
.if . != FS_CHIP_ART + 0xBC0
    .error "WP-D2 chip art: unexpected size (expected 0xBC0)"
.endif

.endif
