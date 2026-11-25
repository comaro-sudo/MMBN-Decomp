/* DC-5 C: GBA BIOS SWI wrappers (code_10, RS 0x08111878-0x0811189B, BM +0x14).
 * Compiled with arm-none-eabi-gcc -mcpu=arm7tdmi -mthumb -O2 and verified
 * byte-identical in both Red Sun and Blue Moon. */
#include <stdint.h>

#define NAKED __attribute__((naked, used))

/* 0x08111878: SWI 0x0E — BgAffineSet */
NAKED void sub_08111878(const void *src, void *dst, int32_t count) {
    __asm__ volatile("svc #0x0e\n\tbx lr");
}

/* 0x0811187C: SWI 0x0C — CpuFastSet */
NAKED void sub_0811187C(const void *src, void *dst, uint32_t control) {
    __asm__ volatile("svc #0x0c\n\tbx lr");
}

/* 0x08111880: SWI 0x0B — CpuSet */
NAKED void sub_08111880(const void *src, void *dst, uint32_t control) {
    __asm__ volatile("svc #0x0b\n\tbx lr");
}

/* 0x08111884: SWI 0x06 — Div */
NAKED int32_t sub_08111884(int32_t num, int32_t denom) {
    __asm__ volatile("svc #0x06\n\tbx lr");
}

/* 0x08111888: SWI 0x06 — DivRem (returns remainder in r0) */
NAKED int32_t sub_08111888(int32_t num, int32_t denom) {
    __asm__ volatile(
        "svc #0x06\n\t"
        "adds r0, r1, #0\n\t"
        "bx lr\n\t"
        ".hword 0x0000"
    );
}

/* 0x08111890: SWI 0x12 — LZ77UnCompWram */
NAKED void sub_08111890(const void *src, void *dst) {
    __asm__ volatile("svc #0x12\n\tbx lr");
}

/* 0x08111894: SWI 0x11 — LZ77UnCompVram */
NAKED void sub_08111894(const void *src, void *dst) {
    __asm__ volatile("svc #0x11\n\tbx lr");
}

/* 0x08111898: SWI 0x08 — Sqrt */
NAKED uint32_t sub_08111898(uint32_t x) {
    __asm__ volatile("svc #0x08\n\tbx lr");
}
