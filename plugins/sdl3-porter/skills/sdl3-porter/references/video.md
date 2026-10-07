# Video traps

Read this when the project creates windows or asks about displays and display modes. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime. The guidance sections after the traps are changes no headless fixture can reproduce, so check them by reading the code.

## display-index-vs-id

**What compiles:**

```c
SDL_Rect bounds;
if (!SDL_GetDisplayBounds(0, &bounds)) {    /* the first display in SDL2, an invalid ID in SDL3 */
```

**What breaks:** SDL2 identified displays by index, from 0 to `SDL_GetNumVideoDisplays() - 1`. SDL3 identifies them by `SDL_DisplayID`, and 0 is never a valid ID. Functions that took an index, such as `SDL_GetDisplayBounds`, `SDL_GetDisplayUsableBounds` and `SDL_GetDisplayName`, now take an ID in the same place, and C converts an `int` index without a word. Index 0 fails with "Invalid display", and any other index names nothing or a different display. `SDL_GetDesktopDisplayMode` and `SDL_GetCurrentDisplayMode` now return a pointer, and for an invalid ID that pointer is NULL, which crashes code that uses it unchecked.

The window position macros changed the same way. `SDL_WINDOWPOS_CENTERED_DISPLAY(X)` and `SDL_WINDOWPOS_UNDEFINED_DISPLAY(X)` take an ID, and 0 means the primary display. SDL2's `SDL_WINDOWPOS_CENTERED_DISPLAY(1)`, the second monitor, now names whichever display has ID 1, if any.

**How to find it:** search the ported sources for display functions and the position macros:

```
SDL_GetDisplay\w*\s*\(|SDL_Get\w*DisplayMode\w*\s*\(
SDL_WINDOWPOS_(CENTERED|UNDEFINED)_DISPLAY
```

For each call, check where the display argument comes from. A literal, a loop counter or a number saved from SDL2 code is an index. An ID from `SDL_GetDisplays`, `SDL_GetPrimaryDisplay` or `SDL_GetDisplayForWindow` is right. Check every display mode pointer for NULL.

**Fix:** pass an ID:

```c
SDL_Rect bounds;
if (!SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &bounds)) {
```

To visit every display, loop over the IDs from `SDL_GetDisplays` and free the array afterwards. For the display a window is on, use `SDL_GetDisplayForWindow`.

**Source:** SDL `docs/README-migration.md`, `SDL_video.h` section: "Rather than iterating over displays using display index, there is a new function SDL_GetDisplays() to get the current list of displays, and functions which used to take a display index now take SDL_DisplayID, with an invalid ID being 0." and "The SDL_WINDOWPOS_UNDEFINED_DISPLAY() and SDL_WINDOWPOS_CENTERED_DISPLAY() macros take a display ID instead of display index. The display ID 0 has a special meaning in this case, and is used to indicate the primary display."

**Fixture:** `fixtures/display-index-vs-id/`.

## window-mode-null

**What compiles:**

```c
const SDL_DisplayMode *mode = SDL_GetWindowFullscreenMode(window);    /* was SDL_GetWindowDisplayMode(window, &mode) */
float hz = mode->refresh_rate;                                        /* mode is NULL for a normal window */
```

**What breaks:** SDL2's `SDL_GetWindowDisplayMode` always filled in a display mode for the window, with its size and the display's refresh rate, and games used it to pace their loops. The rename scripts turn it into SDL3's `SDL_GetWindowFullscreenMode`, which answers a different question. It returns the exclusive fullscreen mode the window will use, or NULL when the window will go fullscreen on the desktop, and every new window starts out that way. So for a game in a window it returns NULL, with an empty `SDL_GetError()`, because nothing failed. Code that dereferences the result crashes, and code that ports SDL2's error check treats the normal case as a failure.

**How to find it:** search the ported sources for the call:

```
SDL_GetWindowFullscreenMode\s*\(
```

Check what each caller wants. The mode the display runs in now, with its refresh rate and resolution, comes from `SDL_GetCurrentDisplayMode`. Only code that sets up exclusive fullscreen wants `SDL_GetWindowFullscreenMode`, and it has to treat NULL as fullscreen on the desktop. Adding a NULL check to the old call isn't a fix: a window that isn't in exclusive fullscreen always gets NULL, so the code silently takes its fallback, such as a fixed 60 Hz.

**Fix:** ask the display:

```c
const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
float hz = (mode && mode->refresh_rate > 0) ? mode->refresh_rate : 60.0f;
```

**Source:** SDL `docs/README-migration.md`, `SDL_video.h` section: "The fullscreen mode for a window can be queried with SDL_GetWindowFullscreenMode(), which returns a pointer to the mode, or NULL if the window will be fullscreen desktop.", and the rename "SDL_GetWindowDisplayMode() => SDL_GetWindowFullscreenMode()". `SDL_GetWindowFullscreenMode` in `SDL_video.h` ([wiki](https://wiki.libsdl.org/SDL3/SDL_GetWindowFullscreenMode)): "returns a pointer to the exclusive fullscreen mode to use or NULL for borderless fullscreen desktop mode."

**Fixture:** `fixtures/window-mode-null/`.

## Exclusive fullscreen

**What changed:** in SDL2, `SDL_WINDOW_FULLSCREEN`, at creation or in `SDL_SetWindowFullscreen`, meant exclusive fullscreen: SDL switched the display to the mode set with `SDL_SetWindowDisplayMode`, or to the mode closest to the window's size. `SDL_WINDOW_FULLSCREEN_DESKTOP` meant a borderless window over the desktop. SDL3 removes `SDL_WINDOW_FULLSCREEN_DESKTOP` and makes borderless the default for every fullscreen window, exclusive only after `SDL_SetWindowFullscreenMode` with a real mode. `SDL_SetWindowFullscreen` now takes a `bool`, and SDL2's `SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN)` still compiles, as `true`. So a game that switched the display to 640×480 now gets a window the size of the desktop. Unless it scales its drawing, it fills only the top-left corner.

**How to find it:** search the ported sources for fullscreen:

```
SDL_WINDOW_FULLSCREEN\b|SDL_SetWindowFullscreen\s*\(
```

Check what the SDL2 code asked for. `SDL_WINDOW_FULLSCREEN_DESKTOP` ports as is. `SDL_WINDOW_FULLSCREEN` without a scaled drawing path, meaning no logical size and no viewport from the window's pixel size, needs a fix.

**Fix:** scale to the window, for example with `SDL_SetRenderLogicalPresentation`. That's usually what players want, and it doesn't change the display. To keep exclusive fullscreen, set a mode before going fullscreen:

```c
SDL_DisplayMode mode;
if (SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window), 640, 480, 0.0f, false, &mode)) {
    SDL_SetWindowFullscreenMode(window, &mode);
}
SDL_SetWindowFullscreen(window, true);
```

**Source:** SDL `docs/README-migration.md`, `SDL_video.h` section: "Windows now have an explicit fullscreen mode that is set, using SDL_SetWindowFullscreenMode(). [...] SDL_SetWindowFullscreen() just takes a boolean value, setting the correct fullscreen state based on the selected mode." and "SDL_WINDOW_FULLSCREEN_DESKTOP has been removed".

## High DPI

**What changed:** SDL2 left a Windows program unaware of display scaling unless it asked, so on a display at 200%, Windows drew a 640×480 window at twice the size, a little blurry. SDL3 makes every Windows program aware of per-monitor scaling, and window sizes are in pixels, so the same window comes out half as big. macOS and Wayland work the other way round: sizes are in points, and the window's pixel size can be larger, with `SDL_WINDOW_HIGH_PIXEL_DENSITY` (SDL2's `SDL_WINDOW_ALLOW_HIGHDPI`). There, code that uses the window size as the size in pixels draws into the wrong area.

**How to find it:** search the ported sources for window sizes used as pixel sizes, and for the pixel density flag:

```
SDL_GetWindowSize\s*\(|SDL_WINDOW_HIGH_PIXEL_DENSITY
```

A window size passed to `glViewport`, to a texture or surface the size of the window, or to `SDL_RenderReadPixels`, needs the size in pixels.

**Fix:** use `SDL_GetWindowSizeInPixels` for anything measured in pixels, and resize on `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED`. For readable sizes on Windows, scale the window by `SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())` when creating it, and handle `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED`. If you change sizes this way, say so under `Also changed:`. Checking it on a scaled display is for a human.

**Source:** SDL `docs/README-migration.md`, `SDL_hints.h` section: "SDL_HINT_VIDEO_HIGHDPI_DISABLED - high DPI support is always enabled". `SDL_video.h` section: "You should use SDL_GetWindowSizeInPixels() to get the actual pixel size of the window back buffer." SDL `docs/README-highdpi.md` at `release-3.4.18`, with its table of window and pixel sizes on macOS and Windows: "Ignoring this scale factor results in graphics appearing tiny." The Windows default is `WIN_InitDPIAwareness` in `src/video/windows/SDL_windowsvideo.c`, which requests per-monitor awareness when no hint is set. SDL2's default was "Do not change the DPI awareness", in `SDL_hints.h` at `release-2.32.10`.

## Asynchronous window operations

**What changed:** in SDL3, these calls only ask the windowing system for a change, and return before it happens: `SDL_SetWindowSize`, `SDL_SetWindowPosition`, `SDL_MinimizeWindow`, `SDL_MaximizeWindow`, `SDL_RestoreWindow` and `SDL_SetWindowFullscreen`. The windowing system can also refuse, or change the request. Code that reads the window back right after the call, with `SDL_GetWindowSize`, `SDL_GetWindowPosition` or `SDL_GetWindowFlags`, can get the old values, and then sizes a texture, a viewport or a layout for the old window. Requests in a row can cancel out, too: `SDL_SetWindowSize` "has no effect" while the window is fullscreen or maximized, so a size set right after `SDL_SetWindowFullscreen(window, false)` or `SDL_RestoreWindow` is lost if the window hasn't left that state yet.

**How to find it:** search the ported sources for the requests:

```
SDL_(SetWindowSize|SetWindowPosition|MinimizeWindow|MaximizeWindow|RestoreWindow|SetWindowFullscreen)\s*\(
```

Check whether the code right after each call depends on the new state.

**Fix:** react to the window events instead: `SDL_EVENT_WINDOW_RESIZED`, `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED`, `SDL_EVENT_WINDOW_MOVED`, `SDL_EVENT_WINDOW_MINIMIZED`, `SDL_EVENT_WINDOW_MAXIMIZED`, `SDL_EVENT_WINDOW_RESTORED`, `SDL_EVENT_WINDOW_ENTER_FULLSCREEN` and `SDL_EVENT_WINDOW_LEAVE_FULLSCREEN`. Where the next line really needs the new state, call `SDL_SyncWindow(window)` after the request. It can block while the window animates.

**Source:** SDL `docs/README-migration.md`, `SDL_video.h` section: "The following window operations are now considered to be asynchronous requests and should not be assumed to succeed unless a corresponding event has been received", and "the `SDL_SyncWindow()` function will attempt to wait until all pending window operations have completed." `SDL_SetWindowSize` in `SDL_video.h` at `release-3.4.18`: "If the window is in a fullscreen or maximized state, this request has no effect."
