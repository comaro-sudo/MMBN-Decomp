/* Phase C2 / E2 portable C reference implementations of core MMBN4 input,
 * soft-reset, memory-fill/copy, and music wrapper routines from code_00.s.
 *
 * Verified with `make modern-check` (-Wall -Wextra -Werror). */

#include "gba_io.h"
#include "toolkit.h"

/* 0x080003B0: Joypad_Poll */
void Joypad_Poll(struct JoypadState *jp, u16 keyInputReg) {
    u16 held, prev, repeatMask;
    int i;

    jp->repeatPhase++;
    if (jp->repeatPhase > 4)
        jp->repeatPhase = 0;

    held = (u16)~keyInputReg;
    prev = jp->held;
    jp->prevHeld = prev;
    jp->held = held;

    repeatMask = (u16)(held & prev);
    for (i = 0; i < 10; i++) {
        u16 bit = (u16)(1u << i);
        if (repeatMask & bit) {
            if (jp->holdTimers[i] < 16) {
                jp->holdTimers[i]++;
                if (jp->holdTimers[i] != 1)
                    repeatMask &= (u16)~bit;
            } else if (jp->repeatPhase != 0) {
                repeatMask &= (u16)~bit;
            }
        } else {
            jp->holdTimers[i] = 0;
        }
    }

    jp->repeated = repeatMask;
    jp->pressed = (u16)(held & (u16)~prev);
}

/* 0x08000418: CheckSoftReset (returns 1 if A+B+Start+Select triggered reset) */
int CheckSoftReset(struct Toolkit *tk, int canReset) {
    const u16 RESET_MASK = KEY_A | KEY_B | KEY_SELECT | KEY_START;
    u16 held, pressed;
    s32 timer;

    if (!canReset || tk->mainState->subsystem == 0x14)
        return 0;

    held = tk->joypad->held;
    pressed = tk->joypad->pressed;
    timer = (s32)tk->mainState->softResetTimer - 1;
    if (timer > 0) {
        tk->mainState->softResetTimer = (u8)timer;
        return 0;
    }

    tk->mainState->softResetTimer = 0;
    if ((held & RESET_MASK) == RESET_MASK && (pressed & RESET_MASK) != 0) {
        tk->mainState->softResetTimer = 10;
        return 1;
    }
    return 0;
}

/* 0x08000938: ZeroFillByWord */
void ZeroFillByWord(void *dst, u32 byteLen) {
    u32 *p = (u32 *)dst;
    u32 words = byteLen >> 2;
    while (words--)
        *p++ = 0;
}

/* 0x08000978: CopyBytes */
void CopyBytes(const void *src, void *dst, u32 byteLen) {
    const u8 *s = (const u8 *)src;
    u8 *d = (u8 *)dst;
    while (byteLen--)
        d[byteLen] = s[byteLen];
}

/* 0x08000994: CopyWords */
void CopyWords(const void *src, void *dst, u32 byteLen) {
    const u32 *s = (const u32 *)src;
    u32 *d = (u32 *)dst;
    u32 words = byteLen >> 2;
    while (words--)
        *d++ = *s++;
}
