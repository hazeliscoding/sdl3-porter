#include <SDL.h>
#include <stdio.h>

#define VIEW_W 160
#define VIEW_H 120
#define SPRITE 16

static const char *const sprite_rows[SPRITE] = {
    "................",
    ".....######.....",
    "...##########...",
    "..############..",
    ".####..##..####.",
    ".####..##..####.",
    "################",
    "################",
    "################",
    "####.######.####",
    ".####......####.",
    "..############..",
    "...##########...",
    ".....######.....",
    "................",
    "................",
};

static SDL_Texture *load_sprite(SDL_Renderer *renderer)
{
    Uint32 pixels[SPRITE * SPRITE];
    SDL_Texture *texture;
    int x, y;

    for (y = 0; y < SPRITE; y++) {
        for (x = 0; x < SPRITE; x++) {
            pixels[y * SPRITE + x] = sprite_rows[y][x] == '#' ? 0xFFF0C020 : 0x00000000;
        }
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SPRITE, SPRITE);
    if (!texture) {
        return NULL;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, NULL, pixels, SPRITE * (int)sizeof(Uint32));
    return texture;
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *sprite;
    SDL_Rect dst = { 0, (VIEW_H - SPRITE) / 2, SPRITE, SPRITE };
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Sprite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VIEW_W * 4, VIEW_H * 4,
                              SDL_WINDOW_RESIZABLE);
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

    /* Draw at 160x120 and let SDL scale it up to the window. */
    SDL_RenderSetLogicalSize(renderer, VIEW_W, VIEW_H);

    sprite = load_sprite(renderer);
    if (!sprite) {
        fprintf(stderr, "Could not create the sprite: %s\n", SDL_GetError());
        running = 0;
    }

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        dst.x = (dst.x + 1) % VIEW_W;

        SDL_SetRenderDrawColor(renderer, 24, 24, 48, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, sprite, NULL, &dst);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
