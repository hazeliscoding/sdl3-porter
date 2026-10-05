#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

static void SDLCALL silence(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    Uint8 zeros[1024] = { 0 };

    (void)userdata;
    (void)total_amount;
    while (additional_amount > 0) {
        int chunk = additional_amount < (int)sizeof zeros ? additional_amount : (int)sizeof zeros;
        SDL_PutAudioStreamData(stream, zeros, chunk);
        additional_amount -= chunk;
    }
}

int main(int argc, char *argv[])
{
    const SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 48000 };
    SDL_AudioStream *stream;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, silence, NULL);
    if (!stream) {
        fprintf(stderr, "audio-init-implicit: opening the audio device failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_ResumeAudioStreamDevice(stream);

    SDL_DestroyAudioStream(stream);
    SDL_Quit();
    return 0;
}
