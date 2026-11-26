/*
 * Portable C PET Mail / Inbox Manager (Phase C4).
 *
 * Models MMBN4's PET Mail bitfields (received, read, attachment claimed)
 * and unread-mail indicator logic.
 */
#include "gba_types.h"

#define MAIL_MAX_COUNT 64u
#define MAIL_BIT_BYTES (MAIL_MAX_COUNT / 8u)

struct MailState {
    u8 received[MAIL_BIT_BYTES];
    u8 read[MAIL_BIT_BYTES];
    u8 claimed[MAIL_BIT_BYTES];
};

static u32 MailBitGet(const u8 *arr, u32 id) {
    return (arr[id >> 3] & (u8)(1u << (id & 7u))) ? 1u : 0u;
}

static void MailBitSet(u8 *arr, u32 id) {
    arr[id >> 3] |= (u8)(1u << (id & 7u));
}

/*
 * Deliver mail `id` to the player's inbox. Returns 1 if newly delivered,
 * 0 if already received or out of range.
 */
u32 MailDeliver(struct MailState *ms, u32 id) {
    if (ms == 0 || id >= MAIL_MAX_COUNT || MailBitGet(ms->received, id)) {
        return 0;
    }
    MailBitSet(ms->received, id);
    return 1;
}

/*
 * Mark received mail `id` as read. Returns 1 if newly read, 0 otherwise.
 */
u32 MailMarkRead(struct MailState *ms, u32 id) {
    if (ms == 0 || id >= MAIL_MAX_COUNT
     || !MailBitGet(ms->received, id) || MailBitGet(ms->read, id)) {
        return 0;
    }
    MailBitSet(ms->read, id);
    return 1;
}

/*
 * Claim a mail attachment (chip/item) once. Returns 1 if claimed now, 0 otherwise.
 */
u32 MailClaimAttachment(struct MailState *ms, u32 id) {
    if (ms == 0 || id >= MAIL_MAX_COUNT
     || !MailBitGet(ms->received, id) || MailBitGet(ms->claimed, id)) {
        return 0;
    }
    MailBitSet(ms->claimed, id);
    return 1;
}

/*
 * Count unread received mails (drives the flashing PET Mail icon).
 */
u32 MailCountUnread(const struct MailState *ms) {
    u32 i;
    u32 unread = 0;

    if (ms == 0) {
        return 0;
    }
    for (i = 0; i < MAIL_MAX_COUNT; i++) {
        if (MailBitGet(ms->received, i) && !MailBitGet(ms->read, i)) {
            unread++;
        }
    }
    return unread;
}
