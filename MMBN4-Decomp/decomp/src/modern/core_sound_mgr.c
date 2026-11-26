/*
 * Portable C Sound / BGM Manager (Phase C2).
 *
 * Models MMBN4's BN4_PLAY_MUSIC (sub_08000568) & BGM fade/fanfare state
 * transitions on top of GameState::currentBgm so audio control logic is
 * decoupled from GBA m4a hardware registers.
 */
#include "toolkit.h"

#define BGM_NONE 0xFFu

struct SoundMgrState {
    u8 activeBgm;
    u8 savedBgm;
    u8 muted;
    u8 fadeFrames;
    u8 fanfareActive;
};

/*
 * Initialize the sound manager to silence.
 */
void SoundMgrInit(struct SoundMgrState *sm) {
    sm->activeBgm = BGM_NONE;
    sm->savedBgm = BGM_NONE;
    sm->muted = 0u;
    sm->fadeFrames = 0u;
    sm->fanfareActive = 0u;
}

/*
 * Request playback of `trackId` (mirrors BN4_PLAY_MUSIC).
 * If `trackId` is already playing (and not fading/fanfare), returns 0 (no-op).
 * Otherwise updates activeBgm and gs->currentBgm and returns 1 (start track).
 */
u32 SoundMgrPlayBgm(struct SoundMgrState *sm, struct GameState *gs, u8 trackId) {
    if (sm == 0 || sm->muted) {
        return 0;
    }
    if (sm->activeBgm == trackId && !sm->fanfareActive && sm->fadeFrames == 0u) {
        return 0;
    }
    sm->activeBgm = trackId;
    sm->fadeFrames = 0u;
    sm->fanfareActive = 0u;
    if (gs != 0) {
        gs->currentBgm = trackId;
    }
    return 1;
}

/*
 * Trigger a short fanfare (e.g. Virus Busting / Mystery Data jingle),
 * saving the current BGM so SoundMgrResumeBgm can restore it.
 */
void SoundMgrPlayFanfare(struct SoundMgrState *sm, u8 fanfareId) {
    if (sm == 0 || sm->muted) {
        return;
    }
    if (!sm->fanfareActive) {
        sm->savedBgm = sm->activeBgm;
    }
    sm->activeBgm = fanfareId;
    sm->fanfareActive = 1u;
}

/*
 * Resume the BGM saved before SoundMgrPlayFanfare.
 * Returns the restored trackId (or BGM_NONE if none was active).
 */
u8 SoundMgrResumeBgm(struct SoundMgrState *sm, struct GameState *gs) {
    if (sm == 0 || !sm->fanfareActive) {
        return sm ? sm->activeBgm : (u8)BGM_NONE;
    }
    sm->fanfareActive = 0u;
    sm->activeBgm = sm->savedBgm;
    if (gs != 0) {
        gs->currentBgm = sm->activeBgm;
    }
    return sm->activeBgm;
}

/*
 * Begin a fade-out over `frames` ticks and advance it per frame.
 * Returns 1 when the fade completes (and silences activeBgm).
 */
u32 SoundMgrTickFade(struct SoundMgrState *sm) {
    if (sm == 0 || sm->fadeFrames == 0u) {
        return 0;
    }
    sm->fadeFrames--;
    if (sm->fadeFrames == 0u) {
        sm->activeBgm = BGM_NONE;
        return 1;
    }
    return 0;
}
