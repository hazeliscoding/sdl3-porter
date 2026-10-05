#include <SDL.h>
#include <stdio.h>

static void SDLCALL silence(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    SDL_memset(stream, 0, (size_t)len);
}

int main(int argc, char *argv[])
{
    SDL_AudioSpec want;

    (void)argc;
    (void)argv;

    /* Only video: SDL2's SDL_OpenAudio initializes the audio subsystem itself. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = silence;
    if (SDL_OpenAudio(&want, NULL) < 0) {
        fprintf(stderr, "audio-init-implicit: opening the audio device failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
