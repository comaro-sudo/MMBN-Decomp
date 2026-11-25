/* DC-5 C (agbcc): Nintendo m4a MusicPlayer track command handlers
 * (code_10, RS 0x08113C38-0x08113C7F, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon! */

typedef unsigned char u8;

struct MPlayTrack {
    u8 pad0[0x1E];
    u8 modT;        /* +0x1E */
    u8 tune;        /* +0x1F */
    u8 pad1[0x0D];
    u8 lfoSpeed;    /* +0x2D */
    u8 lfoDelay;    /* +0x2E */
    u8 mod;         /* +0x2F */
    u8 pad2[0x10];
    u8 *cmdPtr;     /* +0x40 */
};

/* 0x08113C38: ply_lfos */
void sub_08113C38(void *mplayInfo, struct MPlayTrack *track) {
    track->lfoSpeed = *track->cmdPtr;
    track->cmdPtr++;
}

/* 0x08113C4C: ply_lfodl */
void sub_08113C4C(void *mplayInfo, struct MPlayTrack *track) {
    track->lfoDelay = *track->cmdPtr;
    track->cmdPtr++;
}

/* 0x08113C60: ply_mod */
void sub_08113C60(void *mplayInfo, struct MPlayTrack *track) {
    track->mod = *track->cmdPtr;
    track->cmdPtr++;
}

/* 0x08113C74: ply_modt */
void sub_08113C74(void *mplayInfo, struct MPlayTrack *track) {
    track->modT = *track->cmdPtr++;
}
