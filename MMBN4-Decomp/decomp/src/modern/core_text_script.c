/*
 * Portable C text/dialogue bytecode decoder for MMBN4 (Phase C4).
 *
 * Covers the MMBN4 text script control-code engine (code_03, ScriptCmd_*):
 *   - Printable glyphs (< 0xE6)
 *   - Standard control codes: 0xE6 (End), 0xE7 (Newline), 0xE8 (WaitKey),
 *     0xE9 (ClearMsg), 0xEA (Jump), 0xEB (Mugshot), 0xEC (Delay),
 *     0xED (TextSpeed), 0xF2 (FlagOp: Test/Set/Clear), 0xF6 (Item/Chip),
 *     0xF8 (Sound/BGM), 0xFC (Extended / Violet Eclipse hooks)
 *   - Safe step-by-step decoding and event-flag branching on Toolkit.
 */
#include "toolkit.h"

u32 EventFlagTest(const struct Toolkit *tk, u32 flagId);
void EventFlagSet(struct Toolkit *tk, u32 flagId);
void EventFlagClear(struct Toolkit *tk, u32 flagId);

enum TextScriptOp {
    TEXT_OP_END       = 0xE6,
    TEXT_OP_NEWLINE   = 0xE7,
    TEXT_OP_WAIT_KEY  = 0xE8,
    TEXT_OP_CLEAR_MSG = 0xE9,
    TEXT_OP_JUMP      = 0xEA,
    TEXT_OP_MUGSHOT   = 0xEB,
    TEXT_OP_DELAY     = 0xEC,
    TEXT_OP_SPEED     = 0xED,
    TEXT_OP_FLAG      = 0xF2,
    TEXT_OP_ITEM      = 0xF6,
    TEXT_OP_SOUND     = 0xF8,
    TEXT_OP_EXT       = 0xFC
};

enum TextScriptStepResult {
    TEXT_STEP_CHAR    = 0,
    TEXT_STEP_CONTROL = 1,
    TEXT_STEP_WAIT    = 2,
    TEXT_STEP_END     = 3,
    TEXT_STEP_ERR     = 4
};

struct TextScriptState {
    const u8 *buf;
    u32 len;
    u32 pos;
    u8 mugshotId;
    u8 textSpeed;
    u8 delayFrames;
    u8 lastOp;
    u16 lastSoundId;
    u16 lastExtArg;
    u32 lineCount;
};

static u16 ReadLe16(const u8 *p) {
    return (u16)((u16)p[0] | ((u16)p[1] << 8));
}

void TextScriptInit(struct TextScriptState *st, const u8 *buf, u32 len) {
    st->buf = buf;
    st->len = len;
    st->pos = 0;
    st->mugshotId = 0xFF;
    st->textSpeed = 1;
    st->delayFrames = 0;
    st->lastOp = 0;
    st->lastSoundId = 0;
    st->lastExtArg = 0;
    st->lineCount = 0;
}

/*
 * Return byte length of the instruction starting at buf[pos], or 0 on
 * truncated/out-of-bounds input.
 */
u32 TextScriptInstrLen(const u8 *buf, u32 len, u32 pos) {
    u8 op;

    if (pos >= len) {
        return 0;
    }
    op = buf[pos];
    if (op < TEXT_OP_END) {
        return 1;
    }
    switch (op) {
    case TEXT_OP_END:
    case TEXT_OP_NEWLINE:
    case TEXT_OP_WAIT_KEY:
    case TEXT_OP_CLEAR_MSG:
        return 1;
    case TEXT_OP_JUMP:
    case TEXT_OP_MUGSHOT:
    case TEXT_OP_DELAY:
    case TEXT_OP_SPEED:
        return (pos + 2 <= len) ? 2 : 0;
    case TEXT_OP_SOUND:
        return (pos + 3 <= len) ? 3 : 0;
    case TEXT_OP_FLAG:
        /* F2 subop(1) flagId(2) [jumpTarget(1) for test subops 0/1] */
        if (pos + 4 > len) {
            return 0;
        }
        if (buf[pos + 1] <= 1) {
            return (pos + 5 <= len) ? 5 : 0;
        }
        return 4;
    case TEXT_OP_ITEM:
        return (pos + 4 <= len) ? 4 : 0;
    case TEXT_OP_EXT:
        /* FC subop(1) arg(1) */
        return (pos + 3 <= len) ? 3 : 0;
    default:
        return 1;
    }
}

/*
 * Step one glyph or control code in the dialogue stream.
 * If tk != NULL, TEXT_OP_FLAG (0xF2) reads/mutates event flags directly:
 *   subop 0: set flag; subop 1: clear flag (handled as 2/3 below)
 *   subop 0: if flag set -> lastExtArg = target; subop 1: if flag clear -> target
 *   subop 2: set flag; subop 3: clear flag.
 */
u32 TextScriptStep(struct TextScriptState *st, struct Toolkit *tk, u8 *outGlyph) {
    u32 sz;
    u8 op;

    sz = TextScriptInstrLen(st->buf, st->len, st->pos);
    if (sz == 0) {
        return TEXT_STEP_ERR;
    }
    op = st->buf[st->pos];
    st->lastOp = op;

    if (op < TEXT_OP_END) {
        if (outGlyph != 0) {
            *outGlyph = op;
        }
        st->pos += 1;
        return TEXT_STEP_CHAR;
    }

    switch (op) {
    case TEXT_OP_END:
        st->pos += 1;
        return TEXT_STEP_END;
    case TEXT_OP_NEWLINE:
        st->lineCount += 1;
        st->pos += 1;
        return TEXT_STEP_CONTROL;
    case TEXT_OP_WAIT_KEY:
        st->pos += 1;
        return TEXT_STEP_WAIT;
    case TEXT_OP_CLEAR_MSG:
        st->lineCount = 0;
        st->pos += 1;
        return TEXT_STEP_CONTROL;
    case TEXT_OP_JUMP:
        st->lastExtArg = st->buf[st->pos + 1];
        st->pos += 2;
        return TEXT_STEP_END;
    case TEXT_OP_MUGSHOT:
        st->mugshotId = st->buf[st->pos + 1];
        st->pos += 2;
        return TEXT_STEP_CONTROL;
    case TEXT_OP_DELAY:
        st->delayFrames = st->buf[st->pos + 1];
        st->pos += 2;
        return TEXT_STEP_WAIT;
    case TEXT_OP_SPEED:
        st->textSpeed = st->buf[st->pos + 1];
        st->pos += 2;
        return TEXT_STEP_CONTROL;
    case TEXT_OP_SOUND:
        st->lastSoundId = ReadLe16(&st->buf[st->pos + 1]);
        st->pos += 3;
        return TEXT_STEP_CONTROL;
    case TEXT_OP_FLAG: {
        u8 sub = st->buf[st->pos + 1];
        u16 flagId = ReadLe16(&st->buf[st->pos + 2]);
        if (tk != 0) {
            if (sub == 0) {
                if (EventFlagTest(tk, flagId)) {
                    st->lastExtArg = st->buf[st->pos + 4];
                    st->pos += sz;
                    return TEXT_STEP_END;
                }
            } else if (sub == 1) {
                if (!EventFlagTest(tk, flagId)) {
                    st->lastExtArg = st->buf[st->pos + 4];
                    st->pos += sz;
                    return TEXT_STEP_END;
                }
            } else if (sub == 2) {
                EventFlagSet(tk, flagId);
            } else if (sub == 3) {
                EventFlagClear(tk, flagId);
            }
        }
        st->pos += sz;
        return TEXT_STEP_CONTROL;
    }
    case TEXT_OP_EXT:
        st->lastExtArg = (u16)(((u16)st->buf[st->pos + 1] << 8) | st->buf[st->pos + 2]);
        st->pos += 3;
        return TEXT_STEP_CONTROL;
    default:
        st->pos += sz;
        return TEXT_STEP_CONTROL;
    }
}
