/* Portable PET data model populated once per frame from emulator RAM.
 * Contains zero platform or rendering dependencies. */
#ifndef MMBN4_PET_MODEL_H
#define MMBN4_PET_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#include "emu_backend.h"
#include "game.h"

typedef struct {
    double fps;
    bool fast_forward;
    uint32_t keys;
    const char* message;  /* last status message, may be NULL */
} PetStatus;

#define PET_FOLDER_SIZE 30

typedef struct {
    uint16_t chip_id;  /* 0..511 */
    uint8_t code;      /* 0..25 = A..Z, 26 = '*' */
    uint8_t element;   /* 0=None, 1=Fire, 2=Aqua, 3=Elec, 4=Wood */
    uint16_t damage;   /* live base ATK from BN4_CHIP_TABLE in ROM */
} PetChip;

typedef struct {
    char game_code[16];
    const char* short_name;
    uint32_t crc32;
    uint32_t frame;

    bool has_ram;
    uint32_t rng;
    uint16_t battle_hp_p1;
    uint16_t battle_hp_p2;

    bool save_valid;
    uint32_t save_block;   /* active relocated EWRAM base */
    uint16_t cur_hp;
    uint16_t max_hp;
    uint32_t zenny;
    uint32_t bugfrags;
    uint8_t map_group;
    uint8_t map_id;
    const char* map_name;
    uint8_t playthrough;   /* 1-based display value (1..3+) */
    uint16_t souls_mask;   /* 12 bits for Double Soul flags 0x0017..0x0022 */
    uint8_t souls_count;   /* 0..12 */
    uint8_t tuner_mode;    /* 0=Auto, 1=T1, 2=T2, 3=T3 (flags 0x0E58..0x0E59) */
    PetChip folder[PET_FOLDER_SIZE];

    PetStatus status;
} PetModel;

void pet_model_update(PetModel* m, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st);

/* Writes the Frequency Tuner mode (0=Auto, 1=T1, 2=T2, 3=T3) directly to
 * event flags 0x0E58..0x0E59 in EWRAM. Returns true on success. */
bool pet_model_set_tuner(Emu* emu, const GameInfo* game, uint8_t mode);

#endif
