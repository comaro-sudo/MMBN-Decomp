; Identity test: re-assemble the community chip table unchanged over the
; original ROM. The output must be byte-identical to the input; that proves
; the table offset and format are correct for this version.
; Run via: python patch/build.py --check

.gba
.include VERSION_FILE
.open ROM_IN, ROM_OUT, 0x08000000

.include CHIP_MACROS
.org CHIP_TABLE
.include CHIP_TABLE_FILE

.close
