#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* SDL3 returns true on success, so this branch runs on every launch. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "bool-returns: SDL_Init reported failure, error: \"%s\"\n", SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
