/* Portable GBA emulator backend interface shared across PC, Wii U, 3DS, etc.
 * Implementations live beside this header (e.g. emu_mgba.c). */
#ifndef MMBN4_EMU_BACKEND_H
#define MMBN4_EMU_BACKEND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EMU_WIDTH  240
#define EMU_HEIGHT 160
/* GBA: 16777216 Hz / 280896 cycles per frame = 59.7275 fps */
#define EMU_FRAME_NS (1000000000.0 * 280896.0 / 16777216.0)

/* GBA key bits (mGBA's GBA_KEY_* order). */
enum {
    EMU_KEY_A = 1 << 0,
    EMU_KEY_B = 1 << 1,
    EMU_KEY_SELECT = 1 << 2,
    EMU_KEY_START = 1 << 3,
    EMU_KEY_RIGHT = 1 << 4,
    EMU_KEY_LEFT = 1 << 5,
    EMU_KEY_UP = 1 << 6,
    EMU_KEY_DOWN = 1 << 7,
    EMU_KEY_R = 1 << 8,
    EMU_KEY_L = 1 << 9,
};

/* Backend capability flags returned by emu_caps(). */
enum {
    EMU_CAP_MEMORY    = 1 << 0,  /* emu_read* / emu_write* supported */
    EMU_CAP_SAVESTATE = 1 << 1,  /* emu_save_state / emu_load_state supported */
    EMU_CAP_IDLE_HOOK = 1 << 2,  /* idle-loop detection / fast-wait enabled */
    EMU_CAP_AUDIO     = 1 << 3,  /* emu_read_audio produces stereo s16 PCM */
};

typedef enum {
    EMU_MEM_EWRAM = 0,  /* 0x02000000, 256 KB */
    EMU_MEM_IWRAM = 1,  /* 0x03000000,  32 KB */
} EmuMemRegion;

typedef struct Emu Emu;

/* Loads rom_path, attaches save_path as cartridge SRAM (created if missing;
 * same format as mGBA's .sav), resets. Returns NULL on failure. */
Emu* emu_open(const char* rom_path, const char* save_path, int sample_rate);
void emu_close(Emu* emu);

uint32_t emu_caps(const Emu* emu);

void emu_run_frame(Emu* emu);
void emu_set_keys(Emu* emu, uint32_t keys);

/* 240x160 pixels, 32-bit XBGR8888 layout. */
const uint32_t* emu_video(const Emu* emu);

/* Interleaved stereo s16. Returns sample frames written. */
int emu_read_audio(Emu* emu, int16_t* out, int max_frames);

/* Side-effect-free bus reads and writes (host-endian return/value). */
uint32_t emu_read8(Emu* emu, uint32_t addr);
uint32_t emu_read16(Emu* emu, uint32_t addr);
uint32_t emu_read32(Emu* emu, uint32_t addr);

void emu_write8(Emu* emu, uint32_t addr, uint8_t val);
void emu_write16(Emu* emu, uint32_t addr, uint16_t val);
void emu_write32(Emu* emu, uint32_t addr, uint32_t val);

/* Optional raw pointer to a GBA memory region (always little-endian bytes;
 * use gba_r16/gba_r32/gba_w16/gba_w32 to access). Returns NULL if unavailable. */
uint8_t* emu_mem_ptr(Emu* emu, EmuMemRegion region, size_t* out_size);

bool emu_save_state(Emu* emu, const char* path);
bool emu_load_state(Emu* emu, const char* path);

void emu_game_code(const Emu* emu, char out[16]);  /* e.g. "AGB-B4WE" */
uint32_t emu_crc32(const Emu* emu);
uint32_t emu_frame(const Emu* emu);

#endif
