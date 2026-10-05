#include <SDL.h>
#include <stdio.h>

#define VIEW_W 320
#define VIEW_H 240
#define BUTTONS 3

static const char *const labels[BUTTONS] = { "Start", "Options", "Quit" };
static SDL_Rect buttons[BUTTONS];

static int button_at(int x, int y)
{
    SDL_Point p;
    int i;

    p.x = x;
    p.y = y;
    for (i = 0; i < BUTTONS; i++) {
        if (SDL_PointInRect(&p, &buttons[i])) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int hovered = -1;
    int running = 1;
    int i;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VIEW_W * 3, VIEW_H * 3,
                              SDL_WINDOW_RESIZABLE);
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
    SDL_RenderSetLogicalSize(renderer, VIEW_W, VIEW_H);

    for (i = 0; i < BUTTONS; i++) {
        buttons[i].x = 110;
        buttons[i].y = 60 + i * 50;
        buttons[i].w = 100;
        buttons[i].h = 36;
    }

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_MOUSEMOTION:
                hovered = button_at(event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int clicked = button_at(event.button.x, event.button.y);
                    if (clicked == BUTTONS - 1) {
                        running = 0;
                    } else if (clicked >= 0) {
                        printf("%s\n", labels[clicked]);
                    }
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 24, 32, 255);
        SDL_RenderClear(renderer);
        for (i = 0; i < BUTTONS; i++) {
            if (i == hovered) {
                SDL_SetRenderDrawColor(renderer, 251, 146, 60, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 194, 65, 12, 255);
            }
            SDL_RenderFillRect(renderer, &buttons[i]);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
