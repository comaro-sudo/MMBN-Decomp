/* MMBN4 Violet Eclipse - New 3DS port (3DS-1..3DS-3).
 *
 * Top screen (400x240)   : GBA game (240x160 centered, 1:1 pixel crisp)
 * Bottom screen (320x240): Interactive 4-tab PET via citro2d + touchscreen
 *
 * SD card layout:
 *   sdmc:/3ds/mmbn4/mmbn4.3dsx
 *   sdmc:/3ds/mmbn4/bn4_rs_overhaul.gba (or bn4_bm_overhaul.gba / bn4_rs_us.gba)
 *   sdmc:/3ds/mmbn4/saves/<code>.sav
 *
 * Real-hardware benchmark (Gate G-3DS: < 14 ms mean on New 3DS 804 MHz):
 *   Hold L + R while launching (or create sdmc:/3ds/mmbn4/bench.txt).
 *   Writes sdmc:/3ds/mmbn4/bench.log.
 */
#include <3ds.h>
#include <citro2d.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bench.h"
#include "emu_backend.h"
#include "game.h"
#include "pet_model.h"
#include "pet_ui.h"
#include "ui_draw_c2d.h"

#define SAMPLE_RATE 32768
#define APP_DIR     "sdmc:/3ds/mmbn4"
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

static uint64_t n3ds_now_ns(void) {
    return (uint64_t) osGetTime() * 1000000ULL;
}

static uint32_t hid_to_gba_keys(u32 h) {
    uint32_t keys = 0;
    if (h & KEY_A)       keys |= EMU_KEY_A;
    if (h & KEY_B)       keys |= EMU_KEY_B;
    if (h & KEY_SELECT)  keys |= EMU_KEY_SELECT;
    if (h & KEY_START)   keys |= EMU_KEY_START;
    if (h & KEY_DRIGHT)  keys |= EMU_KEY_RIGHT;
    if (h & KEY_DLEFT)   keys |= EMU_KEY_LEFT;
    if (h & KEY_DUP)     keys |= EMU_KEY_UP;
    if (h & KEY_DDOWN)   keys |= EMU_KEY_DOWN;
    if (h & KEY_R)       keys |= EMU_KEY_R;
    if (h & KEY_L)       keys |= EMU_KEY_L;
    return keys;
}

/* Blit 240x160 XBGR8888 frame centered into 3DS top framebuffer (rotated 90 CCW, BGR8). */
static void blit_top_fb(const uint32_t* src) {
    u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    const int ox = (400 - EMU_WIDTH) / 2;
    const int oy = (240 - EMU_HEIGHT) / 2;
    for (int y = 0; y < EMU_HEIGHT; ++y) {
        for (int x = 0; x < EMU_WIDTH; ++x) {
            uint32_t px = src[y * EMU_WIDTH + x];
            int dx = ox + x;
            int dy = 239 - (oy + y);
            size_t off = (size_t) (dx * 240 + dy) * 3u;
            fb[off + 0] = (u8) ((px >> 16) & 0xFFu);
            fb[off + 1] = (u8) ((px >> 8) & 0xFFu);
            fb[off + 2] = (u8) (px & 0xFFu);
        }
    }
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    osSetSpeedupEnable(true);  /* Unlock New 3DS 804 MHz + L2 cache */
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    mkdir(SAVES_DIR, 0777);
    const char* rom = find_rom();
    if (!rom) {
        C2D_Fini(); C3D_Fini(); gfxExit();
        return 1;
    }

    char short_code[5] = "GAME";
    {
        FILE* rf = fopen(rom, "rb");
        if (rf && fseek(rf, 0xAC, SEEK_SET) == 0 && fread(short_code, 1, 4, rf) == 4) short_code[4] = '\0';
        if (rf) fclose(rf);
    }
    char save_path[256];
    snprintf(save_path, sizeof save_path, SAVES_DIR "/%s.sav", short_code);

    Emu* emu = emu_open(rom, save_path, SAMPLE_RATE);
    if (!emu) {
        C2D_Fini(); C3D_Fini(); gfxExit();
        return 2;
    }

    char code[16];
    emu_game_code(emu, code);
    const GameInfo* game = game_identify(code);

    hidScanInput();
    u32 held0 = hidKeysHeld();
    if (file_exists(APP_DIR "/bench.txt") || ((held0 & (KEY_L | KEY_R)) == (KEY_L | KEY_R))) {
        const char* state = file_exists(APP_DIR "/bench.ss") ? (APP_DIR "/bench.ss") : NULL;
        BenchResult res = { 0 };
        if (bench_run(emu, state, 600, n3ds_now_ns, &res)) {
            FILE* f = fopen(APP_DIR "/bench.log", "w");
            if (f) {
                fprintf(f, "New 3DS hardware benchmark (804 MHz, mGBA 0.10.5)\n");
                fprintf(f, "game=%s (%s) frames=%d mean_ms=%.3f p90_ms=%.3f p99_ms=%.3f max_ms=%.3f fps=%.1f\n",
                        code, game->short_name, res.frames, res.mean_ms, res.p90_ms, res.p99_ms, res.max_ms, res.fps);
                fclose(f);
            }
        }
    }

    UiDrawC2D c2d = {
        C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT),
        C2D_TextBufNew(2048),
    };
    PetUiState pet_ui;
    pet_ui_init(&pet_ui);
    PetModel pet_model = { 0 };
    const char* message = NULL;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kdown = hidKeysDown();
        u32 kheld = hidKeysHeld();
        if ((kheld & KEY_START) && (kheld & KEY_SELECT)) break;

        if (kdown & KEY_TOUCH) {
            touchPosition tp;
            hidTouchRead(&tp);
            const char* act = pet_ui_click(&pet_ui, &pet_model, PET_LAYOUT_QVGA, tp.px, tp.py, emu, game);
            if (act) message = act;
        }

        uint32_t keys = hid_to_gba_keys(kheld) | pet_ui_pop_keys(&pet_ui);
        emu_set_keys(emu, keys);
        emu_run_frame(emu);

        blit_top_fb(emu_video(emu));
        gfxFlushBuffers();

        PetStatus st = { 59.7, false, keys, message };
        pet_model_update(&pet_model, emu, game, code, &st);
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        UiDraw ui = ui_draw_c2d(&c2d);
        pet_ui_draw(&ui, &pet_model, &pet_ui, PET_LAYOUT_QVGA);
        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(c2d.text_buf);
    emu_close(emu);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
