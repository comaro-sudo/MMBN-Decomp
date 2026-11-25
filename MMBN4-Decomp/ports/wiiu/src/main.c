/* SPDX-License-Identifier: GPL-3.0-or-later */
/* MMBN4 Violet Eclipse - Wii U port (WU-1..WU-4: dual-screen TV + GamePad PET).
 *
 * Uses ports/common/ (mGBA 0.10.5 static core, endian-safe RAM reader,
 * interactive 4-tab PET UI, portable bench_run) + WUT (WHBProc, VPAD,
 * OSGetSystemTime) + wiiu-sdl2 dual-window GX2 backend:
 *   - TV (1920x1080): GBA game at integer 6x (1440x960 centered)
 *   - GamePad DRC (854x480): Interactive PET (Status/Folder/NetMap/Custom)
 *     with calibrated VPAD touch input; press ZR to toggle Off-TV play.
 *
 * SD card layout:
 *   sd:/wiiu/apps/mmbn4/mmbn4.wuhb
 *   sd:/wiiu/apps/mmbn4/bn4_rs_overhaul.gba  (or bn4_bm_overhaul.gba / bn4_rs_us.gba)
 *   sd:/wiiu/apps/mmbn4/saves/<code>.sav     (mGBA-compatible SRAM)
 *
 * Real-hardware benchmark (Gate G-WU: < 12 ms mean single-threaded):
 *   Hold L + R while launching (or create sd:/wiiu/apps/mmbn4/bench.txt).
 *   Runs 600 unthrottled frames and writes sd:/wiiu/apps/mmbn4/bench.log.
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
#include "pet_ui.h"
#include "ui_draw_sdl2.h"

#define SAMPLE_RATE 48000
#define TV_W        1920
#define TV_H        1080
#define DRC_W       854
#define DRC_H       480
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

    /* Dual-screen Wii U GX2 setup via wiiu-sdl2:
     *   TV  (1920x1080): GBA game at integer 6x (1440x960 centered)
     *   DRC (854x480)  : Interactive PET (logical 320x240 scaled cleanly) */
    SDL_Window* tv_win = SDL_CreateWindow("MMBN4 TV", 0, 0, TV_W, TV_H, SDL_WINDOW_WIIU_TV_ONLY);
    SDL_Renderer* tv_ren = SDL_CreateRenderer(tv_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* tv_tex = SDL_CreateTexture(tv_ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                            EMU_WIDTH, EMU_HEIGHT);

    SDL_Window* drc_win = SDL_CreateWindow("MMBN4 PET", 0, 0, DRC_W, DRC_H,
                                           SDL_WINDOW_WIIU_GAMEPAD_ONLY | SDL_WINDOW_WIIU_PREVENT_SWAP);
    SDL_Renderer* drc_ren = drc_win ? SDL_CreateRenderer(drc_win, -1, SDL_RENDERER_ACCELERATED) : NULL;
    SDL_Texture* drc_game_tex = drc_ren ? SDL_CreateTexture(drc_ren, SDL_PIXELFORMAT_ABGR8888,
                                                            SDL_TEXTUREACCESS_STREAMING, EMU_WIDTH, EMU_HEIGHT) : NULL;
    if (drc_ren) SDL_RenderSetLogicalSize(drc_ren, PET_QVGA_W, PET_QVGA_H);

    SDL_AudioSpec want = { 0 }, have = { 0 };
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    SDL_AudioDeviceID aud = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (aud) SDL_PauseAudioDevice(aud, 0);

    PetUiState pet_ui;
    pet_ui_init(&pet_ui);
    PetModel pet_model = { 0 };
    bool off_tv = false;
    bool was_touched = false;
    const char* message = NULL;
    uint64_t msg_until = 0;
    uint64_t fps_t0 = wiiu_now_ns();
    uint32_t fps_f0 = emu_frame(emu);
    double fps = 59.7;

    int16_t samples[4096 * 2];
    const SDL_Rect tv_dst = { (TV_W - EMU_WIDTH * 6) / 2, (TV_H - EMU_HEIGHT * 6) / 2,
                              EMU_WIDTH * 6, EMU_HEIGHT * 6 };

    while (WHBProcIsRunning()) {
        uint32_t keys = 0;
        VPADRead(VPAD_CHAN_0, &vpad, 1, &verr);
        if (verr == VPAD_READ_SUCCESS) {
            if (vpad.trigger & VPAD_BUTTON_ZR) {
                off_tv = !off_tv;
                if (drc_ren) {
                    SDL_RenderSetLogicalSize(drc_ren, off_tv ? EMU_WIDTH : PET_QVGA_W,
                                             off_tv ? EMU_HEIGHT : PET_QVGA_H);
                }
                message = off_tv ? "Off-TV ON (ZR)" : "Off-TV OFF (ZR)";
                msg_until = wiiu_now_ns() + 2000000000ULL;
            }
            keys = vpad_to_gba_keys(&vpad);

            /* GamePad touch -> interactive PET (rising edge) */
            bool touched = vpad.tpNormal.touched && !vpad.tpNormal.validity;
            if (touched && !was_touched && !off_tv) {
                VPADTouchData cal = { 0 };
                VPADGetTPCalibratedPoint(VPAD_CHAN_0, &cal, &vpad.tpNormal);
                int px = (int) cal.x * PET_QVGA_W / 1280;
                int py = (int) cal.y * PET_QVGA_H / 720;
                const char* act = pet_ui_click(&pet_ui, &pet_model, PET_LAYOUT_QVGA, px, py, emu, game);
                if (act) {
                    message = act;
                    msg_until = wiiu_now_ns() + 2000000000ULL;
                }
            }
            was_touched = touched;
        }
        keys |= pet_ui_pop_keys(&pet_ui);
        emu_set_keys(emu, keys);

        emu_run_frame(emu);
        int n = emu_read_audio(emu, samples, 4096);
        if (aud && n > 0 && SDL_GetQueuedAudioSize(aud) < (uint32_t) (SAMPLE_RATE * 4 / 8)) {
            SDL_QueueAudio(aud, samples, (uint32_t) (n * 4));
        }

        uint64_t now = wiiu_now_ns();
        if (message && now > msg_until) message = NULL;
        if (now - fps_t0 >= 500000000ULL) {
            fps = (double) (emu_frame(emu) - fps_f0) * 1e9 / (double) (now - fps_t0);
            fps_t0 = now;
            fps_f0 = emu_frame(emu);
        }

        /* TV render */
        SDL_SetRenderDrawColor(tv_ren, 0, 0, 0, 255);
        SDL_RenderClear(tv_ren);
        if (!off_tv) {
            SDL_UpdateTexture(tv_tex, NULL, emu_video(emu), EMU_WIDTH * 4);
            SDL_RenderCopy(tv_ren, tv_tex, NULL, &tv_dst);
        }
        SDL_RenderPresent(tv_ren);

        /* GamePad DRC render: PET (normal) or GBA game (Off-TV) */
        if (drc_ren) {
            if (off_tv && drc_game_tex) {
                SDL_SetRenderDrawColor(drc_ren, 0, 0, 0, 255);
                SDL_RenderClear(drc_ren);
                SDL_UpdateTexture(drc_game_tex, NULL, emu_video(emu), EMU_WIDTH * 4);
                SDL_RenderCopy(drc_ren, drc_game_tex, NULL, NULL);
            } else {
                PetStatus st = { fps, false, keys, message };
                pet_model_update(&pet_model, emu, game, code, &st);
                UiDraw ui = ui_draw_sdl2(drc_ren);
                pet_ui_draw(&ui, &pet_model, &pet_ui, PET_LAYOUT_QVGA);
            }
            SDL_RenderPresent(drc_ren);
        }
    }

    if (aud) SDL_CloseAudioDevice(aud);
    if (drc_game_tex) SDL_DestroyTexture(drc_game_tex);
    if (drc_ren) SDL_DestroyRenderer(drc_ren);
    if (drc_win) SDL_DestroyWindow(drc_win);
    SDL_DestroyTexture(tv_tex);
    SDL_DestroyRenderer(tv_ren);
    SDL_DestroyWindow(tv_win);
    SDL_Quit();
    emu_close(emu);
    WHBProcShutdown();
    return 0;
}
