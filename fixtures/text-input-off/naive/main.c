#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("text-input-off", 64, 64, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    /* A name-entry screen that reads SDL_EVENT_TEXT_INPUT events. SDL3 no longer turns text input on in SDL_Init. */
    if (!SDL_TextInputActive(window)) {
        fprintf(stderr, "text-input-off: text input is off, so typing produces no text events\n");
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
