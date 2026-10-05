#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define W 160
#define H 120
#define SCALE 4

static Uint8 heat[W * H];
static Uint32 palette[256];

/* Black through red and yellow to white. */
static void make_palette(void)
{
    int i;

    for (i = 0; i < 256; i++) {
        Uint32 r = (Uint32)(i < 85 ? i * 3 : 255);
        Uint32 g = (Uint32)(i < 85 ? 0 : i < 170 ? (i - 85) * 3 : 255);
        Uint32 b = (Uint32)(i < 170 ? 0 : (i - 170) * 3);
        palette[i] = (r << 16) | (g << 8) | b;
    }
}

/* Each cell cools a little as the heat below it rises. */
static void burn(void)
{
    int x, y;

    for (x = 0; x < W; x++) {
        heat[(H - 1) * W + x] = (Uint8)(rand() % 2 ? 255 : 0);
    }
    for (y = 0; y < H - 1; y++) {
        for (x = 0; x < W; x++) {
            int below = heat[(y + 1) * W + x];
            int left = heat[(y + 1) * W + (x + W - 1) % W];
            int right = heat[(y + 1) * W + (x + 1) % W];
            int sum = (below * 2 + left + right) / 4;
            heat[y * W + x] = (Uint8)(sum > 2 ? sum - 2 : 0);
        }
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Fire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W * SCALE, H * SCALE, 0);
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
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    if (!screen) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        running = 0;
    }

    make_palette();
    while (running) {
        SDL_Event event;
        void *pixels;
        int pitch, x, y;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        burn();
        if (SDL_LockTexture(screen, NULL, &pixels, &pitch) == 0) {
            for (y = 0; y < H; y++) {
                Uint32 *row = (Uint32 *)((Uint8 *)pixels + y * pitch);
                for (x = 0; x < W; x++) {
                    row[x] = palette[heat[y * W + x]];
                }
            }
            SDL_UnlockTexture(screen);
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
