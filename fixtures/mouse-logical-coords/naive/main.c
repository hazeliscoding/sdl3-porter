#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* A button drawn at a logical size of 320x240, in a 640x480 window. */
    const SDL_FRect button = { 100, 100, 40, 40 };
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event click, event;
    bool hit = false, clicked = false;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("mouse-logical-coords", 640, 480, 0);
    if (!window) {
        fprintf(stderr, "setup: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    if (!renderer) {
        fprintf(stderr, "setup: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_SetRenderLogicalPresentation(renderer, 320, 240, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        fprintf(stderr, "setup: SDL_SetRenderLogicalPresentation failed: %s\n", SDL_GetError());
        return 1;
    }

    /* A click on the button's center, at window coordinates (240, 240). */
    SDL_zero(click);
    click.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click.button.windowID = SDL_GetWindowID(window);
    click.button.button = SDL_BUTTON_LEFT;
    click.button.down = true;
    click.button.clicks = 1;
    click.button.x = 240;
    click.button.y = 240;
    if (!SDL_PushEvent(&click)) {
        fprintf(stderr, "setup: SDL_PushEvent failed: %s\n", SDL_GetError());
        return 1;
    }

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            /* SDL3 no longer converts mouse events to the logical size, so these are window coordinates. */
            SDL_FPoint at;
            at.x = event.button.x;
            at.y = event.button.y;
            clicked = true;
            hit = SDL_PointInRectFloat(&at, &button);
            if (!hit) {
                fprintf(stderr, "mouse-logical-coords: a click on the button's center arrived at (%g, %g), outside the button at (100, 100)-(140, 140)\n",
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
