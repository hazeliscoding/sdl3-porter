#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    const SDL_DisplayMode *mode;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("window-mode-null", 320, 240, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
    if (!mode) {
        fprintf(stderr, "window-mode-null: the window's display mode was unavailable: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
