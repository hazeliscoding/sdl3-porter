# Video traps

Read this when the project creates windows or asks about displays and display modes. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

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
