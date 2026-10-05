# File I/O traps

Read this when the project reads or writes files or memory through `SDL_RWops`, which SDL3 calls `SDL_IOStream`. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime. The guidance sections after the traps are changes no headless fixture can reproduce, so check them by reading the code.

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

## Apple bundle paths

**What changed:** on macOS and iOS, SDL2's `SDL_RWFromFile` looked for a file inside the app bundle's resource directory first, then fell back to the path as given. Anything that opened files through it, such as `SDL_LoadBMP` and `SDL_LoadWAV`, found bundled assets by relative path. SDL3's `SDL_IOFromFile` uses the path unchanged, so a relative path resolves against the current directory, which for an app started from Finder or the home screen isn't the bundle. The assets load when the program runs from the build directory, and fail in the packaged app.

**How to find it:** this matters only if the project ships an Apple app bundle, so check the build for `MACOSX_BUNDLE`, an `Info.plist` or an Xcode project. Then search the ported sources for files opened by relative path:

```
SDL_IOFromFile\s*\(|SDL_LoadBMP\s*\(|SDL_LoadWAV\s*\(
```

**Fix:** build asset paths from `SDL_GetBasePath()`, which on Apple platforms returns the bundle's resource directory, and on other platforms the directory that holds the program:

```c
char path[1024];
SDL_snprintf(path, sizeof path, "%sassets/player.bmp", SDL_GetBasePath());
SDL_Surface *player = SDL_LoadBMP(path);
```

If a loading helper already builds paths from a base directory, change it there. Checking the bundled app is for a human on a Mac.

**Source:** SDL `docs/README-migration.md`, `SDL_rwops.h` section: "On Apple platforms, SDL_RWFromFile (now called SDL_IOFromFile) no longer tries to read from inside the app bundle's resource directory, instead now using the specified path unchanged. One can use SDL_GetBasePath() to find the resource directory on these platforms." SDL2's lookup is `SDL_OpenFPFromBundleOrFallback` in `src/file/cocoa/SDL_rwopsbundlesupport.m` at `release-2.32.10`.
