#include <SDL.h>
#include <stdio.h>

#define SIZE 8

int main(int argc, char *argv[])
{
    /* A 2x2 checkerboard, drawn 4 times larger like pixel art. */
    static const Uint32 checker[4] = { 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000 };
    Uint32 pixels[SIZE * SIZE];
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int i, blended = 0;

    (void)argc;
    (void)argv;

    SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("hint-string-ignored", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 2, 2);
    if (!texture) {
        fprintf(stderr, "setup: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_UpdateTexture(texture, NULL, checker, 2 * (int)sizeof checker[0]) != 0) {
        fprintf(stderr, "setup: SDL_UpdateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0) {
        fprintf(stderr, "setup: SDL_RenderCopy failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, SIZE * (int)sizeof pixels[0]) != 0) {
        fprintf(stderr, "setup: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i < SIZE * SIZE; i++) {
        Uint32 rgb = pixels[i] & 0xFFFFFF;
        if (rgb != 0x000000 && rgb != 0xFFFFFF) {
            blended++;
        }
    }
    if (blended) {
        fprintf(stderr, "hint-string-ignored: asked for nearest-pixel scaling, but scaling blurred %d of %d pixels to gray\n",
                blended, SIZE * SIZE);
        return 1;
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
