#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define SIZE 8

int main(int argc, char *argv[])
{
    /* A 2x2 checkerboard, drawn 4 times larger like pixel art. */
    static const Uint32 checker[4] = { 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000 };
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *drawn;
    int x, y, blended = 0;

    (void)argc;
    (void)argv;

    /* SDL3 removed this hint, and stores a hint under any name without reading it. */
    SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("hint-string-ignored", SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 2, 2);
    if (!texture) {
        fprintf(stderr, "setup: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_UpdateTexture(texture, NULL, checker, 2 * (int)sizeof checker[0])) {
        fprintf(stderr, "setup: SDL_UpdateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_RenderTexture(renderer, texture, NULL, NULL)) {
        fprintf(stderr, "setup: SDL_RenderTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    drawn = SDL_RenderReadPixels(renderer, NULL);
    if (!drawn) {
        fprintf(stderr, "setup: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        return 1;
    }

    for (y = 0; y < SIZE; y++) {
        for (x = 0; x < SIZE; x++) {
            Uint8 r, g, b, a;
            if (!SDL_ReadSurfacePixel(drawn, x, y, &r, &g, &b, &a)) {
                fprintf(stderr, "setup: SDL_ReadSurfacePixel failed: %s\n", SDL_GetError());
                return 1;
            }
            if (!(r == 0 && g == 0 && b == 0) && !(r == 255 && g == 255 && b == 255)) {
                blended++;
            }
        }
    }
    if (blended) {
        fprintf(stderr, "hint-string-ignored: asked for nearest-pixel scaling, but scaling blurred %d of %d pixels to gray\n",
                blended, SIZE * SIZE);
        return 1;
    }

    SDL_DestroySurface(drawn);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
