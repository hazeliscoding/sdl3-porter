#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define FREQ 48000

int main(int argc, char *argv[])
{
    static Sint16 samples[FREQ / 10];
    const SDL_AudioSpec spec = { SDL_AUDIO_S16, 1, FREQ };
    SDL_AudioStream *stream;
    Uint64 start;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* SDL3 opens this device paused, and nothing here resumes it. */
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) {
        fprintf(stderr, "setup: SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_PutAudioStreamData(stream, samples, (int)sizeof samples)) {
        fprintf(stderr, "setup: SDL_PutAudioStreamData failed: %s\n", SDL_GetError());
        return 1;
    }

    start = SDL_GetTicks();
    while (SDL_GetAudioStreamQueued(stream) >= (int)sizeof samples && SDL_GetTicks() - start < 2000) {
        SDL_Delay(10);
    }
    if (SDL_GetAudioStreamQueued(stream) >= (int)sizeof samples) {
        fprintf(stderr, "audio-stream-paused: nothing played in 2 seconds, all %d bytes are still queued\n",
                (int)sizeof samples);
        return 1;
    }

    SDL_DestroyAudioStream(stream);
    SDL_Quit();
    return 0;
}
