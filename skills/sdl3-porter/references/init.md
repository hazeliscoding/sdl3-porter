# Init and error handling traps

Read this for every port. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## bool-returns

**What compiles:**

```c
if (SDL_Init(SDL_INIT_VIDEO) != 0) {    /* or < 0, or == -1 */
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
}
```

**What breaks:** SDL3 functions that returned `0` on success and a negative error code on failure now return `bool`: `true` on success, `false` on failure.

- `!= 0` and `== -1` style checks now treat success as failure. The error branch runs on every launch, with an empty `SDL_GetError()`.
- `< 0` checks never fire, so real failures go unnoticed.
- `== 0` success checks now take the failure path.

**How to find it:** search the ported sources for SDL calls compared with a number, and for SDL results stored in an `int`:

```
SDL_\w+\s*\([^;]*\)\s*(==|!=|<|<=|>|>=)\s*-?[0-9]
int\s+\w+\s*=\s*SDL_\w+\s*\(
```

For each hit, look up the function in the SDL3 headers. Fix it if the function now returns `bool`. Functions that return a pointer, an ID or a count keep their old checks. For example, `SDL_OpenAudioDevice` still returns `0` on failure.

**Fix:** test the `bool` directly:

```c
if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
}
```

**Source:** SDL `docs/README-migration.md` (intro): "Functions that previously returned a negative error code now return bool." Each SDL3 header documents the return value, for example `SDL_Init` in `SDL_init.h`: "Returns true on success or false on failure."

**Fixture:** `fixtures/bool-returns/`.
