#include <SDL.h>
#include <stdio.h>

#define VIEW_W 320
#define VIEW_H 180
#define TILE 8

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *tile;
    Uint32 pixels[TILE * TILE];
    int x, y;
    int running = 1;

    (void)argc;
    (void)argv;

    /* Sharp pixels, and don't fight other windows for the top of the screen. */
    SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");
    SDL_SetHint("SDL_ALLOW_TOPMOST", "0");
    SDL_SetHint("SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS", "0");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Arcade", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VIEW_W * 3, VIEW_H * 3,
                              SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, VIEW_W, VIEW_H);

    for (y = 0; y < TILE; y++) {
        for (x = 0; x < TILE; x++) {
            pixels[y * TILE + x] = ((x / 2 + y / 2) % 2) ? 0xFF3050A0 : 0xFF203060;
        }
    }
    tile = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TILE, TILE);
    if (!tile) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        running = 0;
    } else {
        SDL_UpdateTexture(tile, NULL, pixels, TILE * (int)sizeof(Uint32));
    }

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_RenderClear(renderer);
        for (y = 0; y < VIEW_H; y += TILE) {
            for (x = 0; x < VIEW_W; x += TILE) {
                SDL_Rect dst = { x, y, TILE, TILE };
                SDL_RenderCopy(renderer, tile, NULL, &dst);
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(tile);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
