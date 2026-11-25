#include "pet_ui.h"

#include <stdio.h>
#include <string.h>

#define LINE 10

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

void pet_ui_draw(const UiDraw* ui, const PetModel* m, PetLayout layout) {
    const int w = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_W : PET_QVGA_W;
    const int h = (layout == PET_LAYOUT_WIDE) ? PET_WIDE_H : PET_QVGA_H;

    const UiColor bg       = ui_rgb(10, 24, 48);
    const UiColor header   = ui_rgb(18, 42, 82);
    const UiColor border   = ui_rgb(40, 120, 190);
    const UiColor cyan     = ui_rgb(0, 210, 255);
    const UiColor text_col = ui_rgb(230, 235, 245);
    const UiColor muted    = ui_rgb(150, 170, 200);
    const UiColor hp_bg    = ui_rgb(30, 40, 60);
    const UiColor hp_ok    = ui_rgb(60, 220, 110);
    const UiColor hp_low   = ui_rgb(240, 80, 70);
    const UiColor yellow   = ui_rgb(255, 220, 0);

    ui->clear(ui->ctx, bg);
    ui->rect(ui->ctx, 0, 0, w, 18, header);
    ui->line(ui->ctx, 0, 18, w, 18, border);
    ui->text(ui->ctx, 6, 5, "MMBN4 VIOLET ECLIPSE  -  PET", cyan);

    char buf[96];
    int y = 24;

    snprintf(buf, sizeof buf, "Game : %s  %s", m->game_code, m->short_name);
    draw_line(ui, 6, &y, buf, text_col);
    snprintf(buf, sizeof buf, "CRC32: %08X  Frm:%u", (unsigned) m->crc32, (unsigned) m->frame);
    draw_line(ui, 6, &y, buf, text_col);
    snprintf(buf, sizeof buf, "FPS  : %.1f%s", m->status.fps, m->status.fast_forward ? "  >> FAST" : "");
    draw_line(ui, 6, &y, buf, text_col);

    static const char* k_names[] = { "A", "B", "Sel", "Sta", "R", "L", "U", "D", "RB", "LB" };
    char keys[64] = "Keys : ";
    for (int i = 0; i < 10; ++i) {
        if (m->status.keys & (1u << i)) {
            strncat(keys, k_names[i], sizeof keys - strlen(keys) - 2);
            strncat(keys, " ", sizeof keys - strlen(keys) - 1);
        }
    }
    draw_line(ui, 6, &y, keys, text_col);

    y += 4;
    ui->line(ui->ctx, 6, y, w - 6, y, border);
    y += 5;

    if (!m->has_ram) {
        draw_line(ui, 6, &y, "(no RAM map for this version)", muted);
    } else if (m->save_valid) {
        draw_line(ui, 6, &y, "MegaMan Status", cyan);
        snprintf(buf, sizeof buf, "HP   : %4u / %4u   PT%u  Map %02X:%02X",
                 (unsigned) m->cur_hp, (unsigned) m->max_hp, (unsigned) m->playthrough,
                 (unsigned) m->map_group, (unsigned) m->map_id);
        draw_line(ui, 6, &y, buf, text_col);

        /* HP progress bar */
        int bar_x = 6, bar_w = w - 12, bar_h = 6;
        int fill = (m->max_hp > 0) ? (int) ((uint32_t) m->cur_hp * (uint32_t) bar_w / m->max_hp) : 0;
        if (fill > bar_w) fill = bar_w;
        UiColor hp_col = (m->cur_hp * 4u <= m->max_hp) ? hp_low : hp_ok;
        ui->rect(ui->ctx, bar_x, y, bar_w, bar_h, hp_bg);
        if (fill > 0) ui->rect(ui->ctx, bar_x, y, fill, bar_h, hp_col);
        ui->rect_outline(ui->ctx, bar_x, y, bar_w, bar_h, border);
        y += bar_h + 4;

        snprintf(buf, sizeof buf, "Zenny: %-9u  BugFrags: %u",
                 (unsigned) m->zenny, (unsigned) m->bugfrags);
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "Souls: %2u / 12    Tuner   : %s",
                 (unsigned) m->souls_count, tuner_name(m->tuner_mode));
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "BatHP: %u / %u   RNG: %08X",
                 (unsigned) m->battle_hp_p1, (unsigned) m->battle_hp_p2, (unsigned) m->rng);
        draw_line(ui, 6, &y, buf, muted);
    } else {
        draw_line(ui, 6, &y, "Live RAM (title / loading)", cyan);
        snprintf(buf, sizeof buf, "RNG state      : %08X", (unsigned) m->rng);
        draw_line(ui, 6, &y, buf, text_col);
        snprintf(buf, sizeof buf, "Battle HP P1/P2: %u / %u",
                 (unsigned) m->battle_hp_p1, (unsigned) m->battle_hp_p2);
        draw_line(ui, 6, &y, buf, text_col);
    }

    int fy = h - 6 - LINE * 5;
    ui->line(ui->ctx, 6, fy - 4, w - 6, fy - 4, border);
    draw_line(ui, 6, &fy, "Arrows move  X=A  Z=B  A/S=L/R", muted);
    draw_line(ui, 6, &fy, "Enter=Start  Backspace=Select", muted);
    draw_line(ui, 6, &fy, "Tab=fast  F5/F7=save/load state", muted);
    draw_line(ui, 6, &fy, "F1=PET  F11=fullscreen  Esc=quit", muted);
    if (m->status.message) {
        draw_line(ui, 6, &fy, m->status.message, yellow);
    }
}
