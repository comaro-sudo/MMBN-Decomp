/* DC-5 C (agbcc -O2): Nintendo m4a MusicPlayer track command handlers
 * (code_10, RS 0x08113C10-0x08113CB3, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon! */

typedef unsigned char u8;

struct MPlayTrack {
    u8 pad0[0x1E];
    u8 modT;        /* +0x1E */
    u8 tune;        /* +0x1F */
    u8 pad1[0x04];
    u8 bendRange;   /* +0x24 */
    u8 pad1b;       /* +0x25 */
    u8 unk26;       /* +0x26 */
    u8 unk27;       /* +0x27 */
    u8 pad2[0x04];
    u8 lfoUnk;      /* +0x2C */
    u8 lfoSpeed;    /* +0x2D */
    u8 lfoDelay;    /* +0x2E */
    u8 mod;         /* +0x2F */
    u8 pad3[0x10];
    u8 *cmdPtr;     /* +0x40 */
};

/* 0x08113C10: ply_bendr */
void sub_08113C10(void *mplayInfo, struct MPlayTrack *track) {
    track->bendRange = *track->cmdPtr;
    track->cmdPtr++;
}

/* 0x08113C24: ply_lfo_2c */
void sub_08113C24(void *mplayInfo, struct MPlayTrack *track) {
    track->lfoUnk = *track->cmdPtr;
    track->cmdPtr++;
}

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

/* 0x08113C80: ply_tune */
void sub_08113C80(void *mplayInfo, struct MPlayTrack *track) {
    track->tune = *track->cmdPtr++;
}

/* 0x08113C8C: ply_unk26 */
void sub_08113C8C(void *mplayInfo, struct MPlayTrack *track) {
    track->unk26 = *track->cmdPtr;
    track->cmdPtr++;
}

/* 0x08113CA0: ply_unk27 */
void sub_08113CA0(void *mplayInfo, struct MPlayTrack *track) {
    track->unk27 = *track->cmdPtr;
    track->cmdPtr++;
}
