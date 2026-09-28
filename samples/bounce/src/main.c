#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

#define WIDTH 640
#define HEIGHT 360
#define SPRITE 8
#define SCALE 4

static SDL_GameController *pad;
static char name[64];

/* An 8x8 checker, drawn 4x larger: pixel art that must stay sharp. */
static SDL_Texture *make_sprite(SDL_Renderer *renderer)
{
    Uint32 pixels[SPRITE * SPRITE];
    SDL_Texture *texture;
    int x, y;

    for (y = 0; y < SPRITE; y++) {
        for (x = 0; x < SPRITE; x++) {
            pixels[y * SPRITE + x] = ((x + y) % 2) ? 0xFFFFFFFF : 0xFF2040C0;
        }
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SPRITE, SPRITE);
    if (texture) {
        SDL_UpdateTexture(texture, NULL, pixels, SPRITE * (int)sizeof(Uint32));
    }
    return texture;
}

static void handle_event(const SDL_Event *e, SDL_Window *window, int *running)
{
    switch (e->type) {
    case SDL_QUIT:
        *running = 0;
        break;
    case SDL_KEYDOWN:
        if (e->key.keysym.sym == SDLK_ESCAPE) {
            *running = 0;
        }
        break;
    case SDL_TEXTINPUT:
        SDL_strlcat(name, e->text.text, sizeof(name));
        SDL_SetWindowTitle(window, name);
        break;
    case SDL_DROPFILE:
        SDL_Log("Dropped %s", e->drop.file);
        SDL_free(e->drop.file);
        break;
    case SDL_CONTROLLERDEVICEADDED:
        if (!pad) {
            pad = SDL_GameControllerOpen(e->cdevice.which);
        }
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        if (pad && e->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad))) {
            SDL_GameControllerClose(pad);
            pad = NULL;
        }
        break;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *sprite;
    SDL_Rect ball = { WIDTH / 2, HEIGHT / 2, SPRITE * SCALE, SPRITE * SCALE };
    int dx = 3, dy = 2;
    int paddle_x = WIDTH / 2;
    int max_frames = -1;
    int frame = 0;
    int running = 1;
    Uint32 start;
    char *base;

    if (argc == 3 && strcmp(argv[1], "--frames") == 0) {
        max_frames = atoi(argv[2]);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    base = SDL_GetBasePath();
    if (base) {
        SDL_Log("Running from %s", base);
        SDL_free(base);
    }

    SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");

    window = SDL_CreateWindow("Bounce", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    sprite = make_sprite(renderer);
    audio_open();
    start = SDL_GetTicks();

    while (running) {
        SDL_Event e;
        const Uint8 *keys;
        SDL_Rect paddle;

        while (SDL_PollEvent(&e)) {
            handle_event(&e, window, &running);
        }

        keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT]) {
            paddle_x -= 6;
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            paddle_x += 6;
        }
        if (pad) {
            paddle_x += SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) / 4096;
        }
        paddle_x = SDL_clamp(paddle_x, 40, WIDTH - 40);

        ball.x += dx;
        ball.y += dy;
        if (ball.x < 0 || ball.x + ball.w > WIDTH) {
            dx = -dx;
            audio_beep();
        }
        if (ball.y < 0 || ball.y + ball.h > HEIGHT) {
            dy = -dy;
            audio_beep();
        }

        SDL_SetRenderDrawColor(renderer, 16, 20, 24, 255);
        SDL_RenderClear(renderer);
        paddle.x = paddle_x - 40;
        paddle.y = HEIGHT - 24;
        paddle.w = 80;
        paddle.h = 8;
        SDL_SetRenderDrawColor(renderer, 194, 65, 12, 255);
        SDL_RenderFillRect(renderer, &paddle);
        SDL_RenderCopy(renderer, sprite, NULL, &ball);
        SDL_RenderPresent(renderer);

        frame++;
        if (max_frames >= 0 && frame >= max_frames) {
            running = 0;
        }
    }

    SDL_Log("%d frames in %u ms", frame, SDL_GetTicks() - start);

    audio_close();
    if (pad) {
        SDL_GameControllerClose(pad);
    }
    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
