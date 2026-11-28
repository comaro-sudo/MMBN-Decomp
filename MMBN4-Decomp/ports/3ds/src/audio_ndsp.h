/* 3DS audio output: GBA stereo s16 -> ndsp channel 0 (libctru). */
#ifndef MMBN4_3DS_AUDIO_NDSP_H
#define MMBN4_3DS_AUDIO_NDSP_H

#include <stdbool.h>

#include "emu_backend.h"

/* Starts ndsp. Returns false when ndsp is unavailable (usually
 * sdmc:/3ds/dspfirm.cdc is missing: dump it with DSP1); the other functions
 * then still work and just drain the emulator's audio. Call once, after
 * emu_open(). */
bool audio_init(void);
/* audio_init() also registers an aptHook (when ndsp is up) so audio_pump()
 * restarts the channel after sleep or the HOME menu. Emulation itself pauses
 * because aptMainLoop() does not return while the app is suspended. */

/* Call once per emu_run_frame(): moves that frame's audio into a free wave
 * buffer and queues it. If both buffers are still busy the frame's audio is
 * dropped. Always drains the emulator, so mGBA's blip buffers never fill. */
void audio_pump(Emu* emu);

/* Frames of audio dropped because the queue was full (diagnostics). */
unsigned audio_dropped(void);

/* Times the channel was rebuilt after sleep / HOME menu (diagnostics). */
unsigned audio_resyncs(void);

void audio_exit(void);

#endif
