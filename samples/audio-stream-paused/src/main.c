#include <SDL.h>
#include <stdio.h>

#define SAMPLE_RATE 48000
#define TONE_HZ 440

static Uint32 position;

/* Fills the device with a quiet square wave. */
static void SDLCALL fill_audio(void *userdata, Uint8 *stream, int len)
{
    Sint16 *samples = (Sint16 *)stream;
    int count = len / (int)sizeof(Sint16);
    int i;

    (void)userdata;
    for (i = 0; i < count; i++) {
        samples[i] = (Sint16)(((position / (SAMPLE_RATE / TONE_HZ / 2)) % 2) ? 3000 : -3000);
        position++;
    }
}

int main(int argc, char *argv[])
{
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;
    int seconds = 2;

    if (argc > 1) {
        seconds = SDL_atoi(argv[1]);
    }

    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = fill_audio;

    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device == 0) {
        fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Playing %d Hz for %d seconds\n", TONE_HZ, seconds);
    SDL_PauseAudioDevice(device, 0);
    SDL_Delay((Uint32)seconds * 1000);

    SDL_CloseAudioDevice(device);
    SDL_Quit();
    return 0;
}
