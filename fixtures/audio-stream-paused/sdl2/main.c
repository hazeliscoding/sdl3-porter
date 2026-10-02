#include <SDL.h>
#include <stdio.h>

#define FREQ 48000

int main(int argc, char *argv[])
{
    static Sint16 samples[FREQ / 10];
    SDL_AudioSpec want;
    SDL_AudioDeviceID device;
    Uint32 start;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_zero(want);
    want.freq = FREQ;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (device == 0) {
        fprintf(stderr, "setup: SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_QueueAudio(device, samples, sizeof samples) != 0) {
        fprintf(stderr, "setup: SDL_QueueAudio failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL2 devices start paused. */
    SDL_PauseAudioDevice(device, 0);

    start = SDL_GetTicks();
    while (SDL_GetQueuedAudioSize(device) >= sizeof samples && SDL_GetTicks() - start < 2000) {
        SDL_Delay(10);
    }
    if (SDL_GetQueuedAudioSize(device) >= sizeof samples) {
        fprintf(stderr, "audio-stream-paused: nothing played in 2 seconds, all %u bytes are still queued\n",
                (unsigned)sizeof samples);
        return 1;
    }

    SDL_CloseAudioDevice(device);
    SDL_Quit();
    return 0;
}
