/* Portable PET screen renderer. Draws purely through UiDraw so PC (SDL3),
 * Wii U (GX2 GamePad), and New 3DS (citro2d bottom screen) share one UI. */
#ifndef MMBN4_PET_UI_H
#define MMBN4_PET_UI_H

#include "pet_model.h"
#include "ui_draw.h"

typedef enum {
    PET_LAYOUT_QVGA = 0,  /* 320x240 (3DS bottom screen, default PC PET) */
    PET_LAYOUT_WIDE = 1,  /* 854x480 (Wii U GamePad) */
} PetLayout;

#define PET_QVGA_W 320
#define PET_QVGA_H 240
#define PET_WIDE_W 854
#define PET_WIDE_H 480

void pet_ui_draw(const UiDraw* ui, const PetModel* m, PetLayout layout);

#endif
