#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("text-input-off", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64, 64, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    /* A name-entry screen that reads SDL_TEXTINPUT events. SDL2 turned text input on in SDL_Init. */
    if (!SDL_IsTextInputActive()) {
        fprintf(stderr, "text-input-off: text input is off, so typing produces no text events\n");
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
