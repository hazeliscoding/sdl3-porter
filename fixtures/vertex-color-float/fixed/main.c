#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define SIZE 16

int main(int argc, char *argv[])
{
    /* An orange triangle over the top-left corner, colored per vertex. */
    const SDL_Vertex triangle[3] = {
        { { 0, 0 }, { 1.0f, 128 / 255.0f, 0.0f, 1.0f }, { 0, 0 } },
        { { SIZE, 0 }, { 1.0f, 128 / 255.0f, 0.0f, 1.0f }, { 0, 0 } },
        { { 0, SIZE }, { 1.0f, 128 / 255.0f, 0.0f, 1.0f }, { 0, 0 } },
    };
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *drawn;
    Uint8 r, g, b, a;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("vertex-color-float", SIZE, SIZE, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) || !SDL_RenderClear(renderer)) {
        fprintf(stderr, "setup: clearing failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0)) {
        fprintf(stderr, "setup: SDL_RenderGeometry failed: %s\n", SDL_GetError());
        return 1;
    }
    drawn = SDL_RenderReadPixels(renderer, NULL);
    if (!drawn || !SDL_ReadSurfacePixel(drawn, 2, 2, &r, &g, &b, &a)) {
        fprintf(stderr, "setup: reading pixels failed: %s\n", SDL_GetError());
        return 1;
    }

    if (r < 240 || g < 108 || g > 148 || b > 15) {
        fprintf(stderr, "vertex-color-float: an orange vertex color (255, 128, 0) drew as (%u, %u, %u)\n", r, g, b);
        return 1;
    }

    SDL_DestroySurface(drawn);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
