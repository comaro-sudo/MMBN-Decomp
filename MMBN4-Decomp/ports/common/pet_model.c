#include "pet_model.h"

#include <string.h>

static bool read_event_flag(Emu* emu, uint32_t flags_base, uint32_t flag) {
    uint32_t byte_val = emu_read8(emu, flags_base + (flag >> 3));
    return (byte_val & (0x80u >> (flag & 7u))) != 0;
}

static void write_event_flag(Emu* emu, uint32_t flags_base, uint32_t flag, bool set) {
    uint32_t addr = flags_base + (flag >> 3);
    uint8_t mask = (uint8_t) (0x80u >> (flag & 7u));
    uint8_t v = (uint8_t) emu_read8(emu, addr);
    if (set) v |= mask;
    else v &= (uint8_t) ~mask;
    emu_write8(emu, addr, v);
}

void pet_model_update(PetModel* m, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st) {
    memset(m, 0, sizeof(*m));
    if (code) {
        strncpy(m->game_code, code, sizeof(m->game_code) - 1);
        m->game_code[sizeof(m->game_code) - 1] = '\0';
    }
    m->short_name = game ? game->short_name : "Unknown";
    m->crc32 = emu_crc32(emu);
    m->frame = emu_frame(emu);
    if (st) m->status = *st;

    if (!game || !game->ram_rng || !(emu_caps(emu) & EMU_CAP_MEMORY)) return;
    m->has_ram = true;
    m->rng = emu_read32(emu, game->ram_rng);
    m->battle_hp_p1 = (uint16_t) emu_read16(emu, game->ram_battle_navi + 0x30);
    m->battle_hp_p2 = (uint16_t) emu_read16(emu, game->ram_battle_navi + 0x40 + 0x30);

    if (!game->ram_save_base) return;
    uint32_t shift = emu_read32(emu, game->ram_save_shift) & 0x1FEu;
    uint32_t sb = game->ram_save_base + shift;

    uint16_t cur_hp = (uint16_t) emu_read16(emu, sb + game->off_cur_hp);
    uint16_t max_hp = (uint16_t) emu_read16(emu, sb + game->off_max_hp);
    if (max_hp >= 100 && max_hp <= 9999 && cur_hp <= max_hp) {
        m->save_valid = true;
        m->save_block = sb;
        m->cur_hp = cur_hp;
        m->max_hp = max_hp;
        m->zenny = emu_read32(emu, sb + game->off_zenny);
        m->bugfrags = emu_read32(emu, sb + game->off_bugfrags);
        m->map_group = (uint8_t) emu_read8(emu, sb + game->off_map_group);
        m->map_id = (uint8_t) emu_read8(emu, sb + game->off_map_id);
        m->map_name = game_map_name(m->map_group, m->map_id);
        m->playthrough = (uint8_t) ((emu_read8(emu, sb + game->off_playthrough) & 0xFFu) + 1u);

        uint32_t flags_base = sb + game->off_flags;
        uint16_t mask = 0;
        uint8_t count = 0;
        for (uint32_t i = 0; i < 12; ++i) {
            if (read_event_flag(emu, flags_base, 0x0017u + i)) {
                mask |= (uint16_t) (1u << i);
                ++count;
            }
        }
        m->souls_mask = mask;
        m->souls_count = count;

        uint8_t t = 0;
        if (read_event_flag(emu, flags_base, 0x0E58u)) t |= 1u;
        if (read_event_flag(emu, flags_base, 0x0E59u)) t |= 2u;
        m->tuner_mode = t;

        if (game->off_folder1) {
            uint32_t fbase = sb + game->off_folder1;
            for (int i = 0; i < PET_FOLDER_SIZE; ++i) {
                uint16_t w = (uint16_t) emu_read16(emu, fbase + (uint32_t) i * 2u);
                uint16_t cid = (uint16_t) (w & 0x1FFu);
                m->folder[i].chip_id = cid;
                m->folder[i].code    = (uint8_t) ((w >> 9) & 0x1Fu);
                if (game->rom_chip_table && cid > 0 && cid < 389) {
                    uint32_t entry = game->rom_chip_table + (uint32_t) cid * 0x2Cu;
                    m->folder[i].element = (uint8_t) emu_read8(emu, entry + 0x04u);
                    m->folder[i].damage  = (uint16_t) emu_read16(emu, entry + 0x1Au);
                }
            }
        }
    }
}

bool pet_model_set_tuner(Emu* emu, const GameInfo* game, uint8_t mode) {
    if (!emu || !game || !game->ram_save_base || !(emu_caps(emu) & EMU_CAP_MEMORY)) return false;
    uint32_t shift = emu_read32(emu, game->ram_save_shift) & 0x1FEu;
    uint32_t flags_base = game->ram_save_base + shift + game->off_flags;
    write_event_flag(emu, flags_base, 0x0E58u, (mode & 1u) != 0);
    write_event_flag(emu, flags_base, 0x0E59u, (mode & 2u) != 0);
    return true;
}
