/* MMBN4 — GBA cartridge ROM at 0x08000000, 8 MB. */
OUTPUT_ARCH(arm)
ENTRY(Start)

MEMORY
{
    ROM (rx) : ORIGIN = 0x08000000, LENGTH = 8M
}

SECTIONS
{
    .rom 0x08000000 :
    {
        *(.rom)
    } > ROM

    /DISCARD/ : { *(*) }
}
