/*
 * Portable C PET Menu & Scrolling List Cursor Controller (Phase C4).
 *
 * Models MMBN4's PET menu and scrollable list navigation (Folder, Pack,
 * Library, Mail, Shop): cursor wrap-around, L/R page jumps, and visible
 * window scroll-offset clamping.
 */
#include "modern_api.h"

/*
 * Initialize a scrolling list cursor with `itemCount` total items and
 * `visibleRows` rows displayed in the viewport.
 */
void MenuCursorInit(struct MenuCursorState *mc, u16 itemCount, u16 visibleRows) {
    mc->cursor = 0u;
    mc->scrollTop = 0u;
    mc->itemCount = itemCount;
    mc->visibleRows = (visibleRows == 0u) ? 1u : visibleRows;
}

static void MenuSyncScroll(struct MenuCursorState *mc) {
    if (mc->cursor < mc->scrollTop) {
        mc->scrollTop = mc->cursor;
    } else if (mc->cursor >= (u16)(mc->scrollTop + mc->visibleRows)) {
        mc->scrollTop = (u16)(mc->cursor - mc->visibleRows + 1u);
    }
}

/*
 * Move cursor by `delta` (+1 down, -1 up) with wrap-around at list ends.
 */
void MenuCursorMove(struct MenuCursorState *mc, s32 delta) {
    s32 next;

    if (mc == 0 || mc->itemCount == 0u) {
        return;
    }
    next = (s32)mc->cursor + delta;
    while (next < 0) {
        next += (s32)mc->itemCount;
    }
    while (next >= (s32)mc->itemCount) {
        next -= (s32)mc->itemCount;
    }
    mc->cursor = (u16)next;
    MenuSyncScroll(mc);
}

/*
 * Jump cursor by one full page (L = -1, R = +1), clamped to [0..itemCount-1].
 */
void MenuCursorPage(struct MenuCursorState *mc, s32 pageDir) {
    s32 next;

    if (mc == 0 || mc->itemCount == 0u) {
        return;
    }
    next = (s32)mc->cursor + pageDir * (s32)mc->visibleRows;
    if (next < 0) {
        next = 0;
    }
    if (next >= (s32)mc->itemCount) {
        next = (s32)mc->itemCount - 1;
    }
    mc->cursor = (u16)next;
    MenuSyncScroll(mc);
}

/*
 * Return the cursor's row index relative to the visible viewport (0..visibleRows-1).
 */
u16 MenuCursorVisibleRow(const struct MenuCursorState *mc) {
    if (mc == 0 || mc->cursor < mc->scrollTop) {
        return 0u;
    }
    return (u16)(mc->cursor - mc->scrollTop);
}
