/* Thin wrapper around an embedded mGBA core (mGBA 0.10.5, MPL-2.0).
 * The game's own code runs unmodified; the host reads/writes its memory
 * between frames ("option E" in plan/05). */
#ifndef MMBN4_EMU_H
#define MMBN4_EMU_H

#include <stdbool.h>
#include <stdint.h>

#define EMU_WIDTH  240
#define EMU_HEIGHT 160
/* GBA: 16777216 Hz / 280896 cycles per frame = 59.7275 fps */
#define EMU_FRAME_NS (1000000000.0 * 280896.0 / 16777216.0)

/* GBA key bits (mGBA's GBA_KEY_* order). */
enum {
    EMU_KEY_A = 1 << 0, EMU_KEY_B = 1 << 1, EMU_KEY_SELECT = 1 << 2, EMU_KEY_START = 1 << 3,
    EMU_KEY_RIGHT = 1 << 4, EMU_KEY_LEFT = 1 << 5, EMU_KEY_UP = 1 << 6, EMU_KEY_DOWN = 1 << 7,
    EMU_KEY_R = 1 << 8, EMU_KEY_L = 1 << 9,
};

typedef struct Emu Emu;

/* Loads rom_path, attaches save_path as the cartridge SRAM (created if
 * missing; same format as mGBA's .sav), resets. NULL on failure. */
Emu* emu_open(const char* rom_path, const char* save_path, int sample_rate);
void emu_close(Emu* emu);

void emu_run_frame(Emu* emu);
void emu_set_keys(Emu* emu, uint32_t keys);

/* 240x160 pixels, 32-bit, SDL_PIXELFORMAT_XBGR8888 layout. */
const uint32_t* emu_video(const Emu* emu);

/* Interleaved stereo s16. Returns sample frames written. */
int emu_read_audio(Emu* emu, int16_t* out, int max_frames);

uint32_t emu_read8(Emu* emu, uint32_t addr);
uint32_t emu_read16(Emu* emu, uint32_t addr);
uint32_t emu_read32(Emu* emu, uint32_t addr);

bool emu_save_state(Emu* emu, const char* path);
bool emu_load_state(Emu* emu, const char* path);

void emu_game_code(const Emu* emu, char out[16]);  /* e.g. "AGB-B4WE" */
uint32_t emu_crc32(const Emu* emu);
uint32_t emu_frame(const Emu* emu);

#endif
