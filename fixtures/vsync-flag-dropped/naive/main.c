#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int vsync;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("vsync-flag-dropped", 8, 8, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL3 removed the flags argument, and SDL_RENDERER_PRESENTVSYNC with it, so vsync is off. */
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_GetRenderVSync(renderer, &vsync)) {
        fprintf(stderr, "setup: SDL_GetRenderVSync failed: %s\n", SDL_GetError());
        return 1;
    }
    if (vsync == SDL_RENDERER_VSYNC_DISABLED) {
        fprintf(stderr, "vsync-flag-dropped: the renderer has vsync off, so presenting won't wait for the display\n");
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
