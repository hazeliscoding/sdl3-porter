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

## vsync-flag-dropped

**What compiles:**

```c
/* SDL2: SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) */
SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
```

**What breaks:** SDL3's `SDL_CreateRenderer` has no flags argument, so the port has to drop `SDL_RENDERER_PRESENTVSYNC`, and SDL3 creates renderers with vsync off. `SDL_RenderPresent` no longer waits for the display. A loop that relied on vsync to pace itself now runs as fast as it can, keeps a CPU core busy, and speeds up anything it advances once per frame.

**How to find it:** search the SDL2 code in `git diff` for the ways SDL2 turned vsync on, and the ported sources for where SDL3 creates renderers or sets vsync:

```
SDL_RENDERER_PRESENTVSYNC|SDL_HINT_RENDER_VSYNC|SDL_RenderSetVSync
SDL_CreateRenderer\w*|SDL_CreateWindowAndRenderer|SDL_SetRenderVSync|SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER
```

Every renderer that SDL2 created with `SDL_RENDERER_PRESENTVSYNC` needs vsync turned on again in SDL3. `SDL_HINT_RENDER_VSYNC` still works in SDL3, so a renderer that got vsync only from that hint keeps it.

**Fix:** turn vsync on once the renderer exists:

```c
SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
if (!renderer) {
    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    return 1;
}
SDL_SetRenderVSync(renderer, 1);
```

Or set `SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER` to 1 for `SDL_CreateRendererWithProperties`.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "SDL_CreateRenderer()'s flags parameter has been removed" and "SDL_RENDERER_PRESENTVSYNC - replaced with SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER during renderer creation and SDL_PROP_RENDERER_VSYNC_NUMBER after renderer creation". `SDL_SetRenderVSync` in `SDL_render.h` ([wiki](https://wiki.libsdl.org/SDL3/SDL_SetRenderVSync)): "When a renderer is created, vsync defaults to SDL_RENDERER_VSYNC_DISABLED."

**Fixture:** `fixtures/vsync-flag-dropped/`.

## blend-by-default

**What compiles:**

```c
SDL_Texture *screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
pixels[i] = (r << 16) | (g << 8) | b;    /* alpha byte left at 0 */
SDL_RenderTexture(renderer, screen, NULL, NULL);    /* draws nothing */
```

**What breaks:** SDL2 created textures with `SDL_BLENDMODE_NONE`, so the alpha byte of their pixels didn't matter. SDL3 creates every texture whose format has an alpha channel with `SDL_BLENDMODE_BLEND`. Pixels written with an alpha of 0 now draw fully transparent, and the screen shows only what was under the texture, often plain black. Emulators and software renderers are the usual victims: they build `0x00RRGGBB` values for an ARGB format and never set the alpha byte. Pixels with a partial alpha draw partly transparent.

**How to find it:** search the ported sources for textures they create, and for blend-mode settings:

```
SDL_CreateTexture\w*
SDL_SetTextureBlendMode|SDL_BLENDMODE_
```

For each texture with an alpha format, meaning one with an `A` in its name such as `ARGB8888`, `RGBA8888`, `ABGR8888`, `BGRA8888` or `RGBA32`, check whether the code sets its blend mode. If it doesn't, check what it writes into the alpha channel: 0, nothing at all, or values that SDL2 ignored. Formats with an `X` instead, such as `XRGB8888`, have no alpha channel.

**Fix:** turn blending off to keep SDL2's behavior:

```c
SDL_Texture *screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_NONE);
```

Writing an opaque alpha (`0xFF000000 | ...`) or creating the texture with an `X` format such as `SDL_PIXELFORMAT_XRGB8888` also works. Textures that are meant to be blended keep the default.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "Textures are created with SDL_SCALEMODE_LINEAR by default, and use SDL_BLENDMODE_BLEND by default if they are created with a format that has an alpha channel." SDL2's default comes from its source at `release-2.32.10`, not its docs: `SDL_CreateTexture` in `src/render/SDL_render.c` zero-fills the new texture, which leaves its blend mode at `SDL_BLENDMODE_NONE`.

**Fixture:** `fixtures/blend-by-default/`.
