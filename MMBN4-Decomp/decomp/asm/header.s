@ GBA cartridge header (0x08000000-0x080000BF).
@ Entry branch is code; the Nintendo logo and header fields stay incbin from
@ the player's ROM (the logo is Nintendo's data and is never committed).

    .arm
    .global Start
Start:
    b       sub_080000C0                    @ 0x08000000 -> crt0 (asm/crt0.s)

    .incbin "baserom.gba", 0x04, 0xBC       @ logo, title, game code, maker, checksum...
