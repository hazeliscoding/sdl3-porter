#include <SDL.h>
#include <stdio.h>

#define WIDTH 480
#define HEIGHT 270

/* One frame at the display's refresh rate, or 16 ms when SDL can't tell. */
static Uint32 frame_time_ms(SDL_Window *window)
{
    SDL_DisplayMode mode;

    if (SDL_GetWindowDisplayMode(window, &mode) != 0 || mode.refresh_rate <= 0) {
        return 16;
    }
    return 1000 / (Uint32)mode.refresh_rate;
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect dot = { 0, HEIGHT / 2 - 8, 16, 16 };
    Uint32 frame_ms;
    int dx = 3;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Pacer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    frame_ms = frame_time_ms(window);
    printf("Pacing frames at %u ms\n", (unsigned)frame_ms);

    while (running) {
        Uint32 start = SDL_GetTicks();
        Uint32 spent;
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        dot.x += dx;
        if (dot.x < 0 || dot.x + dot.w > WIDTH) {
            dx = -dx;
        }

        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 251, 146, 60, 255);
        SDL_RenderFillRect(renderer, &dot);
        SDL_RenderPresent(renderer);

        spent = SDL_GetTicks() - start;
        if (spent < frame_ms) {
            SDL_Delay(frame_ms - spent);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
