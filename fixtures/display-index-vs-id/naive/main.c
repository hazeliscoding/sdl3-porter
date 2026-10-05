#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Rect bounds;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* SDL3 takes a display ID here, not an index, and 0 is never a valid ID. */
    if (!SDL_GetDisplayBounds(0, &bounds)) {
        fprintf(stderr, "display-index-vs-id: SDL_GetDisplayBounds(0) failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
