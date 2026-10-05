#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define SIZE 4

int main(int argc, char *argv[])
{
    /* A frame buffer of 0x00RRGGBB pixels, as emulators build them, with the alpha byte left at 0. */
    Uint32 frame[SIZE * SIZE];
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen;
    SDL_Surface *drawn;
    Uint8 r, g, b, a;
    int i;

    (void)argc;
    (void)argv;

    for (i = 0; i < SIZE * SIZE; i++) {
        frame[i] = 0x00FFFFFF;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("blend-by-default", SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL3 blends textures created with an alpha format, so the 0 alpha byte now hides every pixel. */
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SIZE, SIZE);
    if (!screen) {
        fprintf(stderr, "setup: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_UpdateTexture(screen, NULL, frame, SIZE * (int)sizeof frame[0])) {
        fprintf(stderr, "setup: SDL_UpdateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) || !SDL_RenderClear(renderer)) {
        fprintf(stderr, "setup: clearing failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_RenderTexture(renderer, screen, NULL, NULL)) {
        fprintf(stderr, "setup: SDL_RenderTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    drawn = SDL_RenderReadPixels(renderer, NULL);
    if (!drawn || !SDL_ReadSurfacePixel(drawn, 0, 0, &r, &g, &b, &a)) {
        fprintf(stderr, "setup: reading pixels failed: %s\n", SDL_GetError());
        return 1;
    }

    if (r != 255 || g != 255 || b != 255) {
        fprintf(stderr, "blend-by-default: a white frame drew as 0x%02X%02X%02X over black, so its alpha byte hid it\n",
                r, g, b);
        return 1;
    }

    SDL_DestroySurface(drawn);
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
