#include <SDL.h>
#include <stdio.h>

#define SIZE 16

int main(int argc, char *argv[])
{
    /* An orange triangle over the top-left corner, colored per vertex. */
    const SDL_Vertex triangle[3] = {
        { { 0, 0 }, { 255, 128, 0, 255 }, { 0, 0 } },
        { { SIZE, 0 }, { 255, 128, 0, 255 }, { 0, 0 } },
        { { 0, SIZE }, { 255, 128, 0, 255 }, { 0, 0 } },
    };
    Uint32 drawn[SIZE * SIZE];
    SDL_Window *window;
    SDL_Renderer *renderer;
    Uint8 r, g, b;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("vertex-color-float", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) != 0 || SDL_RenderClear(renderer) != 0) {
        fprintf(stderr, "setup: clearing failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL2 vertex colors are bytes from 0 to 255. */
    if (SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0) != 0) {
        fprintf(stderr, "setup: SDL_RenderGeometry failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, drawn, SIZE * (int)sizeof drawn[0]) != 0) {
        fprintf(stderr, "setup: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        return 1;
    }

    r = (Uint8)(drawn[2 * SIZE + 2] >> 16);
    g = (Uint8)(drawn[2 * SIZE + 2] >> 8);
    b = (Uint8)drawn[2 * SIZE + 2];
    if (r < 240 || g < 108 || g > 148 || b > 15) {
        fprintf(stderr, "vertex-color-float: an orange vertex color (255, 128, 0) drew as (%u, %u, %u)\n", r, g, b);
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
