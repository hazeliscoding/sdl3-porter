#include <SDL.h>
#include <stdio.h>

#define W 320
#define H 240

static const int quad_indices[6] = { 0, 1, 2, 0, 2, 3 };

/* Dusk: deep blue at the top, fading to a dim red at the horizon. */
static const SDL_Vertex sky[4] = {
    { { 0, 0 }, { 20, 30, 80, 255 }, { 0, 0 } },
    { { W, 0 }, { 20, 30, 80, 255 }, { 0, 0 } },
    { { W, H }, { 120, 40, 20, 255 }, { 0, 0 } },
    { { 0, H }, { 120, 40, 20, 255 }, { 0, 0 } },
};

/* Four corners of a rectangle, with one color on the left and another on the right. */
static void make_bar(SDL_Vertex *v, float x, float y, float w, float h, SDL_Color left, SDL_Color right)
{
    int i;

    v[0].position.x = x;
    v[0].position.y = y;
    v[1].position.x = x + w;
    v[1].position.y = y;
    v[2].position.x = x + w;
    v[2].position.y = y + h;
    v[3].position.x = x;
    v[3].position.y = y + h;
    v[0].color = left;
    v[1].color = right;
    v[2].color = right;
    v[3].color = left;
    for (i = 0; i < 4; i++) {
        v[i].tex_coord.x = 0;
        v[i].tex_coord.y = 0;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Vertex bar[4];
    int health = 100;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Dusk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W * 2, H * 2, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, W, H);

    while (running) {
        SDL_Event event;
        SDL_Color low = { 200, 40, 30, 255 };
        SDL_Color high = { 40, 180, 60, 255 };

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                health = health > 10 ? health - 10 : 100;
            }
        }

        /* The bar fades from red to green across its width, and shrinks with health. */
        make_bar(bar, 20, 20, (float)(health * 2), 12, low, high);

        SDL_RenderClear(renderer);
        SDL_RenderGeometry(renderer, NULL, sky, 4, quad_indices, 6);
        SDL_RenderGeometry(renderer, NULL, bar, 4, quad_indices, 6);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
