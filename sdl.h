#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

sdl_t *sdl_init();
SDL_KeyCode sdl_readkey();
void sdl_quit(sdl_t *sdl);

#endif