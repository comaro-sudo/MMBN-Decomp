/* DC-5 C (agbcc -O1): Complete Nintendo AGB SRAM library
 * (code_10, RS 0x0811189C-0x08111A2F, BM +0x14).
 * Compiled with pret/agbcc -O1 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define REG_WAITCNT (*(volatile u16 *)0x04000204)

/* 0x0811189C: ReadSram_Core (MemCopyForward) */
void sub_0811189C(const u8 *src, volatile u8 *dest, u32 size) {
    while (size-- != 0) {
        *dest++ = *src++;
    }
}

/* 0x081118C0: ReadSram */
void sub_081118C0(const u8 *src, u8 *dest, u32 size) {
    u16 readSram_Work[0x40];
    u16 *src16;
    u16 *dest16;
    u16 size16;
    void (*readSramCore)(const u8 *, u8 *, u32);

    REG_WAITCNT = (REG_WAITCNT & 0xFFFC) | 3;

    src16 = (u16 *)sub_0811189C;
    src16 = (u16 *)((u32)src16 & ~1);
    dest16 = readSram_Work;
    size16 = ((u32)sub_081118C0 - (u32)sub_0811189C) / 2;
    while (size16 != 0) {
        *dest16++ = *src16++;
        size16--;
    }

    readSramCore = (void (*)(const u8 *, u8 *, u32))((u32)readSram_Work + 1);
    readSramCore(src, dest, size);
}

/* 0x08111924: WriteSram */
void sub_08111924(const u8 *src, volatile u8 *dest, u32 size) {
    REG_WAITCNT = (REG_WAITCNT & 0xFFFC) | 3;
    while (size-- != 0) {
        *dest++ = *src++;
    }
}

/* 0x08111964: VerifySram_Core */
u32 sub_08111964(const u8 *src, const u8 *tgt, u32 size) {
    while (size-- != 0) {
        if (*tgt++ != *src++)
            return (u32)(tgt - 1);
    }
    return 0;
}

/* 0x08111994: VerifySram */
u32 sub_08111994(const u8 *src, const u8 *tgt, u32 size) {
    u16 verifySram_Work[0x60];
    u16 *src16;
    u16 *dest16;
    u16 size16;
    u32 (*verifySramCore)(const u8 *, const u8 *, u32);

    REG_WAITCNT = (REG_WAITCNT & 0xFFFC) | 3;

    src16 = (u16 *)sub_08111964;
    src16 = (u16 *)((u32)src16 & ~1);
    dest16 = verifySram_Work;
    size16 = ((u32)sub_08111994 - (u32)sub_08111964) / 2;
    while (size16 != 0) {
        *dest16++ = *src16++;
        size16--;
    }

    verifySramCore = (u32 (*)(const u8 *, const u8 *, u32))((u32)verifySram_Work + 1);
    return verifySramCore(src, tgt, size);
}

/* 0x081119F8: WriteSramEx */
u32 sub_081119F8(const u8 *src, volatile u8 *dest, u32 size) {
    u8 retry;
    u32 errorAddr;

    for (retry = 0; retry < 3; retry++) {
        sub_08111924(src, dest, size);
        errorAddr = sub_08111994(src, (const u8 *)dest, size);
        if (errorAddr == 0)
            break;
    }

    return errorAddr;
}
