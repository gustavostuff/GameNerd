#include "retr01_nano_sim/app.h"

#include "retr01_nano_sim/board.h"
#include "retr01_nano_sim/ui.h"

#include "retr01_sim/island_group.h"
#include "retr01_sim/types.h"

#include <SDL.h>

#include <stdio.h>
#include <string.h>

#ifdef R01NS_DEFAULT_CART
#define DEFAULT_CART R01NS_DEFAULT_CART
#else
#define DEFAULT_CART "../../output/nano/test.r01nano"
#endif

static void logic_from_window(SDL_Window *win, int scale, int win_x, int win_y, int *lx, int *ly) {
    int ww, wh, draw_w, draw_h, ox, oy;
    SDL_GetWindowSize(win, &ww, &wh);
    draw_w = R01S_LOGIC_W * scale;
    draw_h = R01S_LOGIC_H * scale;
    ox = (ww - draw_w) / 2;
    oy = (wh - draw_h) / 2;
    *lx = (win_x - ox) / scale;
    *ly = (win_y - oy) / scale;
}

int r01ns_app_run(const char *cart_path) {
    R01nsBoard board;
    R01nsUi ui;
    char err[256];
    const char *path = cart_path && cart_path[0] ? cart_path : DEFAULT_CART;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    SDL_Texture *target = NULL;
    int scale = 2;
    int running = 1;
    Uint32 fps_last;
    int fps_frames = 0;
    R01sIslandGroup *group;

    if (r01ns_board_build(&board) != 0) {
        fprintf(stderr, "nano_sim: board_build failed\n");
        return 1;
    }
    if (r01ns_board_load_cart(&board, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "nano_sim: load cart failed: %s\n", err);
        r01ns_board_shutdown(&board);
        return 1;
    }
    if (r01ns_board_boot(&board) != 0) {
        fprintf(stderr, "nano_sim: boot failed: %s\n", r01ns_atmega1284p_nano_err(&board.mcu));
        r01ns_board_shutdown(&board);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        r01ns_board_shutdown(&board);
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    win = SDL_CreateWindow("Retr01 Nano Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           R01S_LOGIC_W * scale, R01S_LOGIC_H * scale, 0);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        r01ns_board_shutdown(&board);
        SDL_Quit();
        return 1;
    }
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, 0);
    }
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        r01ns_board_shutdown(&board);
        SDL_Quit();
        return 1;
    }
    target = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, R01S_LOGIC_W,
                               R01S_LOGIC_H);
    if (!target) {
        fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        r01ns_board_shutdown(&board);
        SDL_Quit();
        return 1;
    }

    r01ns_ui_init(&ui, &board);
    r01ns_ui_mount_builder(&ui);
    group = r01ns_board_group(&board);

    printf("nano_sim: IC board (VIDEO | MCU | CART) — scanline RGBS kernel\n");
    printf("  cart: %s (%zu bytes in SST25VF010A)\n", path, board.flash.image_len);
    printf("  parts: ATMEGA1284P + OSC8M + PWR5V + PADS + PWM2CH + SST25 + 24C64 + SCREEN_SINK\n");
    printf("  fidelity: 1 board step = 1 RGBS line (or VBlank line); not one emu frame/UI frame\n");
    printf("  SPACE pause. WASD+G pads. Esc quit. RMB pan.\n");

    fps_last = SDL_GetTicks();
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            int lx = 0, ly = 0;
            if (ev.type == SDL_QUIT) {
                running = 0;
            } else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            } else if (ev.type == SDL_KEYDOWN && (ev.key.keysym.mod & KMOD_CTRL)) {
                if (ev.key.keysym.sym == SDLK_1) {
                    scale = 1;
                    SDL_SetWindowSize(win, R01S_LOGIC_W * scale, R01S_LOGIC_H * scale);
                } else if (ev.key.keysym.sym == SDLK_2) {
                    scale = 2;
                    SDL_SetWindowSize(win, R01S_LOGIC_W * scale, R01S_LOGIC_H * scale);
                } else if (ev.key.keysym.sym == SDLK_3) {
                    scale = 3;
                    SDL_SetWindowSize(win, R01S_LOGIC_W * scale, R01S_LOGIC_H * scale);
                }
            }
            if (ev.type == SDL_MOUSEMOTION || ev.type == SDL_MOUSEBUTTONDOWN ||
                ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEWHEEL || ev.type == SDL_KEYDOWN) {
                int mx = 0, my = 0;
                SDL_GetMouseState(&mx, &my);
                logic_from_window(win, scale, mx, my, &lx, &ly);
                if (ev.type == SDL_MOUSEMOTION) {
                    /* scale mouse deltas into logic space */
                    ev.motion.xrel /= scale > 0 ? scale : 1;
                    ev.motion.yrel /= scale > 0 ? scale : 1;
                }
                r01ns_ui_handle_event(&ui, &ev, lx, ly);
            }
        }

        r01ns_ui_sync_gamepads(&ui);
        r01ns_board_set_pads(&board, r01ns_ui_pad_byte(&ui), r01s_gamepad_encode(&ui.gamepad[1]));

        ui.sim_steps = 0;
        if (group && group->running) {
            /*
             * Advance exactly one RGBS field per UI frame.
             * Scanline steps are cheap — a wall-time budget would finish 2+ fields
             * per vsync and Host Play would run >60 Hz (looks like frame-skip / turbo).
             */
            uint32_t fields0 = board.mcu.frames;
            int n = 0;
            while (n < R01NS_FIELD_LINES + 2) {
                r01s_island_group_step(group);
                n++;
                if (board.mcu.frames > fields0) {
                    break;
                }
            }
            ui.sim_steps = n;
        } else if (group) {
            r01s_island_group_eval_idle(group);
        }
        if (group) {
            r01s_island_group_fill_status(group, ui.status, sizeof(ui.status));
            r01s_island_group_update_probes(group, &ui.probe_vdd, &ui.probe_phi2, NULL);
        }

        fps_frames++;
        {
            Uint32 now = SDL_GetTicks();
            if (now - fps_last >= 1000u) {
                ui.fps = fps_frames;
                fps_frames = 0;
                fps_last = now;
            }
        }

        SDL_SetRenderTarget(ren, target);
        r01ns_ui_draw(&ui, ren);
        SDL_SetRenderTarget(ren, NULL);

        {
            int ww, wh, sx, sy, dw, dh;
            SDL_Rect dst;
            SDL_GetWindowSize(win, &ww, &wh);
            sx = ww / R01S_LOGIC_W;
            sy = wh / R01S_LOGIC_H;
            scale = sx < sy ? sx : sy;
            if (scale < 1) {
                scale = 1;
            }
            dw = R01S_LOGIC_W * scale;
            dh = R01S_LOGIC_H * scale;
            dst.x = (ww - dw) / 2;
            dst.y = (wh - dh) / 2;
            dst.w = dw;
            dst.h = dh;
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);
            SDL_RenderCopy(ren, target, NULL, &dst);
            SDL_RenderPresent(ren);
        }
    }

    printf("nano_sim: steps=%u frames=%u SPI_MAP=%uB eeprom pins wired pwm_ticks=%u\n", board.steps,
           board.mcu.frames, board.mcu.map_bytes_spi, board.pwm.ticks);

    r01ns_ui_shutdown(&ui);
    SDL_DestroyTexture(target);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    r01ns_board_shutdown(&board);
    SDL_Quit();
    return 0;
}
