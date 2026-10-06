# Hint traps

Read this when the project sets SDL hints, whether in code or through environment variables, launch scripts or config files. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## hint-string-ignored

**What compiles:**

```c
SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");    /* removed in SDL3 */
SDL_SetHint("SDL_VIDEODRIVER", "x11");                 /* renamed SDL_VIDEO_DRIVER */
```

**What breaks:** SDL3 renamed some hints and removed others, and a hint's name is only a string. SDL3 stores a hint under any name: `SDL_SetHint` returns true and `SDL_GetHint` returns the value, but nothing in SDL reads it, so the setting does nothing. A hint written as an `SDL_HINT_*` macro is safe, because the rename scripts rename it or it fails to compile. The scripts never touch a name written as a string: a string literal, an environment variable, a launch script or a config file. SDL3 still reads the SDL2 environment variables `SDL_VIDEODRIVER` and `SDL_AUDIODRIVER`, but no other old name, and it ignores those two when they're set as hints in code.

**How to find it:** search the whole project, not only the C and C++ sources, for hint names written as strings. Include scripts, `.desktop` files, Dockerfiles, CI files, docs and config files:

```
"SDL_[A-Z0-9_]+"
\bSDL_[A-Z0-9_]+=
```

For each name that is used as a hint, either passed to an `SDL_*Hint*` function or set as an environment variable, look for it in quotes in SDL3's `SDL_hints.h`. If it isn't there, find it in the migration guide's `SDL_hints.h` section, in the lists of renamed and removed hints and environment variables. The lists give the macro names. A hint's string is its macro name with `SDL_HINT_` replaced by `SDL_`. For example, `SDL_HINT_RENDER_SCALE_QUALITY` is `"SDL_RENDER_SCALE_QUALITY"`. SDL2 has four exceptions: `SDL_HINT_IDLE_TIMER_DISABLED` is `"SDL_IOS_IDLE_TIMER_DISABLED"`, `SDL_HINT_ORIENTATIONS` is `"SDL_IOS_ORIENTATIONS"`, and `SDL_HINT_FORCE_RAISEWINDOW` and `SDL_HINT_VITA_TOUCH_MOUSE_DEVICE` keep their `SDL_HINT_` prefix in the string.

**Fix:** use the SDL3 macro for a renamed hint, so the compiler checks the name:

```c
SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
```

For a removed hint, do what the migration guide's list says instead. For `SDL_RENDER_SCALE_QUALITY` set to `0` or `nearest`, call `SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)` on each texture (see [render.md: linear-by-default](render.md#linear-by-default)). Update environment variables, scripts and config files to the new names, and tell the user about the ones outside the repository.

**Source:** SDL `docs/README-migration.md`, `SDL_hints.h` section: the lists "The following hints have been renamed", "The following hints have been removed" and "The following environment variables have been renamed", including "SDL_HINT_RENDER_SCALE_QUALITY - textures now default to linear filtering, use SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST) if you want nearest pixel mode instead". That SDL3 accepts any name and reads only the two old environment variables comes from SDL's `src/SDL_hints.c` at `release-3.4.18`, not from the docs: `SDL_SetHintWithPriority` adds a hint it doesn't know, and `GetHintEnvironmentVariable` falls back only to `SDL_VIDEODRIVER` and `SDL_AUDIODRIVER`.

**Fixture:** `fixtures/hint-string-ignored/`.
