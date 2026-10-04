# Renderer traps

Read this when the project uses the 2D renderer (`SDL_Renderer`). Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## linear-by-default

**What compiles:**

```c
SDL_Texture *sprites = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 16, 16);
SDL_RenderTexture(renderer, sprites, &src, &dst);    /* dst larger than src: blurred */
```

**What breaks:** SDL2 created textures with nearest-pixel scaling unless the app set `SDL_HINT_RENDER_SCALE_QUALITY`. SDL3 removed that hint and creates every texture with `SDL_SCALEMODE_LINEAR`. Code that relied on SDL2's default compiles unchanged, and every texture drawn at a size other than its own now blends neighboring pixels, so pixel art and tile maps blur. That includes scaling through the destination rectangle, `SDL_SetRenderScale` and a logical presentation, which SDL3 applies to each draw call, and a low-resolution target texture stretched to the window.

**How to find it:** search the ported sources for every texture the project creates, and for any scale-mode setting:

```
SDL_CreateTexture\w*|IMG_LoadTexture\w*
SDL_SetTextureScaleMode|SDL_SetDefaultTextureScaleMode|SCALE_QUALITY
```

In SDL2, a texture used nearest-pixel scaling unless `SDL_HINT_RENDER_SCALE_QUALITY` was set to `1`, `linear`, `2` or `best` when it was created, or the code called `SDL_SetTextureScaleMode` on it. Check `git diff` for the SDL2 hint. Every texture that was nearest in SDL2 and is ever drawn scaled needs its scale mode set in SDL3. Textures that SDL_image creates count too.

**Fix:** set the scale mode on each texture after creating it:

```c
SDL_Texture *sprites = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 16, 16);
SDL_SetTextureScaleMode(sprites, SDL_SCALEMODE_NEAREST);
```

From SDL 3.4.0, `SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST)` sets it once for every texture the renderer creates afterwards, and `SDL_SCALEMODE_PIXELART` adds "nearest pixel sampling with improved scaling for pixel art" (`SDL_surface.h`). Use them only if the project requires SDL 3.4 or later.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "Textures are created with SDL_SCALEMODE_LINEAR by default", and `SDL_hints.h` section: "SDL_HINT_RENDER_SCALE_QUALITY - textures now default to linear filtering, use SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST) if you want nearest pixel mode instead". `SDL_SetDefaultTextureScaleMode` in `SDL_render.h` ([wiki](https://wiki.libsdl.org/SDL3/SDL_SetDefaultTextureScaleMode)): "When a renderer is created, scale_mode defaults to SDL_SCALEMODE_LINEAR."

**Fixture:** `fixtures/linear-by-default/`.
