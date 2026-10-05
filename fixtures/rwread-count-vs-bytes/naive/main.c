#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
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
    SDL_IOStream *io;
    Header header;
    size_t got;

    (void)argc;
    (void)argv;

    io = SDL_IOFromConstMem(save, sizeof save);
    if (!io) {
        fprintf(stderr, "setup: SDL_IOFromConstMem failed: %s\n", SDL_GetError());
        return 1;
    }

    /* SDL3's SDL_ReadIO returns the number of bytes read, not the number of objects. */
    got = SDL_ReadIO(io, &header, sizeof header);
    if (got != 1) {
        fprintf(stderr, "rwread-count-vs-bytes: reading one %u-byte header returned %u, so the read looked like a failure\n",
                (unsigned)sizeof header, (unsigned)got);
        return 1;
    }
    if (SDL_memcmp(header.magic, "SAVE", 4) != 0 || header.level != 7) {
        fprintf(stderr, "rwread-count-vs-bytes: the header read back wrong: level %u instead of 7\n", header.level);
        return 1;
    }

    SDL_CloseIO(io);
    return 0;
}
