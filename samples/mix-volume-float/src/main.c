#include <SDL.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define LOOP_SAMPLES (SAMPLE_RATE / 10)

static Sint16 music[LOOP_SAMPLES];
static Sint16 click[LOOP_SAMPLES];
static int music_pos;
static int click_pos = LOOP_SAMPLES;

static void SDLCALL mix_audio(void *userdata, Uint8 *stream, int len)
{
    Uint8 *out = stream;
    int left = len;
    int chunk;

    (void)userdata;
    SDL_memset(stream, 0, (size_t)len);

    /* The music loops quietly under everything. */
    while (left > 0) {
        chunk = (LOOP_SAMPLES - music_pos) * (int)sizeof(Sint16);
        if (chunk > left) {
            chunk = left;
        }
        SDL_MixAudioFormat(out, (const Uint8 *)&music[music_pos], AUDIO_S16SYS, (Uint32)chunk, SDL_MIX_MAXVOLUME / 4);
        music_pos = (music_pos + chunk / (int)sizeof(Sint16)) % LOOP_SAMPLES;
        out += chunk;
        left -= chunk;
    }

    /* A click plays once, at full volume. */
    chunk = (LOOP_SAMPLES - click_pos) * (int)sizeof(Sint16);
    if (chunk > 0) {
        if (chunk > len) {
            chunk = len;
        }
        SDL_MixAudioFormat(stream, (const Uint8 *)&click[click_pos], AUDIO_S16SYS, (Uint32)chunk, SDL_MIX_MAXVOLUME);
        click_pos += chunk / (int)sizeof(Sint16);
    }
}

int main(int argc, char *argv[])
{
    SDL_AudioSpec want;
    SDL_AudioDeviceID device;
    int i;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i < LOOP_SAMPLES; i++) {
        music[i] = (Sint16)(((i / 200) % 2) ? 6000 : -6000);
        click[i] = (Sint16)(i < 400 ? (((i / 20) % 2) ? 12000 : -12000) : 0);
    }

    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = mix_audio;

    device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (device == 0) {
        fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_PauseAudioDevice(device, 0);

    for (i = 0; i < 3; i++) {
        SDL_Delay(1000);
        SDL_LockAudioDevice(device);
        click_pos = 0;
        SDL_UnlockAudioDevice(device);
    }
    SDL_Delay(500);

    SDL_CloseAudioDevice(device);
    SDL_Quit();
    return 0;
}
