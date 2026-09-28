#include <SDL.h>

#include "audio.h"

#define SAMPLE_RATE 44100
#define BEEP_SAMPLES (SAMPLE_RATE / 10)

static SDL_AudioDeviceID device;
static Sint16 beep[BEEP_SAMPLES];
static int beep_pos = BEEP_SAMPLES;

static void fill_audio(void *userdata, Uint8 *stream, int len)
{
    int remaining;
    int n;

    (void)userdata;
    SDL_memset(stream, 0, (size_t)len);

    remaining = (BEEP_SAMPLES - beep_pos) * (int)sizeof(Sint16);
    if (remaining <= 0) {
        return;
    }
    n = len < remaining ? len : remaining;
    SDL_MixAudioFormat(stream, (const Uint8 *)&beep[beep_pos], AUDIO_S16SYS, (Uint32)n, SDL_MIX_MAXVOLUME / 2);
    beep_pos += n / (int)sizeof(Sint16);
}

int audio_open(void)
{
    SDL_AudioSpec want, have;
    int i;

    for (i = 0; i < BEEP_SAMPLES; i++) {
        beep[i] = (Sint16)(((i / 50) % 2) ? 8000 : -8000);
    }

    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = fill_audio;

    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device == 0) {
        SDL_Log("No audio: %s", SDL_GetError());
        return -1;
    }
    SDL_PauseAudioDevice(device, 0);
    return 0;
}

void audio_beep(void)
{
    if (device == 0) {
        return;
    }
    SDL_LockAudioDevice(device);
    beep_pos = 0;
    SDL_UnlockAudioDevice(device);
}

void audio_close(void)
{
    if (device != 0) {
        SDL_CloseAudioDevice(device);
        device = 0;
    }
}
