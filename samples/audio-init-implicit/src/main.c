#include <SDL.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define BPM 120

static Uint32 sample_pos;

/* A short click at the start of every beat. */
static void SDLCALL click_track(void *userdata, Uint8 *stream, int len)
{
    Sint16 *out = (Sint16 *)stream;
    int count = len / (int)sizeof(Sint16);
    Uint32 beat_len = SAMPLE_RATE * 60 / BPM;
    int i;

    (void)userdata;
    for (i = 0; i < count; i++) {
        Uint32 into_beat = sample_pos % beat_len;
        out[i] = (Sint16)(into_beat < 400 ? (((into_beat / 20) % 2) ? 9000 : -9000) : 0);
        sample_pos++;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioSpec want;
    SDL_Rect light = { 100, 60, 120, 120 };
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Metronome", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, 0);
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

    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = click_track;
    if (SDL_OpenAudio(&want, NULL) < 0) {
        fprintf(stderr, "No audio: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudio(0);
    }

    while (running) {
        SDL_Event event;
        int on_beat;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        on_beat = (SDL_GetTicks() % (60000 / BPM)) < 100;
        SDL_SetRenderDrawColor(renderer, 16, 16, 24, 255);
        SDL_RenderClear(renderer);
        if (on_beat) {
            SDL_SetRenderDrawColor(renderer, 251, 146, 60, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 60, 40, 30, 255);
        }
        SDL_RenderFillRect(renderer, &light);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
