/* DC-5 C (agbcc): Nintendo AGB SRAM library core copy routine
 * (code_10, RS 0x0811189C-0x081118BF, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;
typedef unsigned int u32;

/* 0x0811189C: MemCopyForward (ReadSram_Core / WriteSram_Core) */
void sub_0811189C(const u8 *src, volatile u8 *dest, u32 size) {
    while (size-- != 0) {
        *dest++ = *src++;
    }
}
