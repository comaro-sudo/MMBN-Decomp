/* Portable PET screen renderer and touch/mouse controller.
 * Draws purely through UiDraw so PC (SDL3), Wii U (SDL2/GX2 GamePad), and
 * New 3DS (citro2d bottom screen) share one interactive UI. */
#ifndef MMBN4_PET_UI_H
#define MMBN4_PET_UI_H

#include <stdbool.h>
#include <stdint.h>

#include "emu_backend.h"
#include "game.h"
#include "pet_model.h"
#include "ui_draw.h"

typedef enum {
    PET_LAYOUT_QVGA = 0,  /* 320x240 (3DS bottom screen, default PC PET) */
    PET_LAYOUT_WIDE = 1,  /* 854x480 (Wii U GamePad) */
} PetLayout;

typedef enum {
    PET_TAB_STATUS = 0,
    PET_TAB_FOLDER = 1,
    PET_TAB_MAP    = 2,
    PET_TAB_CUSTOM = 3,
    PET_TAB_COUNT  = 4,
} PetTab;

#define PET_QVGA_W 320
#define PET_QVGA_H 240
#define PET_WIDE_W 854
#define PET_WIDE_H 480

#define PET_KEY_QUEUE_CAP 32

typedef struct {
    uint32_t keys;
    uint8_t hold_frames;
    uint8_t gap_frames;
} PetKeyPulse;

typedef struct {
    PetTab active_tab;
    uint8_t folder_page;      /* 0 = chips 1..15, 1 = chips 16..30 */
    uint8_t custom_col;       /* tracked Custom Screen cursor (0..4) */
    uint8_t custom_row;       /* tracked Custom Screen cursor (0..1) */

    /* Touch-injected GBA key pulse FIFO */
    PetKeyPulse queue[PET_KEY_QUEUE_CAP];
    uint8_t q_head;
    uint8_t q_tail;
    uint8_t q_countdown;
    bool q_in_gap;

    char last_action[64];
} PetUiState;

void pet_ui_init(PetUiState* st);
void pet_ui_draw(const UiDraw* ui, const PetModel* m, const PetUiState* st, PetLayout layout);

/* Handles a tap/click at logical coordinates (x, y) inside the PET viewport.
 * Returns a short status string (pointing into st->last_action) if handled,
 * or NULL if nothing was hit. */
const char* pet_ui_click(PetUiState* st, const PetModel* m, PetLayout layout, int x, int y,
                         Emu* emu, const GameInfo* game);

/* Advance the touch-injected key queue by 1 frame and return GBA key bits
 * to OR into emu_set_keys(). */
uint32_t pet_ui_pop_keys(PetUiState* st);

#endif
