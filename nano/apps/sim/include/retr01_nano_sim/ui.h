#ifndef RETR01_NANO_SIM_UI_H
#define RETR01_NANO_SIM_UI_H

#include "retr01_nano_sim/board.h"

#include "retr01_sim/gamepad.h"
#include "retr01_sim/types.h"

#include <SDL.h>

#define R01NS_UI_MAX_CHIPS 32
#define R01NS_UI_VIEW_Y 24
#define R01NS_UI_VIEW_H (R01S_LOGIC_H - 48)

typedef struct R01nsUi {
    R01nsBoard *board;
    R01sEntity *chips[R01NS_UI_MAX_CHIPS];
    uint8_t chip_island[R01NS_UI_MAX_CHIPS];
    int chip_count;
    int selected;
    int pan_x;
    int pan_y;
    int drag_pan;
    int drag_chip;
    int mouse_lx;
    int mouse_ly;
    R01sGamepadInput gamepad[2];
    SDL_Texture *lcd_tex;
    char status[192];
    int fps;
    int sim_steps;
    int probe_vdd;
    int probe_phi2;
} R01nsUi;

int r01ns_ui_init(R01nsUi *ui, R01nsBoard *board);
void r01ns_ui_shutdown(R01nsUi *ui);
void r01ns_ui_mount_builder(R01nsUi *ui);
void r01ns_ui_sync_gamepads(R01nsUi *ui);
uint8_t r01ns_ui_pad_byte(const R01nsUi *ui);
void r01ns_ui_handle_event(R01nsUi *ui, const SDL_Event *e, int logic_x, int logic_y);
void r01ns_ui_draw(R01nsUi *ui, SDL_Renderer *ren);

#endif
