/* DC-5 C: GBA BIOS SWI 0x2A — SoundBias (code_10, RS 0x08112B98, BM 0x08112BAC).
 * Compiled with arm-none-eabi-gcc -mcpu=arm7tdmi -mthumb -O2 and verified
 * byte-identical in both Red Sun and Blue Moon. */
#include <stdint.h>

#define NAKED __attribute__((naked, used))

/* 0x08112B98: SWI 0x2A — SoundBias (called by m4aSoundVSyncOff / m4aSoundVSyncOn) */
NAKED void sub_08112B98(uint32_t level) {
    __asm__ volatile("svc #0x2a\n\tbx lr");
}
