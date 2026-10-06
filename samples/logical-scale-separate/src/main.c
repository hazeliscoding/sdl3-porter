#include <SDL.h>
#include <stdio.h>

#define VIEW_W 320
#define VIEW_H 200
#define TILE 20
#define COLS (VIEW_W / TILE)
#define ROWS (VIEW_H / TILE)

static int lit[ROWS][COLS];

/* Converts a position in the window to the board's 320x200 space. */
static void window_to_board(SDL_Renderer *renderer, int wx, int wy, int *bx, int *by)
{
    SDL_Rect viewport;
    float scale_x, scale_y;

    SDL_RenderGetViewport(renderer, &viewport);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);
    *bx = (int)(wx / scale_x) - viewport.x;
    *by = (int)(wy / scale_y) - viewport.y;
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    Uint32 last_buttons = 0;
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Tiles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 600, SDL_WINDOW_RESIZABLE);
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
    SDL_RenderSetLogicalSize(renderer, VIEW_W, VIEW_H);

    while (running) {
        SDL_Event event;
        Uint32 buttons;
        int mx, my, bx, by, row, col;
        int hovered;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        buttons = SDL_GetMouseState(&mx, &my);
        window_to_board(renderer, mx, my, &bx, &by);
        hovered = bx >= 0 && by >= 0 && bx < VIEW_W && by < VIEW_H;
        col = bx / TILE;
        row = by / TILE;
        if (hovered && (buttons & SDL_BUTTON_LMASK) && !(last_buttons & SDL_BUTTON_LMASK)) {
            lit[row][col] = !lit[row][col];
        }
        last_buttons = buttons;

        SDL_SetRenderDrawColor(renderer, 18, 18, 24, 255);
        SDL_RenderClear(renderer);
        for (row = 0; row < ROWS; row++) {
            for (col = 0; col < COLS; col++) {
                SDL_Rect tile = { col * TILE + 1, row * TILE + 1, TILE - 2, TILE - 2 };

                if (lit[row][col]) {
                    SDL_SetRenderDrawColor(renderer, 251, 146, 60, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 48, 54, 61, 255);
                }
                SDL_RenderFillRect(renderer, &tile);
            }
        }
        if (hovered) {
            SDL_Rect cursor = { (bx / TILE) * TILE, (by / TILE) * TILE, TILE, TILE };

            SDL_SetRenderDrawColor(renderer, 230, 237, 243, 255);
            SDL_RenderDrawRect(renderer, &cursor);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
