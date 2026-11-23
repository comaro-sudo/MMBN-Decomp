; ============================================================
; WP-D2: Duo (0x137) and Grand Prix Power (0x138) names and descriptions come from the chip text archives.
; Both chip text routines send every chip id >= 0x137 to a 2-entry table of e-Reader RAM text
; (Gemini, decomp/notes/patch-test-results-4.md): names sub_0802F154, descriptions sub_08106BF0.
; Raising the threshold above every chip id (the last is 0x184) sends 0x137/0x138 to bank 1
; (script = id & 0xFF), where our patched text is (patch/src/scripts/text/). Gemini proposed 0x139, but
; then ids above 0x138 would index the 2-entry table out of range; with 0x1000 no chip takes that path.
; ============================================================

.org CHIP_NAME_EREADER_MIN          ; literal: first e-Reader chip id, names
    .word   0x1000
.org CHIP_DESC_EREADER_MIN          ; literal: first e-Reader chip id, descriptions
    .word   0x1000
