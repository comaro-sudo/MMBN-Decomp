/* ndsp audio backend for the 3DS port.
 *
 * Design: one ndsp channel (0), stereo PCM16, AUDIO_NBUF wave buffers
 * (default 2, double buffering) in one linearAlloc block. Each pump moves the
 * audio of one emulated frame (~549 sample frames at 32768 Hz) into the next
 * buffer and queues it with ndspChnWaveBufAdd. A buffer is refilled only when
 * libctru reports it NDSP_WBUF_FREE or NDSP_WBUF_DONE; otherwise the frame is
 * dropped. The main loop is paced by the display, so emulation and playback
 * stay locked on average, and the drop path absorbs the small rate mismatch.
 *
 * Sleep / HOME menu: libctru's ndsp already puts the DSP to sleep and wakes it
 * itself (dspHook in source/ndsp/ndsp.c), restoring the channel parameters.
 * What it does not do is keep our wave buffer queue meaningful: the buffers
 * that were queued or playing when the DSP went away have an undefined status
 * afterwards. So an aptHook only sets a flag, and the next audio_pump() (on
 * the main thread, after aptMainLoop() has returned) clears the queue and
 * restarts the channel from scratch. No ndsp call is made inside the hook.
 */
#include <3ds.h>

#include <stdint.h>
#include <string.h>

#include "audio_ndsp.h"

#define AUDIO_RATE   32768.0f /* same rate main.c passes to emu_open() */
#define AUDIO_NBUF   2        /* 3-4 trades latency for fewer underruns */
#define AUDIO_CAP    1024     /* sample frames per buffer (one frame is ~549) */
#define AUDIO_CHANS  2

static bool g_ok;
static volatile bool g_resync;   /* set by the APT hook, cleared by audio_pump */
static unsigned g_resyncs;
static aptHookCookie g_cookie;
static int g_next;
static unsigned g_dropped;
static int16_t* g_pcm; /* linear memory, AUDIO_NBUF * AUDIO_CAP * 2 s16 */
static ndspWaveBuf g_wb[AUDIO_NBUF];
static int16_t g_scratch[AUDIO_CAP * AUDIO_CHANS];

static void chn_setup(void) {
    ndspChnReset(0);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, AUDIO_RATE);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

    float mix[12];
    memset(mix, 0, sizeof mix);
    mix[0] = 1.0f; /* front left  */
    mix[1] = 1.0f; /* front right */
    ndspChnSetMix(0, mix);
}

/* Runs on the main thread, inside aptMainLoop() (aptHandleSleep,
 * aptJumpToHomeMenu / aptWaitForWakeUp in libctru source/services/apt.c), so
 * it must only record the event. */
static void apt_event(APT_HookType hook, void* param) {
    (void) param;
    switch (hook) {
    case APTHOOK_ONSUSPEND:
    case APTHOOK_ONSLEEP:
    case APTHOOK_ONRESTORE:
    case APTHOOK_ONWAKEUP:
        g_resync = true;
        break;
    default:
        break; /* APTHOOK_ONEXIT: audio_exit() runs right after the loop */
    }
}

/* Drop everything queued before the DSP went away and start again. */
static void resync(void) {
    g_resync = false;
    ndspChnWaveBufClear(0); /* marks queued buffers NDSP_WBUF_DONE */
    chn_setup();            /* ndspChnReset + parameters */
    for (int i = 0; i < AUDIO_NBUF; ++i) g_wb[i].status = NDSP_WBUF_FREE;
    memset(g_pcm, 0, (size_t) AUDIO_NBUF * AUDIO_CAP * AUDIO_CHANS * sizeof(int16_t));
    g_next = 0;
    ++g_resyncs;
}

bool audio_init(void) {
    const size_t bytes = (size_t) AUDIO_NBUF * AUDIO_CAP * AUDIO_CHANS * sizeof(int16_t);
    g_ok = false;
    g_next = 0;
    g_dropped = 0;

    /* Fails (non-zero Result) when sdmc:/3ds/dspfirm.cdc is not on the SD card. */
    if (R_FAILED(ndspInit())) return false;

    g_pcm = (int16_t*) linearAlloc(bytes);
    if (!g_pcm) {
        ndspExit();
        return false;
    }
    memset(g_pcm, 0, bytes);

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    chn_setup();

    memset(g_wb, 0, sizeof g_wb);
    for (int i = 0; i < AUDIO_NBUF; ++i) {
        g_wb[i].data_pcm16 = g_pcm + (size_t) i * AUDIO_CAP * AUDIO_CHANS;
        g_wb[i].status = NDSP_WBUF_FREE;
    }
    g_resync = false;
    g_resyncs = 0;
    aptHook(&g_cookie, apt_event, NULL);
    g_ok = true;
    return true;
}

void audio_pump(Emu* emu) {
    if (!g_ok) {
        (void) emu_read_audio(emu, g_scratch, AUDIO_CAP);
        return;
    }

    if (g_resync) {
        /* Back from sleep or the HOME menu: this frame's audio is dropped too,
         * so the first buffer is queued on a clean channel next frame. */
        resync();
        (void) emu_read_audio(emu, g_scratch, AUDIO_CAP);
        return;
    }

    ndspWaveBuf* wb = &g_wb[g_next];
    if (wb->status != NDSP_WBUF_FREE && wb->status != NDSP_WBUF_DONE) {
        /* All buffers busy: playback is behind. Drop this frame's audio. */
        (void) emu_read_audio(emu, g_scratch, AUDIO_CAP);
        ++g_dropped;
        return;
    }

    int n = emu_read_audio(emu, wb->data_pcm16, AUDIO_CAP);
    if (n <= 0) return;
    wb->nsamples = (u32) n; /* PCM16 stereo: one "sample" is one L+R pair */
    DSP_FlushDataCache(wb->data_pcm16, (u32) n * AUDIO_CHANS * sizeof(int16_t));
    ndspChnWaveBufAdd(0, wb);
    g_next = (g_next + 1) % AUDIO_NBUF;
}

unsigned audio_dropped(void) {
    return g_dropped;
}

unsigned audio_resyncs(void) {
    return g_resyncs;
}

void audio_exit(void) {
    if (!g_ok) return;
    aptUnhook(&g_cookie);
    ndspChnWaveBufClear(0);
    ndspExit();
    linearFree(g_pcm);
    g_pcm = NULL;
    g_ok = false;
}
