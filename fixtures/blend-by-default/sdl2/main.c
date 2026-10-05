#include <SDL.h>
#include <stdio.h>

#define SIZE 4

int main(int argc, char *argv[])
{
    /* A frame buffer of 0x00RRGGBB pixels, as emulators build them, with the alpha byte left at 0. */
    Uint32 frame[SIZE * SIZE];
    Uint32 drawn[SIZE * SIZE];
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen;
    int i;

    (void)argc;
    (void)argv;

    for (i = 0; i < SIZE * SIZE; i++) {
        frame[i] = 0x00FFFFFF;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("blend-by-default", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL2 creates textures with SDL_BLENDMODE_NONE, so the alpha byte is ignored. */
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SIZE, SIZE);
    if (!screen) {
        fprintf(stderr, "setup: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_UpdateTexture(screen, NULL, frame, SIZE * (int)sizeof frame[0]) != 0) {
        fprintf(stderr, "setup: SDL_UpdateTexture failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) != 0 || SDL_RenderClear(renderer) != 0) {
        fprintf(stderr, "setup: clearing failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderCopy(renderer, screen, NULL, NULL) != 0) {
        fprintf(stderr, "setup: SDL_RenderCopy failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, drawn, SIZE * (int)sizeof drawn[0]) != 0) {
        fprintf(stderr, "setup: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        return 1;
    }

    if ((drawn[0] & 0xFFFFFF) != 0xFFFFFF) {
        fprintf(stderr, "blend-by-default: a white frame drew as 0x%06X over black, so its alpha byte hid it\n",
                (unsigned)(drawn[0] & 0xFFFFFF));
        return 1;
    }

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
