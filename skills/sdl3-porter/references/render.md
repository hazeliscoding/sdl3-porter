# Renderer traps

Read this when the project uses the 2D renderer (`SDL_Renderer`). Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime. The guidance sections after the traps are changes no headless fixture can reproduce, so check them by reading the code.

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
SDL_SetRenderLogicalPresentation|SDL_SetRenderScale
```

If the project sets a logical presentation or a render scale, every texture it draws is scaled, even one drawn at its own size in logical coordinates, unless the window happens to be exactly the logical size. A 16×16 sprite drawn at 16×16 in a 160×120 logical view is scaled up with the rest of the view, so it blurs.

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

You usually meet this in step 5: SDL3's `SDL_CreateRenderer` has no flags argument, so the old call doesn't compile. When you delete the flags to make it compile, add `SDL_SetRenderVSync` right there if they included `SDL_RENDERER_PRESENTVSYNC`, instead of waiting for the sweep.

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

For each texture with an alpha format, meaning one with an `A` in its name such as `ARGB8888`, `RGBA8888`, `ABGR8888`, `BGRA8888` or `RGBA32`, check whether the code sets its blend mode. If it doesn't, check what it writes into the alpha channel. Alpha left at 0, never set, or below 255 draws differently in SDL3, so that's the trap. If every pixel is fully opaque, with alpha 255, blending changes nothing: it isn't this trap and needs no change. Formats with an `X` instead, such as `XRGB8888`, have no alpha channel.

**Fix:** turn blending off to keep SDL2's behavior:

```c
SDL_Texture *screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_NONE);
```

Writing an opaque alpha (`0xFF000000 | ...`) or creating the texture with an `X` format such as `SDL_PIXELFORMAT_XRGB8888` also works. Textures that are meant to be blended keep the default.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "Textures are created with SDL_SCALEMODE_LINEAR by default, and use SDL_BLENDMODE_BLEND by default if they are created with a format that has an alpha channel." SDL2's default comes from its source at `release-2.32.10`, not its docs: `SDL_CreateTexture` in `src/render/SDL_render.c` zero-fills the new texture, which leaves its blend mode at `SDL_BLENDMODE_NONE`.

**Fixture:** `fixtures/blend-by-default/`.

## vertex-color-float

**What compiles:**

```c
SDL_Vertex v = { { 10, 10 }, { 255, 128, 0, 255 }, { 0, 0 } };    /* 128 now clamps to full */
v.color.a = sprite_color.a;                                       /* a 0-255 byte in a 0-1 float */
```

**What breaks:** SDL2's `SDL_Vertex` held an `SDL_Color`, four bytes from 0 to 255. SDL3's holds an `SDL_FColor`, four floats from 0 to 1 for ordinary content. Integer values still compile, whether constants or copies of `Uint8` fields, and anything above 1 clamps to full. Only 0 and 255 survive, so mid-tones go to full strength: orange (255, 128, 0) draws yellow, a half-transparent alpha of 128 draws opaque, and dimmed vertex colors draw at full brightness. Assigning a whole `SDL_Color` to the field doesn't compile, so the trap hides in initializers and channel-by-channel copies.

**How to find it:** search the ported sources for vertices and their colors:

```
SDL_Vertex|SDL_RenderGeometry\b
\bcolor\.(r|g|b|a)\s*=
```

Check that every vertex color is a float from 0 to 1. `SDL_RenderGeometryRaw` takes `SDL_FColor` as well, so the compiler catches a byte color array passed to it.

**Fix:** divide byte values by 255:

```c
SDL_Vertex v = { { 10, 10 }, { 1.0f, 128 / 255.0f, 0.0f, 1.0f }, { 0, 0 } };
v.color.a = sprite_color.a / 255.0f;
```

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "SDL_Vertex has been changed to use floating point colors, in the range of [0..1] for SDR content." The clamp comes from SDL's software renderer, in `src/render/software/SDL_render_sw.c` at `release-3.4.18`.

**Fixture:** `fixtures/vertex-color-float/`.

## logical-scale-separate

**What compiles:**

```c
SDL_SetRenderLogicalPresentation(renderer, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX);    /* was SDL_RenderSetLogicalSize */
SDL_GetRenderViewport(renderer, &viewport);    /* SDL3: 0, 0, 320, 200, with no letterbox offset */
SDL_GetRenderScale(renderer, &sx, &sy);        /* SDL3: 1.0, not the logical scale */
int game_x = (int)(mouse_x / sx) - viewport.x;
```

**What breaks:** SDL2's `SDL_RenderSetLogicalSize` worked by setting the renderer's viewport and scale, so `SDL_RenderGetViewport` returned the letterbox offset and `SDL_RenderGetScale` the logical scale. Code that converted a window position into the logical size, such as one from `SDL_GetMouseState`, used those two. SDL3 keeps the logical presentation apart from the viewport and scale: with a logical size set, the viewport still starts at 0 and the scale is 1. The conversion then returns window coordinates unchanged. In a 640×480 window showing 320×200, the window's center maps to (320, 240) instead of (160, 100), and mouse hover and clicks miss whenever the window isn't at the logical size.

**How to find it:** search the ported sources for code that reads the viewport or the scale:

```
SDL_GetRenderViewport\s*\(|SDL_GetRenderScale\s*\(
```

If the project also sets a logical presentation, check what each caller does with the values. Converting a position between the window and the logical size is this trap. Code that reads back a viewport or scale it set itself is fine.

**Fix:** let SDL convert:

```c
float game_x, game_y;
SDL_RenderCoordinatesFromWindow(renderer, mouse_x, mouse_y, &game_x, &game_y);
```

`SDL_RenderCoordinatesToWindow` converts the other way, `SDL_ConvertEventToRenderCoordinates` converts an event in place (see `mouse-logical-coords`), and `SDL_GetRenderLogicalPresentationRect` gives the letterboxed area in window pixels. Don't work the scale out yourself from `SDL_GetRenderOutputSize`, `SDL_GetCurrentRenderOutputSize` or the window size: that misses the letterbox offset, and on a high-DPI display the output size is in pixels while mouse positions are in window coordinates.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "SDL_RenderSetLogicalSize() (now called SDL_SetRenderLogicalPresentation()) in SDL2 would modify the scaling and viewport state. In SDL3, logical presentation maintains its state separately, so the app can use its own viewport and scaling while also setting a logical size."

**Fixture:** `fixtures/logical-scale-separate/`.

## Batching with direct OpenGL

**What changed:** SDL2's renderer queued draw calls and sent them to the GPU in batches, but turned batching off when the program asked for a specific backend, such as `SDL_HINT_RENDER_DRIVER` set to `opengl`, because such a program might draw with OpenGL itself. SDL3 always batches. A port that mixes the renderer with its own OpenGL, Direct3D, Metal or Vulkan calls compiles unchanged, then draws in the wrong order: its own drawing runs before the renderer's queued drawing, which paints over it, or the renderer's state ends up in the program's calls.

**How to find it:** check whether a file that uses `SDL_Renderer` also calls a graphics API directly:

```
\bgl[A-Z]\w*\s*\(|SDL_GetRenderMetalCommandEncoder|SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER|SDL_PROP_RENDERER_\w*(D3D|VULKAN)\w*
```

**Fix:** call `SDL_FlushRenderer(renderer)` before each stretch of direct graphics calls, so SDL's queued drawing goes first. In SDL2 code that already called `SDL_RenderFlush`, the rename scripts make that `SDL_FlushRenderer`, which is right.

`SDL_GL_BindTexture` is gone too. Its replacement reads the texture's ID from `SDL_GetTextureProperties`, and the property depends on the renderer: `SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER` under `opengl`, `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER` under `opengles2`. The other one reads 0, so code that checks only for an OpenGL renderer by a name prefix and reads the `opengl` property binds texture 0 under OpenGL ES 2. Check `SDL_GetRendererName` and read the matching property.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "The 2D renderer API always uses batching in SDL3. [...] all apps that use SDL3's 2D renderer and also want to call directly into the platform's lower-layer graphics API _must_ call SDL_FlushRenderer() before doing so." SDL2's default is in `SDL_hints.h` at `release-2.32.10`, under `SDL_HINT_RENDER_BATCHING`: "SDL will disable batching if a specific render backend is requested". For texture IDs, the same section: "SDL_GL_BindTexture() - use SDL_GetTextureProperties() to get the OpenGL texture ID and bind the texture directly", and the separate `SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER` and `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER` in `SDL_render.h` at `release-3.4.18`.

## YUV color space

**What changed:** SDL2 set the YUV conversion for every texture at once, with `SDL_SetYUVConversionMode`. Its default was BT.601, and `SDL_YUV_CONVERSION_AUTOMATIC` chose BT.709 for video taller than 576 lines and BT.601 below. SDL3 removes the function, so the old call doesn't compile, and sets the color space per texture instead. A texture created without one uses `SDL_COLORSPACE_YUV_DEFAULT`, which is BT.601 limited range. A port that drops the call, or swaps in the default to make it compile, decodes HD video with BT.601 math where SDL2's automatic mode used BT.709, and the colors shift slightly.

**How to find it:** search the ported sources for YUV textures and color spaces:

```
SDL_PIXELFORMAT_(YV12|IYUV|NV12|NV21|P010)|SDL_COLORSPACE_|SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER
```

Check what the SDL2 code passed to `SDL_SetYUVConversionMode`, if anything.

**Fix:** create the texture with `SDL_CreateTextureWithProperties` and `SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER`, choosing what SDL2 chose: `SDL_COLORSPACE_BT709_LIMITED` above 576 lines for `SDL_YUV_CONVERSION_AUTOMATIC`, `SDL_COLORSPACE_BT709_LIMITED` for `SDL_YUV_CONVERSION_BT709`, `SDL_COLORSPACE_JPEG` for `SDL_YUV_CONVERSION_JPEG`, and the default for BT.601. A video decoder that reports the stream's color space, such as FFmpeg, gives a better answer than any of these.

**Source:** SDL `docs/README-migration.md`, `SDL_surface.h` section: "SDL_SetYUVConversionMode() - use SDL_SetSurfaceColorspace() to set the surface colorspace and SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER with SDL_CreateTextureWithProperties() to set the texture colorspace." The guide goes on to say the default is `SDL_COLORSPACE_JPEG`, but `SDL_pixels.h` at `release-3.4.18` defines `SDL_COLORSPACE_YUV_DEFAULT = SDL_COLORSPACE_BT601_LIMITED`, "The default colorspace for YUV surfaces if no colorspace is specified", and SDL's code agrees. SDL2's modes are in `src/video/SDL_yuv.c` at `release-2.32.10`: the default `SDL_YUV_CONVERSION_BT601`, and `SDL_YUV_SD_THRESHOLD` of 576 lines for the automatic mode.
