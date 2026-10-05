# File I/O traps

Read this when the project reads or writes files or memory through `SDL_RWops`, which SDL3 calls `SDL_IOStream`. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## rwread-count-vs-bytes

**What compiles:**

```c
if (SDL_ReadIO(io, &header, sizeof header) != 1) {    /* was SDL_RWread(rw, &header, sizeof header, 1) */
    return false;                                      /* runs on every successful read */
}
```

**What breaks:** SDL2's `SDL_RWread` and `SDL_RWwrite` took an object size and a count, like `fread`, and returned the number of whole objects. SDL3 renamed them `SDL_ReadIO` and `SDL_WriteIO`, and they take one size in bytes and return the number of bytes, like POSIX `read`. The rename scripts rename the function and keep the count argument, which no longer compiles. Dropping the count leaves the old check: `!= 1` now fails every successful read, and a check against the count is wrong the same way. Dropping the size instead reads count bytes rather than count objects.

**How to find it:** search the ported sources for every read and write:

```
SDL_(ReadIO|WriteIO)\s*\(
```

Read each call together with the check or variable that uses its result, and compare it with the SDL2 call in `git diff`. The size argument has to be the total number of bytes, and the result is a number of bytes.

**Fix:** pass and compare byte counts:

```c
if (SDL_ReadIO(io, &header, sizeof header) != sizeof header) {
```

To read `count` objects, pass `size * count` and divide the result by `size`.

**Source:** SDL `docs/README-migration.md`, `SDL_rwops.h` section: "SDL_RWread and SDL_RWwrite (and the read and write function pointers) have a different function signature in SDL3, in addition to being renamed.", followed by the SDL2 signatures that "looked more like stdio" and the SDL3 ones that "look more like POSIX".

**Fixture:** `fixtures/rwread-count-vs-bytes/`.
