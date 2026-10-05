#include <SDL.h>
#include <stdio.h>

/* The header of a save file: a magic tag, then three single-byte fields. */
typedef struct {
    char magic[4];
    Uint8 version;
    Uint8 level;
    Uint8 lives;
    Uint8 unused;
} Header;

static const Uint8 save[] = { 'S', 'A', 'V', 'E', 1, 7, 3, 0 };

int main(int argc, char *argv[])
{
    SDL_RWops *rw;
    Header header;
    size_t got;

    (void)argc;
    (void)argv;

    rw = SDL_RWFromConstMem(save, (int)sizeof save);
    if (!rw) {
        fprintf(stderr, "setup: SDL_RWFromConstMem failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Read one header. SDL2 returns the number of whole objects read. */
    got = SDL_RWread(rw, &header, sizeof header, 1);
    if (got != 1) {
        fprintf(stderr, "rwread-count-vs-bytes: reading one %u-byte header returned %u, so the read looked like a failure\n",
                (unsigned)sizeof header, (unsigned)got);
        return 1;
    }
    if (SDL_memcmp(header.magic, "SAVE", 4) != 0 || header.level != 7) {
        fprintf(stderr, "rwread-count-vs-bytes: the header read back wrong: level %u instead of 7\n", header.level);
        return 1;
    }

    SDL_RWclose(rw);
    return 0;
}
