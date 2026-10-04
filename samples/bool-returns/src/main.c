#include <SDL.h>
#include <stdio.h>

static const SDL_Color palette[] = {
    { 0x1d, 0x2b, 0x53, 0xff },
    { 0x7e, 0x25, 0x53, 0xff },
    { 0x00, 0x87, 0x51, 0xff },
    { 0xab, 0x52, 0x36, 0xff },
};

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    Uint32 last_switch;
    int current = 0;
    int running = 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Palette", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Could not create the window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Could not create the renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (argc > 1 && SDL_strcmp(argv[1], "--fullscreen") == 0) {
        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            fprintf(stderr, "Staying in a window: %s\n", SDL_GetError());
        }
    }

    last_switch = SDL_GetTicks();
    while (running) {
        SDL_Event event;
        const SDL_Color *color;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
        }

        if (SDL_GetTicks() - last_switch >= 1000) {
            current = (current + 1) % (int)SDL_arraysize(palette);
            last_switch = SDL_GetTicks();
        }

        color = &palette[current];
        if (SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a) < 0 ||
            SDL_RenderClear(renderer) < 0) {
            fprintf(stderr, "Drawing failed: %s\n", SDL_GetError());
            break;
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
