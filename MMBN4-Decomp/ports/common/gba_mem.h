/* Endian-safe little-endian memory helpers for reading/writing GBA RAM.
 * Required by all ports so big-endian hosts (Wii U PowerPC) never cast raw
 * EWRAM/IWRAM buffers to host structs. */
#ifndef MMBN4_GBA_MEM_H
#define MMBN4_GBA_MEM_H

#include <stdint.h>

static inline uint16_t gba_r16(const uint8_t* p) {
    return (uint16_t) ((uint16_t) p[0] | ((uint16_t) p[1] << 8));
}

static inline uint32_t gba_r32(const uint8_t* p) {
    return (uint32_t) p[0]
         | ((uint32_t) p[1] << 8)
         | ((uint32_t) p[2] << 16)
         | ((uint32_t) p[3] << 24);
}

static inline void gba_w16(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t) (v & 0xFFu);
    p[1] = (uint8_t) ((v >> 8) & 0xFFu);
}

static inline void gba_w32(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t) (v & 0xFFu);
    p[1] = (uint8_t) ((v >> 8) & 0xFFu);
    p[2] = (uint8_t) ((v >> 16) & 0xFFu);
    p[3] = (uint8_t) ((v >> 24) & 0xFFu);
}

#endif
