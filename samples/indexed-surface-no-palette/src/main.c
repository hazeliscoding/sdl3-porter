#include <SDL.h>
#include <stdio.h>

#define W 160
#define H 100

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *image;
    SDL_Color colors[256];
    int x, y, i;
    int shift = 0;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Plasma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W * 4, H * 4, 0);
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

    /* An 8-bit image that is drawn once and animated by cycling its palette. */
    image = SDL_CreateRGBSurfaceWithFormat(0, W, H, 8, SDL_PIXELFORMAT_INDEX8);
    if (!image) {
        fprintf(stderr, "Could not create the image: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    for (y = 0; y < H; y++) {
        Uint8 *row = (Uint8 *)image->pixels + y * image->pitch;
        for (x = 0; x < W; x++) {
            row[x] = (Uint8)((x * x + y * y) / 16 + x * 2);
        }
    }

    while (running) {
        SDL_Event event;
        SDL_Texture *texture;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        for (i = 0; i < 256; i++) {
            int v = (i + shift) & 255;
            colors[i].r = (Uint8)v;
            colors[i].g = (Uint8)(255 - v);
            colors[i].b = (Uint8)(v / 2);
            colors[i].a = 255;
        }
        SDL_SetPaletteColors(image->format->palette, colors, 0, 256);
        shift++;

        texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_RenderClear(renderer);
        if (texture) {
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_DestroyTexture(texture);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_FreeSurface(image);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
