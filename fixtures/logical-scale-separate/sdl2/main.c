#include <SDL.h>
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("logical-scale-separate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderSetLogicalSize(renderer, 320, 200) != 0) {
        fprintf(stderr, "setup: SDL_RenderSetLogicalSize failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Map the window's center to the game, the way menus that read SDL_GetMouseState do. */
    SDL_RenderGetViewport(renderer, &viewport);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);
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
