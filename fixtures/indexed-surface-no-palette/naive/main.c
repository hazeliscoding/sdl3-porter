#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* A two-pixel indexed image, as old games and emulators keep their graphics. */
    static const SDL_Color colors[2] = { { 255, 0, 0, 255 }, { 0, 255, 0, 255 } };
    SDL_Surface *indexed, *screen;
    Uint32 pixel;

    (void)argc;
    (void)argv;

    indexed = SDL_CreateSurface(2, 1, SDL_PIXELFORMAT_INDEX8);
    screen = SDL_CreateSurface(2, 1, SDL_PIXELFORMAT_ARGB8888);
    if (!indexed || !screen) {
        fprintf(stderr, "setup: creating the surfaces failed: %s\n", SDL_GetError());
        return 1;
    }
    /* SDL3 no longer gives indexed surfaces a palette, so this palette is NULL and the colors go nowhere. */
    SDL_SetPaletteColors(SDL_GetSurfacePalette(indexed), colors, 0, 2);
    ((Uint8 *)indexed->pixels)[0] = 0;
    ((Uint8 *)indexed->pixels)[1] = 1;

    if (!SDL_BlitSurface(indexed, NULL, screen, NULL)) {
        fprintf(stderr, "indexed-surface-no-palette: blitting the indexed image failed: %s\n", SDL_GetError());
        return 1;
    }
    pixel = ((const Uint32 *)screen->pixels)[0];
    if ((pixel & 0xFFFFFF) != 0xFF0000) {
        fprintf(stderr, "indexed-surface-no-palette: palette index 0 blitted as 0x%06X, not red\n",
                (unsigned)(pixel & 0xFFFFFF));
        return 1;
    }

    SDL_DestroySurface(screen);
    SDL_DestroySurface(indexed);
    return 0;
}
