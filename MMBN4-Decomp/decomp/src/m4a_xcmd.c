/* DC-5 C (agbcc -O2): Nintendo m4a ply_xcmd & ply_xcmd_00
 * (code_10, RS 0x08113B94-0x08113BC7, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

struct MPlayTrack {
    u8 pad0[0x40];
    u8 *cmdPtr;
};

typedef void (*XcmdFunc)(void *, struct MPlayTrack *);
extern const XcmdFunc dat_0820A42C[];

/* 0x08113B94: ply_xcmd */
void sub_08113B94(void *mplayInfo, struct MPlayTrack *track) {
    u32 n = *track->cmdPtr++;
    dat_0820A42C[n](mplayInfo, track);
}

/* 0x08113BB4: ply_xcmd_00 */
void sub_08113BB4(void *mplayInfo, struct MPlayTrack *track) {
    (*(XcmdFunc *)0x03004650)(mplayInfo, track);
}
