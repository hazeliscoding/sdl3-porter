#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define SDL_MIX_MAXVOLUME 128

int main(int argc, char *argv[])
{
    Sint16 source = 8000;
    Sint16 mixed = 0;

    (void)argc;
    (void)argv;

    /* SDL3 removed SDL_MIX_MAXVOLUME and takes a float from 0.0 to 1.0, so SDL2's half volume is now 64 times full volume. */
    if (!SDL_MixAudio((Uint8 *)&mixed, (const Uint8 *)&source, SDL_AUDIO_S16, (Uint32)sizeof source,
                      SDL_MIX_MAXVOLUME / 2)) {
        fprintf(stderr, "setup: SDL_MixAudio failed: %s\n", SDL_GetError());
        return 1;
    }
    if (mixed < 3900 || mixed > 4100) {
        fprintf(stderr, "mix-volume-float: mixing %d at half volume gave %d, not about 4000\n", source, mixed);
        return 1;
    }

    return 0;
}
