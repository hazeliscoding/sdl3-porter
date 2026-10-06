# Surface traps

Read this when the project creates or blits `SDL_Surface`s itself. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## indexed-surface-no-palette

**What compiles:**

```c
SDL_Surface *image = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX8);
SDL_SetPaletteColors(SDL_GetSurfacePalette(image), colors, 0, 256);    /* the palette is NULL */
```

**What breaks:** in SDL2, an indexed surface came with a palette of its own, reached through `surface->format->palette`. SDL3 creates indexed surfaces without one. `surface->format` is no longer a struct, so that expression doesn't compile, and its natural replacement, `SDL_GetSurfacePalette(surface)`, returns NULL. Setting colors on NULL fails, and blitting the surface to one that isn't indexed fails with "src does not have a palette set", so the graphics never appear. A blit between two indexed surfaces copies the indexes without translating them.

**How to find it:** search the ported sources for indexed formats and for palette calls:

```
SDL_PIXELFORMAT_INDEX\w+|SDL_GetPixelFormatForMasks\s*\(\s*[1248]\s*,
SDL_GetSurfacePalette|SDL_SetPaletteColors|SDL_SetSurfacePalette|SDL_CreateSurfacePalette
```

For each indexed surface the project creates, check that it gets a palette from `SDL_CreateSurfacePalette` or `SDL_SetSurfacePalette` before the code sets its colors or blits it. Surfaces loaded from an indexed BMP with `SDL_LoadBMP` come with their palette.

**Fix:** create the palette with the surface:

```c
SDL_Surface *image = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX8);
SDL_Palette *palette = SDL_CreateSurfacePalette(image);
SDL_SetPaletteColors(palette, colors, 0, 256);
```

**Source:** SDL `docs/README-migration.md`, `SDL_surface.h` section: "Indexed format surfaces no longer have a palette by default. Surfaces without a palette will copy the pixels untranslated between surfaces.", followed by the example that replaces `surface->format->palette` with `SDL_CreateSurfacePalette(surface)`. The blit error comes from `Map1toN` in SDL's `src/video/SDL_pixels.c` at `release-3.4.18`.

**Fixture:** `fixtures/indexed-surface-no-palette/`.
