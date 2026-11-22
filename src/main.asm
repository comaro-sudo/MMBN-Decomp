; ============================================================
; MMBN4 Overhaul — main entry point
; Assembled by build.py with:
;   -strequ ROM_IN  <original rom>   -strequ ROM_OUT <output rom>
;   -strequ VERSION_FILE src/bm/version.asm | src/rs/version.asm
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
; .include "common/wp_a1_woodman_fix.asm"   ; M2

.close
