#include "retr01_nano_emu/machine.h"

#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef R01NE_DEFAULT_CART
#define DEFAULT_CART R01NE_DEFAULT_CART
#else
#define DEFAULT_CART "../../output/nano/test.r01nano"
#endif

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : DEFAULT_CART;
    R01neMachine machine;
    char err[256];
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    SDL_Texture *tex = NULL;
    int scale = 2;
    int running = 1;
    Uint32 last_ticks;

    memset(&machine, 0, sizeof(machine));
    if (r01ne_machine_boot(&machine, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "nano_emu: boot failed: %s\n", err);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        r01ne_machine_shutdown(&machine);
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    /* Hidden until the first composed frame is presented (avoids open flicker). */
    win = SDL_CreateWindow("Retr01 Nano Emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           R01NE_VISIBLE_W * scale, R01NE_VISIBLE_H * scale, SDL_WINDOW_HIDDEN);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        r01ne_machine_shutdown(&machine);
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
        r01ne_machine_shutdown(&machine);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(ren, R01NE_VISIBLE_W, R01NE_VISIBLE_H);
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, R01NE_VISIBLE_W,
                            R01NE_VISIBLE_H);
    if (!tex) {
        fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        r01ne_machine_shutdown(&machine);
        SDL_Quit();
        return 1;
    }

    printf("nano_emu: %s (%zu bytes)\n", path, machine.cart.len);
    printf("  world 0: %u screens, spawn (%u,%u) — MAP preview only (no sprites/input)\n",
           (unsigned)machine.world.screen_count, (unsigned)machine.world.spawn_col,
           (unsigned)machine.world.spawn_row);
    printf("Esc quit. R reset. Ctrl+1/2/3 scale.\n");

    SDL_UpdateTexture(tex, NULL, machine.video.fb, R01NE_VISIBLE_W * 3);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
    SDL_ShowWindow(win);

    last_ticks = SDL_GetTicks();
    while (running) {
        SDL_Event ev;
        int framed = 0;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = 0;
            } else if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_1) {
                    scale = 1;
                    SDL_SetWindowSize(win, R01NE_VISIBLE_W * scale, R01NE_VISIBLE_H * scale);
                } else if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_2) {
                    scale = 2;
                    SDL_SetWindowSize(win, R01NE_VISIBLE_W * scale, R01NE_VISIBLE_H * scale);
                } else if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_3) {
                    scale = 3;
                    SDL_SetWindowSize(win, R01NE_VISIBLE_W * scale, R01NE_VISIBLE_H * scale);
                } else if (ev.key.keysym.sym == SDLK_r) {
                    (void)r01ne_machine_reset(&machine);
                    framed = 1;
                }
            }
        }

        {
            Uint32 now = SDL_GetTicks();
            if ((int)(now - last_ticks) >= 16) {
                r01ne_machine_frame(&machine);
                last_ticks = now;
                framed = 1;
            }
        }

        if (framed) {
            SDL_UpdateTexture(tex, NULL, machine.video.fb, R01NE_VISIBLE_W * 3);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);
            SDL_RenderCopy(ren, tex, NULL, NULL);
            SDL_RenderPresent(ren);
        } else {
            SDL_Delay(1);
        }
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    r01ne_machine_shutdown(&machine);
    SDL_Quit();
    return 0;
}
