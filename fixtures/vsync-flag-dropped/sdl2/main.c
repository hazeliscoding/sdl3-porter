#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_RendererInfo info;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("vsync-flag-dropped", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 8, 8, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_GetRendererInfo(renderer, &info) != 0) {
        fprintf(stderr, "setup: SDL_GetRendererInfo failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!(info.flags & SDL_RENDERER_PRESENTVSYNC)) {
        fprintf(stderr, "vsync-flag-dropped: the renderer has vsync off, so presenting won't wait for the display\n");
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
