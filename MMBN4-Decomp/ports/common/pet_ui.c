#include "pet_ui.h"

#include <stdio.h>
#include <string.h>

#define LINE     10
#define HEADER_H 16
#define TAB_Y    16
#define TAB_H    16
#define BODY_Y   36

static const char* TAB_LABELS[PET_TAB_COUNT] = { "Status", "Folder", "NetMap", "Custom" };

static bool hit(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

static void draw_line(const UiDraw* ui, int x, int* y, const char* s, UiColor c) {
    ui->text(ui->ctx, x, *y, s, c);
    *y += LINE;
}

static const char* tuner_name(uint8_t mode) {
    switch (mode & 3u) {
    case 1:  return "T1";
    case 2:  return "T2";
    case 3:  return "T3";
    default: return "Auto";
    }
}

static char chip_code_char(uint8_t code) {
    if (code < 26) return (char) ('A' + code);
    if (code == 26) return '*';
    return '?';
}

static void queue_pulse(PetUiState* st, uint32_t keys, uint8_t hold, uint8_t gap) {
    uint8_t next = (uint8_t) ((st->q_tail + 1u) % PET_KEY_QUEUE_CAP);
    if (next == st->q_head) return;  /* full */
    st->queue[st->q_tail].keys = keys;
    st->queue[st->q_tail].hold_frames = hold;
    st->queue[st->q_tail].gap_frames = gap;
    st->q_tail = next;
}

void pet_ui_init(PetUiState* st) {
    memset(st, 0, sizeof(*st));
    st->active_tab = PET_TAB_STATUS;
}

uint32_t pet_ui_pop_keys(PetUiState* st) {
    if (st->q_head == st->q_tail) return 0;
    PetKeyPulse* cur = &st->queue[st->q_head];
    if (st->q_countdown == 0) {
        st->q_in_gap = false;
        st->q_countdown = cur->hold_frames ? cur->hold_frames : 2;
    }
    uint32_t out = st->q_in_gap ? 0 : cur->keys;
    --st->q_countdown;
    if (st->q_countdown == 0) {
        if (!st->q_in_gap && cur->gap_frames > 0) {
            st->q_in_gap = true;
            st->q_countdown = cur->gap_frames;
        } else {
            st->q_in_gap = false;
            st->q_head = (uint8_t) ((st->q_head + 1u) % PET_KEY_QUEUE_CAP);
        }
    }
    return out;
}

static void draw_button(const UiDraw* ui, int x, int y, int w, int h, const char* label, bool active) {
    UiColor fill   = active ? ui_rgb(0, 120, 195) : ui_rgb(20, 44, 84);
    UiColor border = active ? ui_rgb(0, 220, 255) : ui_rgb(50, 110, 175);
    UiColor fg     = active ? ui_rgb(255, 255, 255) : ui_rgb(210, 225, 240);
    ui->rect(ui->ctx, x, y, w, h, fill);
    ui->rect_outline(ui->ctx, x, y, w, h, border);
    int tx = x + (w - (int) strlen(label) * 8) / 2;
    int ty = y + (h - 8) / 2;
    if (tx < x + 2) tx = x + 2;
    ui->text(ui->ctx, tx, ty, label, fg);
}

static void draw_status_tab(const UiDraw* ui, const PetModel* m, int w, int h) {
    const UiColor border   = ui_rgb(40, 120, 190);
    const UiColor cyan     = ui_rgb(0, 210, 255);
    const UiColor text_col = ui_rgb(230, 235, 245);
    const UiColor muted    = ui_rgb(150, 170, 200);
    const UiColor hp_bg    = ui_rgb(30, 40, 60);
    const UiColor hp_ok    = ui_rgb(60, 220, 110);
    const UiColor hp_low   = ui_rgb(240, 80, 70);
    const UiColor yellow   = ui_rgb(255, 220, 0);

    char buf[96];
    int y = BODY_Y;

    snprintf(buf, sizeof buf, "Game : %s  %s", m->game_code, m->short_name);
    draw_line(ui, 6, &y, buf, text_col);
    snprintf(buf, sizeof buf, "CRC32: %08X  FPS:%.1f%s", (unsigned) m->crc32,
             m->status.fps, m->status.fast_forward ? " >>" : "");
    draw_line(ui, 6, &y, buf, text_col);

    y += 2;
    ui->line(ui->ctx, 6, y, w - 6, y, border);
    y += 5;

    if (!m->has_ram) {
        draw_line(ui, 6, &y, "(no RAM map for this version)", muted);
    } else if (m->save_valid) {
        snprintf(buf, sizeof buf, "HP: %4u/%4u  PT%u  %s",
                 (unsigned) m->cur_hp, (unsigned) m->max_hp, (unsigned) m->playthrough,
                 m->map_name ? m->map_name : "");
        draw_line(ui, 6, &y, buf, cyan);

        int bar_x = 6, bar_w = w - 12, bar_h = 6;
        int fill = (m->max_hp > 0) ? (int) ((uint32_t) m->cur_hp * (uint32_t) bar_w / m->max_hp) : 0;
        if (fill > bar_w) fill = bar_w;
        UiColor hp_col = (m->cur_hp * 4u <= m->max_hp) ? hp_low : hp_ok;
        ui->rect(ui->ctx, bar_x, y, bar_w, bar_h, hp_bg);
        if (fill > 0) ui->rect(ui->ctx, bar_x, y, fill, bar_h, hp_col);
        ui->rect_outline(ui->ctx, bar_x, y, bar_w, bar_h, border);
        y += bar_h + 5;

        snprintf(buf, sizeof buf, "Zenny: %-9u BugFrags: %u",
                 (unsigned) m->zenny, (unsigned) m->bugfrags);
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "Souls: %2u / 12   Tuner   : %s",
                 (unsigned) m->souls_count, tuner_name(m->tuner_mode));
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "BatHP: %u / %u  RNG:%08X",
                 (unsigned) m->battle_hp_p1, (unsigned) m->battle_hp_p2, (unsigned) m->rng);
        draw_line(ui, 6, &y, buf, muted);

        /* Clickable Frequency Tuner bar */
        y += 4;
        ui->text(ui->ctx, 6, y + 4, "Tuner:", cyan);
        static const char* t_lbl[4] = { "Auto", "T1", "T2", "T3" };
        int bw = (w - 68) / 4;
        for (int i = 0; i < 4; ++i) {
            draw_button(ui, 60 + i * bw, y, bw - 3, 16, t_lbl[i], (m->tuner_mode & 3u) == (uint8_t) i);
        }
    } else {
        draw_line(ui, 6, &y, "Live RAM (title / loading)", cyan);
        snprintf(buf, sizeof buf, "RNG state      : %08X", (unsigned) m->rng);
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "Battle HP P1/P2: %u / %u",
                 (unsigned) m->battle_hp_p1, (unsigned) m->battle_hp_p2);
        draw_line(ui, 6, &y, buf, text_col);
    }

    int fy = h - 4 - LINE * 4;
    ui->line(ui->ctx, 6, fy - 4, w - 6, fy - 4, border);
    draw_line(ui, 6, &fy, "Arrows move  X=A  Z=B  A/S=L/R", muted);
    draw_line(ui, 6, &fy, "Tab=fast  F5/F7=state  F1=PET", muted);
    draw_line(ui, 6, &fy, "Click tabs / Tuner / Custom chips", muted);
    if (m->status.message) draw_line(ui, 6, &fy, m->status.message, yellow);
}

static void draw_folder_tab(const UiDraw* ui, const PetModel* m, const PetUiState* st, int w, int h) {
    const UiColor cyan     = ui_rgb(0, 210, 255);
    const UiColor text_col = ui_rgb(230, 235, 245);
    const UiColor muted    = ui_rgb(150, 170, 200);
    int y = BODY_Y;

    if (!m->save_valid) {
        draw_line(ui, 6, &y, "Folder 1 (load a save to inspect)", muted);
        return;
    }

    char hdr[64];
    int start = (st->folder_page & 1u) ? 15 : 0;
    snprintf(hdr, sizeof hdr, "Folder 1  (Chips %02d-%02d / 30)", start + 1, start + 15);
    ui->text(ui->ctx, 6, y + 3, hdr, cyan);
    draw_button(ui, w - 58, y - 1, 52, 14, (st->folder_page & 1u) ? "1-15" : "16-30", false);
    y += 18;

    /* Two columns of 8 rows (15 chips per page) */
    int col_w = (w - 12) / 2;
    for (int i = 0; i < 15; ++i) {
        int idx = start + i;
        int col = i / 8;
        int row = i % 8;
        char row_txt[32];
        const PetChip* c = &m->folder[idx];
        snprintf(row_txt, sizeof row_txt, "%02d: #%03u [%c]",
                 idx + 1, (unsigned) c->chip_id, chip_code_char(c->code));
        ui->text(ui->ctx, 6 + col * col_w, y + row * 18, row_txt, c->chip_id ? text_col : muted);
    }

    int fy = h - 14;
    ui->text(ui->ctx, 6, fy, "Live EWRAM Folder 1 (click page btn)", muted);
}

static void draw_map_tab(const UiDraw* ui, const PetModel* m, int w, int h) {
    const UiColor border   = ui_rgb(40, 120, 190);
    const UiColor cyan     = ui_rgb(0, 210, 255);
    const UiColor text_col = ui_rgb(230, 235, 245);
    const UiColor muted    = ui_rgb(150, 170, 200);
    const UiColor grid_bg  = ui_rgb(8, 28, 42);
    const UiColor grid_ln  = ui_rgb(20, 62, 92);
    const UiColor node_ok  = ui_rgb(0, 220, 140);
    int y = BODY_Y;

    if (!m->save_valid) {
        draw_line(ui, 6, &y, "Net Map (load a save to inspect)", muted);
        return;
    }

    char buf[96];
    bool is_net = (m->map_group >= 0x80);
    snprintf(buf, sizeof buf, "Area: %s (%02X:%02X)",
             m->map_name ? m->map_name : "Unknown",
             (unsigned) m->map_group, (unsigned) m->map_id);
    draw_line(ui, 6, &y, buf, cyan);
    snprintf(buf, sizeof buf, "Zone: %s   PT%u   Tuner: %s",
             is_net ? "Cyberworld" : "Real World",
             (unsigned) m->playthrough, tuner_name(m->tuner_mode));
    draw_line(ui, 6, &y, buf, text_col);

    /* Cyberworld / Real-World topology overview panel */
    int gx = 6, gy = y + 4, gw = w - 12, gh = h - gy - 26;
    ui->rect(ui->ctx, gx, gy, gw, gh, grid_bg);
    for (int x = gx + 16; x < gx + gw; x += 16) ui->line(ui->ctx, x, gy, x, gy + gh, grid_ln);
    for (int ly = gy + 16; ly < gy + gh; ly += 16) ui->line(ui->ctx, gx, ly, gx + gw, ly, grid_ln);
    ui->rect_outline(ui->ctx, gx, gy, gw, gh, border);

    /* Highlight current network hub node */
    static const struct { const char* name; uint8_t grp; int nx, ny; } NODES[] = {
        { "ACDC",    0x00, 18, 22 },
        { "ElecTwn", 0x01, 46, 22 },
        { "Castlo",  0x02, 76, 22 },
        { "ACDCNet", 0x80, 18, 56 },
        { "TownNet", 0x81, 46, 56 },
        { "ParkNet", 0x82, 76, 56 },
        { "Under",   0x83, 32, 84 },
        { "BlkErth", 0x84, 68, 84 },
    };
    for (size_t i = 0; i < sizeof(NODES) / sizeof(NODES[0]); ++i) {
        int px = gx + (gw * NODES[i].nx) / 100;
        int py = gy + (gh * NODES[i].ny) / 100;
        bool here = (m->map_group == NODES[i].grp);
        ui->rect(ui->ctx, px - 5, py - 5, 10, 10, here ? node_ok : border);
        ui->text(ui->ctx, px - 20, py + 7, NODES[i].name, here ? node_ok : muted);
    }

    /* Bottom Tuner bar */
    int ty = h - 20;
    ui->text(ui->ctx, 6, ty + 4, "Tuner:", cyan);
    static const char* t_lbl[4] = { "Auto", "T1", "T2", "T3" };
    int bw = (w - 68) / 4;
    for (int i = 0; i < 4; ++i) {
        draw_button(ui, 60 + i * bw, ty, bw - 3, 16, t_lbl[i], (m->tuner_mode & 3u) == (uint8_t) i);
    }
}

static void draw_custom_tab(const UiDraw* ui, const PetModel* m, const PetUiState* st, int w, int h) {
    const UiColor cyan     = ui_rgb(0, 210, 255);
    const UiColor muted    = ui_rgb(150, 170, 200);
    int y = BODY_Y;

    char buf[96];
    snprintf(buf, sizeof buf, "Custom Touch Pad   BatHP: %u / %u",
             (unsigned) m->battle_hp_p1, (unsigned) m->battle_hp_p2);
    draw_line(ui, 6, &y, buf, cyan);

    /* 5x2 touch chip grid */
    int gx = 6, gy = y + 4;
    int cell_w = (w - 12) / 5;
    int cell_h = (h - gy - 46) / 2;
    if (cell_h > 44) cell_h = 44;
    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 5; ++c) {
            char lbl[16];
            snprintf(lbl, sizeof lbl, "Chip %d", r * 5 + c + 1);
            if (r * 5 + c + 1 >= 10) snprintf(lbl, sizeof lbl, "Chp %d", r * 5 + c + 1);
            bool sel = (st->custom_col == (uint8_t) c && st->custom_row == (uint8_t) r);
            draw_button(ui, gx + c * cell_w, gy + r * (cell_h + 4), cell_w - 4, cell_h, lbl, sel);
        }
    }

    /* Action buttons: OK (START), Pick (A), Cancel (B), L, R */
    int ay = gy + 2 * (cell_h + 4) + 6;
    static const char* act_lbl[5] = { "OK", "A", "B", "L", "R" };
    int aw = (w - 12) / 5;
    for (int i = 0; i < 5; ++i) {
        draw_button(ui, 6 + i * aw, ay, aw - 4, 22, act_lbl[i], i == 0);
    }
    ui->text(ui->ctx, 6, h - 12, "Click slot or button to inject input", muted);
}

void pet_ui_draw(const UiDraw* ui, const PetModel* m, const PetUiState* st, PetLayout layout) {
    const int w = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_W : PET_QVGA_W;
    const int h = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_H : PET_QVGA_H;

    const UiColor bg     = ui_rgb(10, 24, 48);
    const UiColor header = ui_rgb(18, 42, 82);
    const UiColor border = ui_rgb(40, 120, 190);
    const UiColor cyan   = ui_rgb(0, 210, 255);

    ui->clear(ui->ctx, bg);
    ui->rect(ui->ctx, 0, 0, w, HEADER_H, header);
    ui->text(ui->ctx, 6, 4, "MMBN4 VIOLET ECLIPSE  -  PET", cyan);

    /* Tab bar */
    int tw = w / PET_TAB_COUNT;
    for (int i = 0; i < PET_TAB_COUNT; ++i) {
        draw_button(ui, i * tw, TAB_Y, tw, TAB_H, TAB_LABELS[i], st && st->active_tab == (PetTab) i);
    }
    ui->line(ui->ctx, 0, TAB_Y + TAB_H, w, TAB_Y + TAB_H, border);

    PetTab tab = st ? st->active_tab : PET_TAB_STATUS;
    switch (tab) {
    case PET_TAB_FOLDER: draw_folder_tab(ui, m, st, w, h); break;
    case PET_TAB_MAP:    draw_map_tab(ui, m, w, h); break;
    case PET_TAB_CUSTOM: draw_custom_tab(ui, m, st, w, h); break;
    case PET_TAB_STATUS:
    default:             draw_status_tab(ui, m, w, h); break;
    }
}

const char* pet_ui_click(PetUiState* st, const PetModel* m, PetLayout layout, int x, int y,
                         Emu* emu, const GameInfo* game) {
    const int w = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_W : PET_QVGA_W;
    const int h = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_H : PET_QVGA_H;

    /* 1. Top tab bar */
    if (hit(x, y, 0, TAB_Y, w, TAB_H)) {
        int tw = w / PET_TAB_COUNT;
        int idx = x / (tw > 0 ? tw : 1);
        if (idx >= 0 && idx < PET_TAB_COUNT) {
            st->active_tab = (PetTab) idx;
            snprintf(st->last_action, sizeof st->last_action, "PET tab -> %s", TAB_LABELS[idx]);
            return st->last_action;
        }
    }

    /* 2. Status tab: Frequency Tuner buttons */
    if (st->active_tab == PET_TAB_STATUS && m && m->save_valid) {
        int ty = BODY_Y + 2 * LINE + 7 + LINE + 11 + 3 * LINE + 4;
        int bw = (w - 68) / 4;
        for (int i = 0; i < 4; ++i) {
            if (hit(x, y, 60 + i * bw, ty, bw - 3, 16)) {
                if (pet_model_set_tuner(emu, game, (uint8_t) i)) {
                    snprintf(st->last_action, sizeof st->last_action, "Tuner -> %s", tuner_name((uint8_t) i));
                    return st->last_action;
                }
            }
        }
    }

    /* 3. Folder tab: Page toggle button */
    if (st->active_tab == PET_TAB_FOLDER) {
        if (hit(x, y, w - 58, BODY_Y - 1, 52, 14)) {
            st->folder_page ^= 1u;
            snprintf(st->last_action, sizeof st->last_action, "Folder page %u", (unsigned) (st->folder_page + 1u));
            return st->last_action;
        }
    }

    /* 4. Net Map tab: Frequency Tuner bar at bottom */
    if (st->active_tab == PET_TAB_MAP && m && m->save_valid) {
        int ty = h - 20;
        int bw = (w - 68) / 4;
        for (int i = 0; i < 4; ++i) {
            if (hit(x, y, 60 + i * bw, ty, bw - 3, 16)) {
                if (pet_model_set_tuner(emu, game, (uint8_t) i)) {
                    snprintf(st->last_action, sizeof st->last_action, "Tuner -> %s", tuner_name((uint8_t) i));
                    return st->last_action;
                }
            }
        }
    }

    /* 5. Custom Screen tab: 5x2 chip slots + action buttons */
    if (st->active_tab == PET_TAB_CUSTOM) {
        int gx = 6, gy = BODY_Y + LINE + 4;
        int cell_w = (w - 12) / 5;
        int cell_h = (h - gy - 46) / 2;
        if (cell_h > 44) cell_h = 44;
        for (int r = 0; r < 2; ++r) {
            for (int c = 0; c < 5; ++c) {
                if (hit(x, y, gx + c * cell_w, gy + r * (cell_h + 4), cell_w - 4, cell_h)) {
                    int dc = c - (int) st->custom_col;
                    int dr = r - (int) st->custom_row;
                    while (dc > 0) { queue_pulse(st, EMU_KEY_RIGHT, 2, 2); --dc; }
                    while (dc < 0) { queue_pulse(st, EMU_KEY_LEFT,  2, 2); ++dc; }
                    while (dr > 0) { queue_pulse(st, EMU_KEY_DOWN,  2, 2); --dr; }
                    while (dr < 0) { queue_pulse(st, EMU_KEY_UP,    2, 2); ++dr; }
                    queue_pulse(st, EMU_KEY_A, 3, 2);
                    st->custom_col = (uint8_t) c;
                    st->custom_row = (uint8_t) r;
                    snprintf(st->last_action, sizeof st->last_action, "Custom -> Slot %d", r * 5 + c + 1);
                    return st->last_action;
                }
            }
        }
        int ay = gy + 2 * (cell_h + 4) + 6;
        int aw = (w - 12) / 5;
        static const uint32_t act_key[5] = { EMU_KEY_START, EMU_KEY_A, EMU_KEY_B, EMU_KEY_L, EMU_KEY_R };
        static const char* act_name[5]   = { "OK (START)", "A", "B", "L", "R" };
        for (int i = 0; i < 5; ++i) {
            if (hit(x, y, 6 + i * aw, ay, aw - 4, 22)) {
                queue_pulse(st, act_key[i], 3, 2);
                snprintf(st->last_action, sizeof st->last_action, "Custom -> %s", act_name[i]);
                return st->last_action;
            }
        }
    }

    return NULL;
}
