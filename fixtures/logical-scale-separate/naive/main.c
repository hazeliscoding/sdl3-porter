#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* A 320x200 game, letterboxed in a 640x480 window. */
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect viewport;
    float scale_x, scale_y;
    int x, y;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("logical-scale-separate", 640, 480, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_SetRenderLogicalPresentation(renderer, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        fprintf(stderr, "setup: SDL_SetRenderLogicalPresentation failed: %s\n", SDL_GetError());
        return 1;
    }

    /* SDL3 keeps the logical presentation apart from these: the viewport starts at 0 and the scale is 1. */
    SDL_GetRenderViewport(renderer, &viewport);
    SDL_GetRenderScale(renderer, &scale_x, &scale_y);
    x = (int)(320 / scale_x) - viewport.x;
    y = (int)(240 / scale_y) - viewport.y;
    if (x != 160 || y != 100) {
        fprintf(stderr, "logical-scale-separate: the window's center mapped to (%d, %d) in the game, not (160, 100)\n", x, y);
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
