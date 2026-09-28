#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* SDL2: 0 on success, a negative error code on failure. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "bool-returns: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
