/* mGBA 0.10.5 (MPL-2.0) implementation of emu_backend.h. */
#include "emu_backend.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>

struct Emu {
    struct mCore* core;
    uint32_t* fb;
    blip_t* left;
    blip_t* right;
};

/* mGBA logs everything (every BIOS call) by default; keep warnings and up. */
static void quiet_log(struct mLogger* logger, int category, enum mLogLevel level, const char* format,
                      va_list args) {
    (void) logger;
    if (!(level & (mLOG_FATAL | mLOG_ERROR | mLOG_WARN))) return;
    fprintf(stderr, "mgba[%s]: ", mLogCategoryName(category));
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}
static struct mLogger QUIET_LOGGER = { quiet_log, NULL };

Emu* emu_open(const char* rom_path, const char* save_path, int sample_rate) {
    mLogSetDefaultLogger(&QUIET_LOGGER);
    Emu* emu = calloc(1, sizeof(*emu));
    if (!emu) return NULL;

    emu->core = mCoreFind(rom_path);
    if (!emu->core) {
        fprintf(stderr, "emu: %s is not a ROM mGBA recognises\n", rom_path);
        free(emu);
        return NULL;
    }
    struct mCore* core = emu->core;
    if (!core->init(core)) {
        fprintf(stderr, "emu: core init failed\n");
        free(emu);
        return NULL;
    }
    /* No config file is read: defaults only (no BIOS, HLE BIOS calls).
     * Enable mGBA's idle-loop detector so slow hosts (Wii U / 3DS) skip
     * busy-wait spins cleanly. */
    mCoreInitConfig(core, NULL);
    mCoreConfigSetDefaultValue(&core->config, "idleOptimization", "detect");
    mCoreLoadConfig(core);

    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    emu->fb = calloc((size_t) EMU_WIDTH * EMU_HEIGHT, sizeof(uint32_t));
    core->setVideoBuffer(core, (color_t*) emu->fb, EMU_WIDTH);
    core->setAudioBufferSize(core, 4096);

    if (!mCoreLoadFile(core, rom_path)) {
        fprintf(stderr, "emu: could not load %s\n", rom_path);
        emu_close(emu);
        return NULL;
    }

    if (save_path && save_path[0]) {
        struct VFile* save = VFileOpen(save_path, O_CREAT | O_RDWR);
        if (!save || !core->loadSave(core, save)) {
            fprintf(stderr, "emu: could not open save %s (continuing without)\n", save_path);
        }
    }

    core->reset(core);

    emu->left = core->getAudioChannel(core, 0);
    emu->right = core->getAudioChannel(core, 1);
    blip_set_rates(emu->left, core->frequency(core), sample_rate);
    blip_set_rates(emu->right, core->frequency(core), sample_rate);
    return emu;
}

void emu_close(Emu* emu) {
    if (!emu) return;
    if (emu->core) {
        emu->core->unloadROM(emu->core);  /* flushes SRAM */
        mCoreConfigDeinit(&emu->core->config);
        emu->core->deinit(emu->core);
    }
    free(emu->fb);
    free(emu);
}

uint32_t emu_caps(const Emu* emu) {
    (void) emu;
    return EMU_CAP_MEMORY | EMU_CAP_SAVESTATE | EMU_CAP_IDLE_HOOK | EMU_CAP_AUDIO;
}

void emu_run_frame(Emu* emu) { emu->core->runFrame(emu->core); }

void emu_set_keys(Emu* emu, uint32_t keys) { emu->core->setKeys(emu->core, keys); }

const uint32_t* emu_video(const Emu* emu) { return emu->fb; }

int emu_read_audio(Emu* emu, int16_t* out, int max_frames) {
    int avail = blip_samples_avail(emu->left);
    if (avail > max_frames) avail = max_frames;
    if (avail <= 0) return 0;
    blip_read_samples(emu->left, out, avail, 1);
    blip_read_samples(emu->right, out + 1, avail, 1);
    return avail;
}

/* busRead* / busWrite* go through the GBA memory map without side effects on timing. */
uint32_t emu_read8(Emu* emu, uint32_t addr) { return emu->core->busRead8(emu->core, addr); }
uint32_t emu_read16(Emu* emu, uint32_t addr) { return emu->core->busRead16(emu->core, addr); }
uint32_t emu_read32(Emu* emu, uint32_t addr) { return emu->core->busRead32(emu->core, addr); }

void emu_write8(Emu* emu, uint32_t addr, uint8_t val) { emu->core->busWrite8(emu->core, addr, val); }
void emu_write16(Emu* emu, uint32_t addr, uint16_t val) { emu->core->busWrite16(emu->core, addr, val); }
void emu_write32(Emu* emu, uint32_t addr, uint32_t val) { emu->core->busWrite32(emu->core, addr, val); }

uint8_t* emu_mem_ptr(Emu* emu, EmuMemRegion region, size_t* out_size) {
    size_t sz = 0;
    /* GBA memory region IDs in mGBA: 0x2 = EWRAM, 0x3 = IWRAM. */
    size_t id = (region == EMU_MEM_IWRAM) ? 0x3u : 0x2u;
    void* ptr = emu->core->getMemoryBlock(emu->core, id, &sz);
    if (out_size) *out_size = sz;
    return (uint8_t*) ptr;
}

bool emu_save_state(Emu* emu, const char* path) {
    size_t size = emu->core->stateSize(emu->core);
    void* buf = malloc(size);
    if (!buf) return false;
    bool ok = emu->core->saveState(emu->core, buf);
    if (ok) {
        FILE* f = fopen(path, "wb");
        ok = f && fwrite(buf, 1, size, f) == size;
        if (f) fclose(f);
    }
    free(buf);
    return ok;
}

bool emu_load_state(Emu* emu, const char* path) {
    size_t size = emu->core->stateSize(emu->core);
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    void* buf = malloc(size);
    bool ok = buf && fread(buf, 1, size, f) == size;
    fclose(f);
    if (ok) ok = emu->core->loadState(emu->core, buf);
    free(buf);
    return ok;
}

void emu_game_code(const Emu* emu, char out[16]) {
    memset(out, 0, 16);
    emu->core->getGameCode(emu->core, out);
}

uint32_t emu_crc32(const Emu* emu) {
    uint32_t crc = 0;
    emu->core->checksum(emu->core, &crc, mCHECKSUM_CRC32);
    return crc;
}

uint32_t emu_frame(const Emu* emu) { return emu->core->frameCounter(emu->core); }
