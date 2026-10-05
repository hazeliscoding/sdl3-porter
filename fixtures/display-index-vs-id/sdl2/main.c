#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Rect bounds;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* The first display's size, to center a window on it. SDL2 takes a display index here. */
    if (SDL_GetDisplayBounds(0, &bounds) != 0) {
        fprintf(stderr, "display-index-vs-id: SDL_GetDisplayBounds(0) failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
