#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_DisplayID display;
    SDL_Rect bounds;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    display = SDL_GetPrimaryDisplay();
    if (display == 0) {
        fprintf(stderr, "setup: SDL_GetPrimaryDisplay failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_GetDisplayBounds(display, &bounds)) {
        fprintf(stderr, "display-index-vs-id: SDL_GetDisplayBounds(%u) failed: %s\n", (unsigned)display, SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
