#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_DisplayMode mode;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("window-mode-null", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    /* The window's display mode, for the refresh rate to pace the game loop by. SDL2 always fills it in. */
    if (SDL_GetWindowDisplayMode(window, &mode) != 0) {
        fprintf(stderr, "window-mode-null: the window's display mode was unavailable: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
