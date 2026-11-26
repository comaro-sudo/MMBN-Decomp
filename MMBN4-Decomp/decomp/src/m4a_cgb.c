/* DC-5 C (old_agbcc -O2): Nintendo m4a CGB oscillator off
 * (code_10, RS 0x081132C4-0x08113313, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;

#define REG_NR12 (*(volatile u8 *)0x04000063)
#define REG_NR14 (*(volatile u8 *)0x04000065)
#define REG_NR22 (*(volatile u8 *)0x04000069)
#define REG_NR24 (*(volatile u8 *)0x0400006D)
#define REG_NR30 (*(volatile u8 *)0x04000070)
#define REG_NR42 (*(volatile u8 *)0x04000079)
#define REG_NR44 (*(volatile u8 *)0x0400007D)

/* 0x081132C4: CgbOscOff */
void sub_081132C4(u8 ch) {
    switch (ch) {
    case 1:
        REG_NR12 = 8;
        REG_NR14 = 0x80;
        break;
    case 2:
        REG_NR22 = 8;
        REG_NR24 = 0x80;
        break;
    case 3:
        REG_NR30 = 0;
        break;
    default:
        REG_NR42 = 8;
        REG_NR44 = 0x80;
        break;
    }
}
