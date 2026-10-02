#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    Sint16 source = 8000;
    Sint16 mixed = 0;

    (void)argc;
    (void)argv;

    /* SDL2: volume runs from 0 to SDL_MIX_MAXVOLUME (128). */
    SDL_MixAudioFormat((Uint8 *)&mixed, (const Uint8 *)&source, AUDIO_S16SYS, (Uint32)sizeof source,
                       SDL_MIX_MAXVOLUME / 2);
    if (mixed < 3900 || mixed > 4100) {
        fprintf(stderr, "mix-volume-float: mixing %d at half volume gave %d, not about 4000\n", source, mixed);
        return 1;
    }

    return 0;
}
