#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_DisplayMode desktop;
    int i;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i < SDL_GetNumVideoDisplays(); i++) {
        SDL_Rect bounds;
        if (SDL_GetDisplayBounds(i, &bounds) == 0) {
            printf("Display %d: %s, %dx%d at (%d, %d)\n", i, SDL_GetDisplayName(i), bounds.w, bounds.h, bounds.x,
                   bounds.y);
        }
    }

    /* Open at three quarters of the main display's size. */
    if (SDL_GetDesktopDisplayMode(0, &desktop) != 0) {
        fprintf(stderr, "SDL_GetDesktopDisplayMode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    window = SDL_CreateWindow("Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, desktop.w * 3 / 4,
                              desktop.h * 3 / 4, SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    while (running) {
        SDL_Event event;

        if (!SDL_WaitEvent(&event)) {
            break;
        }
        if (event.type == SDL_QUIT) {
            running = 0;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
