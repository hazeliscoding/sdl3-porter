#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "bool-returns: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Quit();
    return 0;
}
