@ MMBN4 disassembly — top-level ROM layout.
@ DC-0: the whole ROM is still one incbin of the player's own dump.
@ Segments are carved out of it step by step (config/segments_rs.yaml),
@ and every step must keep `make compare` matching.

    .section .rom, "ax"
    .global Start
Start:
    .incbin "baserom.gba"
