/* MMBN4 Overhaul - PC port (Windows ARM64 / x64), first milestone:
 * the game running from the player's own ROM on an embedded mGBA core,
 * presented with SDL3, plus a second "PET" window. See plan/10-pc-port.md.
 *
 *   mmbn4.exe [path/to/rom.gba]
 *
 * Without an argument it looks for build/bn4_rs_overhaul.gba, then
 * roms/bn4_rs_us.gba (relative to the working directory).
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "emu.h"
#include "game.h"
#include "pet.h"

#define SAMPLE_RATE 48000
#define SCALE 4
#define FAST_FORWARD_FRAMES 4

static const char* DEFAULT_ROMS[] = { "build/bn4_rs_overhaul.gba", "roms/bn4_rs_us.gba" };

static const char* find_rom(int argc, char** argv) {
    if (argc > 1) return argv[1];
    for (size_t i = 0; i < SDL_arraysize(DEFAULT_ROMS); ++i) {
        SDL_PathInfo info;
        if (SDL_GetPathInfo(DEFAULT_ROMS[i], &info)) return DEFAULT_ROMS[i];
    }
    return NULL;
}

static uint32_t keyboard_keys(void) {
    const bool* k = SDL_GetKeyboardState(NULL);
    uint32_t keys = 0;
    if (k[SDL_SCANCODE_X]) keys |= EMU_KEY_A;
    if (k[SDL_SCANCODE_Z]) keys |= EMU_KEY_B;
    if (k[SDL_SCANCODE_BACKSPACE]) keys |= EMU_KEY_SELECT;
    if (k[SDL_SCANCODE_RETURN]) keys |= EMU_KEY_START;
    if (k[SDL_SCANCODE_RIGHT]) keys |= EMU_KEY_RIGHT;
    if (k[SDL_SCANCODE_LEFT]) keys |= EMU_KEY_LEFT;
    if (k[SDL_SCANCODE_UP]) keys |= EMU_KEY_UP;
    if (k[SDL_SCANCODE_DOWN]) keys |= EMU_KEY_DOWN;
    if (k[SDL_SCANCODE_S]) keys |= EMU_KEY_R;
    if (k[SDL_SCANCODE_A]) keys |= EMU_KEY_L;
    return keys;
}

/* Positional Nintendo layout: east = A, south = B. */
static uint32_t gamepad_keys(SDL_Gamepad* pad) {
    if (!pad) return 0;
    uint32_t keys = 0;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_EAST)) keys |= EMU_KEY_A;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH)) keys |= EMU_KEY_B;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_BACK)) keys |= EMU_KEY_SELECT;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_START)) keys |= EMU_KEY_START;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) keys |= EMU_KEY_RIGHT;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) keys |= EMU_KEY_LEFT;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP)) keys |= EMU_KEY_UP;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) keys |= EMU_KEY_DOWN;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) keys |= EMU_KEY_R;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) keys |= EMU_KEY_L;
    /* Left stick as a d-pad (battle grid needs clean 4-way input). */
    const int dead = 16000;
    int x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
    int y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
    if (SDL_abs(x) > SDL_abs(y)) {
        if (x > dead) keys |= EMU_KEY_RIGHT;
        if (x < -dead) keys |= EMU_KEY_LEFT;
    } else {
        if (y > dead) keys |= EMU_KEY_DOWN;
        if (y < -dead) keys |= EMU_KEY_UP;
    }
    return keys;
}

static bool gamepad_fast_forward(SDL_Gamepad* pad) {
    return pad && SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16000;
}

/* Headless test mode:
 *   mmbn4.exe rom.gba --test FRAMES --shot out.bmp [--input "300=START,306=,600=A,606="]
 * Runs FRAMES frames unthrottled with no windows or audio, applying the input
 * script (frame=KEYS, KEYS joined with '+', empty releases all), then saves the
 * last frame as a BMP and prints a few RAM values. */
static uint32_t parse_keys(const char* s) {
    static const struct { const char* name; uint32_t bit; } K[] = {
        { "A", EMU_KEY_A }, { "B", EMU_KEY_B }, { "SELECT", EMU_KEY_SELECT }, { "START", EMU_KEY_START },
        { "RIGHT", EMU_KEY_RIGHT }, { "LEFT", EMU_KEY_LEFT }, { "UP", EMU_KEY_UP }, { "DOWN", EMU_KEY_DOWN },
        { "R", EMU_KEY_R }, { "L", EMU_KEY_L },
    };
    uint32_t keys = 0;
    char buf[64];
    SDL_strlcpy(buf, s, sizeof buf);
    for (char* tok = SDL_strtok_r(buf, "+", &(char*){ NULL }); tok; tok = SDL_strtok_r(NULL, "+", &(char*){ NULL })) {
        for (size_t i = 0; i < SDL_arraysize(K); ++i)
            if (SDL_strcasecmp(tok, K[i].name) == 0) keys |= K[i].bit;
    }
    return keys;
}

/* --dump FILE: EWRAM (0x02000000, 256 KB) written raw at the end of a test run,
 * for reverse engineering (docs/re/). --state-in / --state-out chain long runs. */
static bool dump_ewram(Emu* emu, const char* path) {
    SDL_IOStream* io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = true;
    for (uint32_t a = 0x02000000; a < 0x02040000 && ok; a += 4) {
        uint32_t w = emu_read32(emu, a);
        ok = SDL_WriteIO(io, &w, 4) == 4;  /* little-endian host (ARM64/x64) */
    }
    return SDL_CloseIO(io) && ok;
}

typedef struct { const char *dump, *state_in, *state_out; } TestExtras;

static int run_test(const char* rom, int frames, const char* shot, const char* script, const char* save,
                    const TestExtras* x) {
    Emu* emu = emu_open(rom, save, SAMPLE_RATE);
    if (!emu) return 2;
    if (x->state_in && !emu_load_state(emu, x->state_in)) fprintf(stderr, "could not load state %s\n", x->state_in);
    char code[16];
    emu_game_code(emu, code);
    const GameInfo* game = game_identify(code);
    int16_t samples[4096 * 2];
    long audio_frames = 0;
    uint32_t keys = 0;
    const char* p = script;
    for (int f = 0; f < frames; ++f) {
        while (p && *p) {
            int at = SDL_atoi(p);
            if (at != f) break;
            const char* eq = SDL_strchr(p, '=');
            const char* end = SDL_strchr(p, ',');
            char k[64] = "";
            if (eq) SDL_strlcpy(k, eq + 1, (size_t) ((end ? end : eq + 1 + SDL_strlen(eq + 1)) - eq));
            keys = parse_keys(k);
            p = end ? end + 1 : NULL;
        }
        emu_set_keys(emu, keys);
        emu_run_frame(emu);
        audio_frames += emu_read_audio(emu, samples, 4096);
    }
    SDL_Surface* s = SDL_CreateSurfaceFrom(EMU_WIDTH, EMU_HEIGHT, SDL_PIXELFORMAT_XBGR8888,
                                           (void*) emu_video(emu), EMU_WIDTH * 4);
    bool ok = s && shot && SDL_SaveBMP(s, shot);
    long nonblack = 0;
    for (int i = 0; i < EMU_WIDTH * EMU_HEIGHT; ++i) nonblack += (emu_video(emu)[i] & 0xFFFFFF) != 0;
    printf("game=%s (%s) crc32=%08X frames=%u audio_samples=%ld nonblack_pixels=%ld shot=%s\n", code,
           game->name, (unsigned) emu_crc32(emu), (unsigned) emu_frame(emu), audio_frames, nonblack,
           ok ? shot : "(none)");
    if (game->ram_rng) printf("ram rng=%08X\n", (unsigned) emu_read32(emu, game->ram_rng));
    if (x->dump) printf("dump=%s %s\n", x->dump, dump_ewram(emu, x->dump) ? "ok" : "FAILED");
    if (x->state_out) printf("state=%s %s\n", x->state_out, emu_save_state(emu, x->state_out) ? "ok" : "FAILED");
    SDL_DestroySurface(s);
    emu_close(emu);
    return 0;
}

static void fail(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MMBN4", msg, NULL);
}

int main(int argc, char** argv) {
    int test_frames = 0, quit_after = 0;
    const char *shot = NULL, *script = NULL, *test_save = "build/test.sav", *pet_shot = NULL;
    TestExtras extras = { 0 };
    for (int i = 2; i + 1 < argc; ++i) {
        if (!strcmp(argv[i], "--test")) test_frames = SDL_atoi(argv[++i]);
        else if (!strcmp(argv[i], "--quit-after")) quit_after = SDL_atoi(argv[++i]);
        else if (!strcmp(argv[i], "--pet-shot")) pet_shot = argv[++i];
        else if (!strcmp(argv[i], "--shot")) shot = argv[++i];
        else if (!strcmp(argv[i], "--input")) script = argv[++i];
        else if (!strcmp(argv[i], "--save")) test_save = argv[++i];
        else if (!strcmp(argv[i], "--dump")) extras.dump = argv[++i];
        else if (!strcmp(argv[i], "--state-in")) extras.state_in = argv[++i];
        else if (!strcmp(argv[i], "--state-out")) extras.state_out = argv[++i];
    }
    if (test_frames > 0) {
        SDL_Init(0);
        int rc = run_test(argv[1], test_frames, shot, script, test_save, &extras);
        SDL_Quit();
        return rc;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11,direct3d12,vulkan,software");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        fail(SDL_GetError());
        return 1;
    }

    const char* rom = find_rom(argc, argv);
    if (!rom) {
        fail("No ROM found.\n\nUsage: mmbn4.exe path\\to\\rom.gba\n"
             "or run from the repo root with build/bn4_rs_overhaul.gba or roms/bn4_rs_us.gba present.");
        SDL_Quit();
        return 1;
    }

    char* data_dir = SDL_GetPrefPath("MMBN4Overhaul", "PC");
    char save_path[1024], state_path[1024];

    /* The save is named by the header's game code (0xAC), so patched and
     * original builds of the same version share one save. */
    char short_code[5] = "GAME";
    {
        SDL_IOStream* io = SDL_IOFromFile(rom, "rb");
        if (io && SDL_SeekIO(io, 0xAC, SDL_IO_SEEK_SET) == 0xAC && SDL_ReadIO(io, short_code, 4) == 4) {
            short_code[4] = '\0';
        }
        if (io) SDL_CloseIO(io);
    }
    snprintf(save_path, sizeof save_path, "%s%s.sav", data_dir, short_code);
    snprintf(state_path, sizeof state_path, "%s%s.state1", data_dir, short_code);

    Emu* emu = emu_open(rom, save_path, SAMPLE_RATE);
    if (!emu) {
        fail("Could not load the ROM.");
        SDL_Quit();
        return 1;
    }
    char code[16];
    emu_game_code(emu, code);
    const GameInfo* game = game_identify(code);
    SDL_Log("ROM %s (%s), save %s", rom, code, save_path);

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Rect pet_place_next_to = { 0, 0, 0, 0 };
    char title[128];
    snprintf(title, sizeof title, "MMBN4 Overhaul - %s", game->name);
    if (!SDL_CreateWindowAndRenderer(title, EMU_WIDTH * SCALE, EMU_HEIGHT * SCALE, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        fail(SDL_GetError());
        return 1;
    }
    /* Fit the game + PET side by side in the usable display area, using the
     * largest integer scale that fits (the laptop runs 150% DPI scaling). */
    {
        SDL_Rect usable;
        SDL_DisplayID disp = SDL_GetDisplayForWindow(window);
        if (SDL_GetDisplayUsableBounds(disp, &usable)) {
            int title_h = 40;
            int pet_w_base = 320;  /* PET at 1x of its logical size */
            int scale = 1;
            while ((scale + 1) * EMU_WIDTH + pet_w_base * (scale + 1) / 2 <= usable.w &&
                   (scale + 1) * EMU_HEIGHT + title_h <= usable.h)
                ++scale;
            int gw = EMU_WIDTH * scale, gh = EMU_HEIGHT * scale;
            int pw = pet_w_base * scale / 2, ph = 240 * scale / 2;
            if (pw < 320) { pw = 320; ph = 240; }
            int total = gw + pw;
            int x0 = usable.x + (usable.w > total ? (usable.w - total) / 2 : 0);
            int y0 = usable.y + title_h;
            SDL_SetWindowSize(window, gw, gh);
            SDL_SetWindowPosition(window, x0, y0);
            pet_place_next_to = (SDL_Rect){ x0 + gw, y0, pw, ph };
            if (pet_place_next_to.x + pw > usable.x + usable.w) pet_place_next_to.x = usable.x + usable.w - pw;
            SDL_Log("Display usable %dx%d at %d,%d -> game %dx%d (x%d) at %d,%d, PET %dx%d at %d,%d", usable.w,
                    usable.h, usable.x, usable.y, gw, gh, scale, x0, y0, pw, ph, pet_place_next_to.x,
                    pet_place_next_to.y);
        }
    }
    SDL_SetRenderVSync(renderer, 0);
    SDL_SetRenderLogicalPresentation(renderer, EMU_WIDTH, EMU_HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_Texture* screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XBGR8888, SDL_TEXTUREACCESS_STREAMING,
                                            EMU_WIDTH, EMU_HEIGHT);
    SDL_SetTextureScaleMode(screen, SDL_SCALEMODE_NEAREST);
    SDL_Log("Renderer: %s", SDL_GetRendererName(renderer));

    Pet pet = { 0 };
    if (!pet_create(&pet)) SDL_Log("PET window failed: %s", SDL_GetError());
    else if (pet_place_next_to.w > 0) {
        SDL_SetWindowSize(pet.window, pet_place_next_to.w, pet_place_next_to.h);
        SDL_SetWindowPosition(pet.window, pet_place_next_to.x, pet_place_next_to.y);
    }
    SDL_RaiseWindow(window);

    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, SAMPLE_RATE };
    SDL_AudioStream* audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (audio) SDL_ResumeAudioStreamDevice(audio);
    else SDL_Log("No audio: %s", SDL_GetError());

    SDL_Gamepad* pad = NULL;
    int16_t samples[4096 * 2];
    bool running = true, fullscreen = false;
    const char* message = NULL;
    Uint64 message_until = 0;
    Uint64 next = SDL_GetTicksNS();
    Uint64 fps_t0 = next;
    uint32_t fps_f0 = emu_frame(emu);
    double fps = 0;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (pet.window && e.window.windowID == SDL_GetWindowID(pet.window)) pet_toggle(&pet);
                else running = false;
                break;
            case SDL_EVENT_GAMEPAD_ADDED:
                if (!pad) pad = SDL_OpenGamepad(e.gdevice.which);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                if (pad && e.gdevice.which == SDL_GetGamepadID(pad)) {
                    SDL_CloseGamepad(pad);
                    pad = NULL;
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                if (e.key.repeat) break;
                if (e.key.key == SDLK_ESCAPE) running = false;
                else if (e.key.key == SDLK_F1 && pet.window) pet_toggle(&pet);
                else if (e.key.key == SDLK_F11) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen);
                } else if (e.key.key == SDLK_F5) {
                    message = emu_save_state(emu, state_path) ? "State saved" : "State save FAILED";
                    message_until = SDL_GetTicksNS() + SDL_NS_PER_SECOND * 2;
                } else if (e.key.key == SDLK_F7) {
                    message = emu_load_state(emu, state_path) ? "State loaded" : "No state to load";
                    message_until = SDL_GetTicksNS() + SDL_NS_PER_SECOND * 2;
                }
                break;
            }
        }
        if (message && SDL_GetTicksNS() > message_until) message = NULL;

        uint32_t keys = keyboard_keys() | gamepad_keys(pad);
        const bool* kb = SDL_GetKeyboardState(NULL);
        bool ff = kb[SDL_SCANCODE_TAB] || gamepad_fast_forward(pad);
        emu_set_keys(emu, keys);

        int frames = ff ? FAST_FORWARD_FRAMES : 1;
        for (int i = 0; i < frames; ++i) {
            emu_run_frame(emu);
            int n = emu_read_audio(emu, samples, 4096);
            if (audio && !ff && n > 0) SDL_PutAudioStreamData(audio, samples, n * 4);
        }

        SDL_UpdateTexture(screen, NULL, emu_video(emu), EMU_WIDTH * 4);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);

        Uint64 now = SDL_GetTicksNS();
        if (now - fps_t0 >= SDL_NS_PER_SECOND / 2) {
            fps = (emu_frame(emu) - fps_f0) * 1e9 / (double) (now - fps_t0);
            fps_t0 = now;
            fps_f0 = emu_frame(emu);
        }
        PetStatus st = { fps, ff, keys, message };
        pet_draw(&pet, emu, game, code, &st);

        /* --quit-after N [--pet-shot file.bmp]: dev check of the windowed
         * path; saves the PET window's own pixels (not the desktop). */
        if (quit_after > 0 && emu_frame(emu) >= (uint32_t) quit_after) {
            if (pet_shot && pet.renderer) {
                pet_render(&pet, emu, game, code, &st);  /* read back before presenting */
                SDL_Surface* s = SDL_RenderReadPixels(pet.renderer, NULL);
                if (s) {
                    SDL_SaveBMP(s, pet_shot);
                    SDL_DestroySurface(s);
                }
            }
            int ww, wh, pw, ph, wx, wy;
            SDL_GetWindowSize(window, &ww, &wh);
            SDL_GetWindowSizeInPixels(window, &pw, &ph);
            SDL_GetWindowPosition(window, &wx, &wy);
            SDL_Log("game window %dx%d (pixels %dx%d) at %d,%d; display scale %.2f, window scale %.2f", ww, wh,
                    pw, ph, wx, wy, SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(window)),
                    SDL_GetWindowDisplayScale(window));
            if (pet.window) {
                SDL_GetWindowSize(pet.window, &ww, &wh);
                SDL_GetWindowSizeInPixels(pet.window, &pw, &ph);
                SDL_GetWindowPosition(pet.window, &wx, &wy);
                SDL_Log("PET window %dx%d (pixels %dx%d) at %d,%d", ww, wh, pw, ph, wx, wy);
            }
            SDL_Log("quit-after %d reached, fps %.2f", quit_after, fps);
            running = false;
        }

        /* Pace to 59.7275 fps; nudge by the audio queue so it neither
         * underruns nor builds latency. */
        if (!ff) {
            double step = EMU_FRAME_NS;
            if (audio) {
                int queued = SDL_GetAudioStreamQueued(audio) / 4;  /* sample frames */
                int target = SAMPLE_RATE / 20;                     /* ~50 ms */
                if (queued > target * 2) step *= 1.02;
                else if (queued < target / 2) step *= 0.98;
            }
            next += (Uint64) step;
            now = SDL_GetTicksNS();
            if (next > now) SDL_DelayPrecise(next - now);
            else if (now - next > (Uint64) (EMU_FRAME_NS * 5)) next = now;
        } else {
            next = SDL_GetTicksNS();
            if (audio) SDL_ClearAudioStream(audio);
        }
    }

    emu_close(emu);  /* writes SRAM */
    if (pad) SDL_CloseGamepad(pad);
    if (audio) SDL_DestroyAudioStream(audio);
    pet_destroy(&pet);
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_free(data_dir);
    SDL_Quit();
    return 0;
}
