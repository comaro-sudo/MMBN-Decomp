/* MMBN4 Violet Eclipse - Wii U port (WU-1 / WU-2 / WU-3 hardware bench).
 *
 * Uses ports/common/ (mGBA 0.10.5 static core, endian-safe RAM reader,
 * portable PET model, portable bench_run) + WUT (WHBProc, VPAD, OSGetSystemTime)
 * + SDL2 for initial TV/GamePad presentation and 48 kHz audio.
 *
 * SD card layout:
 *   sd:/wiiu/apps/mmbn4/mmbn4.wuhb
 *   sd:/wiiu/apps/mmbn4/bn4_rs_overhaul.gba  (or bn4_bm_overhaul.gba / bn4_rs_us.gba)
 *   sd:/wiiu/apps/mmbn4/saves/<code>.sav     (mGBA-compatible SRAM)
 *
 * Real-hardware benchmark (Gate G-WU: < 12 ms mean single-threaded):
 *   Hold L + R while launching (or create sd:/wiiu/apps/mmbn4/bench.txt).
 *   Runs 600 unthrottled frames (using bench.ss if present) and writes
 *   mean / p90 / p99 / max ms + FPS to sd:/wiiu/apps/mmbn4/bench.log.
 */
#include <SDL2/SDL.h>
#include <coreinit/time.h>
#include <sys/stat.h>
#include <vpad/input.h>
#include <whb/proc.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bench.h"
#include "emu_backend.h"
#include "game.h"
#include "pet_model.h"

#define SAMPLE_RATE 48000
#define APP_DIR     "fs:/vol/external01/wiiu/apps/mmbn4"
#define SAVES_DIR   APP_DIR "/saves"

static const char* ROM_CANDIDATES[] = {
    APP_DIR "/bn4_rs_overhaul.gba",
    APP_DIR "/bn4_bm_overhaul.gba",
    APP_DIR "/bn4_rs_us.gba",
    APP_DIR "/bn4_bm_us.gba",
    APP_DIR "/rom.gba",
};

static bool file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static const char* find_rom(void) {
    for (size_t i = 0; i < sizeof(ROM_CANDIDATES) / sizeof(ROM_CANDIDATES[0]); ++i) {
        if (file_exists(ROM_CANDIDATES[i])) return ROM_CANDIDATES[i];
    }
    return NULL;
}

static uint64_t wiiu_now_ns(void) {
    return (uint64_t) OSTicksToNanoseconds(OSGetSystemTime());
}

static uint32_t vpad_to_gba_keys(const VPADStatus* v) {
    uint32_t keys = 0;
    uint32_t h = v->hold;
    if (h & VPAD_BUTTON_A)     keys |= EMU_KEY_A;
    if (h & VPAD_BUTTON_B)     keys |= EMU_KEY_B;
    if (h & VPAD_BUTTON_MINUS) keys |= EMU_KEY_SELECT;
    if (h & VPAD_BUTTON_PLUS)  keys |= EMU_KEY_START;
    if (h & VPAD_BUTTON_RIGHT) keys |= EMU_KEY_RIGHT;
    if (h & VPAD_BUTTON_LEFT)  keys |= EMU_KEY_LEFT;
    if (h & VPAD_BUTTON_UP)    keys |= EMU_KEY_UP;
    if (h & VPAD_BUTTON_DOWN)  keys |= EMU_KEY_DOWN;
    if (h & VPAD_BUTTON_R)     keys |= EMU_KEY_R;
    if (h & VPAD_BUTTON_L)     keys |= EMU_KEY_L;
    /* Left stick snapped to clean 4-way for BN4 battle grid */
    float sx = v->leftStick.x, sy = v->leftStick.y;
    float ax = sx < 0 ? -sx : sx;
    float ay = sy < 0 ? -sy : sy;
    if (ax > ay) {
        if (sx > 0.45f)  keys |= EMU_KEY_RIGHT;
        if (sx < -0.45f) keys |= EMU_KEY_LEFT;
    } else {
        if (sy > 0.45f)  keys |= EMU_KEY_UP;
        if (sy < -0.45f) keys |= EMU_KEY_DOWN;
    }
    return keys;
}

static void run_hardware_bench(Emu* emu, const GameInfo* game, const char* code) {
    const char* state = file_exists(APP_DIR "/bench.ss") ? (APP_DIR "/bench.ss") : NULL;
    BenchResult res = { 0 };
    bool ok = bench_run(emu, state, 600, wiiu_now_ns, &res);
    FILE* f = fopen(APP_DIR "/bench.log", "w");
    if (f) {
        if (ok) {
            fprintf(f, "Wii U hardware benchmark (mGBA 0.10.5, idleOptimization=detect)\n");
            fprintf(f, "game=%s (%s) state=%s frames=%d\n",
                    code, game->short_name, state ? state : "(cold boot)", res.frames);
            fprintf(f, "mean_ms=%.3f p90_ms=%.3f p99_ms=%.3f max_ms=%.3f fps=%.1f\n",
                    res.mean_ms, res.p90_ms, res.p99_ms, res.max_ms, res.fps);
            fprintf(f, "gate_G_WU (<12.0 ms single-threaded): %s\n",
                    res.mean_ms < 12.0 ? "PASS" : "NEEDS_CORE_THREAD");
        } else {
            fprintf(f, "bench failed\n");
        }
        fclose(f);
    }
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    WHBProcInit();
    mkdir(SAVES_DIR, 0777);

    const char* rom = find_rom();
    if (!rom) {
        WHBProcShutdown();
        return 1;
    }

    /* Name save by ROM header game code (0xAC), matching PC port. */
    char short_code[5] = "GAME";
    {
        FILE* rf = fopen(rom, "rb");
        if (rf && fseek(rf, 0xAC, SEEK_SET) == 0 && fread(short_code, 1, 4, rf) == 4) {
            short_code[4] = '\0';
        }
        if (rf) fclose(rf);
    }
    char save_path[256];
    snprintf(save_path, sizeof save_path, SAVES_DIR "/%s.sav", short_code);

    Emu* emu = emu_open(rom, save_path, SAMPLE_RATE);
    if (!emu) {
        WHBProcShutdown();
        return 2;
    }

    char code[16];
    emu_game_code(emu, code);
    const GameInfo* game = game_identify(code);

    /* Check if L+R held at launch or bench.txt exists on SD -> run WU-3 bench */
    VPADStatus vpad = { 0 };
    VPADReadError verr = VPAD_READ_SUCCESS;
    VPADRead(VPAD_CHAN_0, &vpad, 1, &verr);
    bool do_bench = file_exists(APP_DIR "/bench.txt") ||
                    ((vpad.hold & (VPAD_BUTTON_L | VPAD_BUTTON_R)) == (VPAD_BUTTON_L | VPAD_BUTTON_R));
    if (do_bench) {
        run_hardware_bench(emu, game, code);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        emu_close(emu);
        WHBProcShutdown();
        return 3;
    }

    SDL_Window* win = SDL_CreateWindow("MMBN4 Violet Eclipse", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(ren, EMU_WIDTH, EMU_HEIGHT);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                         EMU_WIDTH, EMU_HEIGHT);

    SDL_AudioSpec want = { 0 }, have = { 0 };
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    SDL_AudioDeviceID aud = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (aud) SDL_PauseAudioDevice(aud, 0);

    int16_t samples[4096 * 2];
    while (WHBProcIsRunning()) {
        VPADRead(VPAD_CHAN_0, &vpad, 1, &verr);
        if (verr == VPAD_READ_SUCCESS) {
            emu_set_keys(emu, vpad_to_gba_keys(&vpad));
        }

        emu_run_frame(emu);
        int n = emu_read_audio(emu, samples, 4096);
        if (aud && n > 0 && SDL_GetQueuedAudioSize(aud) < (uint32_t) (SAMPLE_RATE * 4 / 8)) {
            SDL_QueueAudio(aud, samples, (uint32_t) (n * 4));
        }

        SDL_UpdateTexture(tex, NULL, emu_video(emu), EMU_WIDTH * 4);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    if (aud) SDL_CloseAudioDevice(aud);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    emu_close(emu);
    WHBProcShutdown();
    return 0;
}
