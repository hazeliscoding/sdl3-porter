#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* A button drawn at a logical size of 320x240, in a 640x480 window. */
    const SDL_Rect button = { 100, 100, 40, 40 };
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event click, event;
    int hit = 0, clicked = 0;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("mouse-logical-coords", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_RenderSetLogicalSize(renderer, 320, 240) != 0) {
        fprintf(stderr, "setup: SDL_RenderSetLogicalSize failed: %s\n", SDL_GetError());
        return 1;
    }

    /* A click on the button's center, at window coordinates (240, 240). */
    SDL_zero(click);
    click.type = SDL_MOUSEBUTTONDOWN;
    click.button.windowID = SDL_GetWindowID(window);
    click.button.button = SDL_BUTTON_LEFT;
    click.button.state = SDL_PRESSED;
    click.button.clicks = 1;
    click.button.x = 240;
    click.button.y = 240;
    if (SDL_PushEvent(&click) < 0) {
        fprintf(stderr, "setup: SDL_PushEvent failed: %s\n", SDL_GetError());
        return 1;
    }

    /* SDL2 hands the click over in logical coordinates. */
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_Point at;
            at.x = event.button.x;
            at.y = event.button.y;
            clicked = 1;
            hit = SDL_PointInRect(&at, &button);
            if (!hit) {
                fprintf(stderr, "mouse-logical-coords: a click on the button's center arrived at (%d, %d), outside the button at (100, 100)-(140, 140)\n",
                        event.button.x, event.button.y);
            }
        }
    }
    if (!clicked) {
        fprintf(stderr, "setup: the pushed click never arrived\n");
        return 1;
    }
    if (!hit) {
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
