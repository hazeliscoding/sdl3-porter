---
name: sdl3-porter
description: Ports C and C++ projects from SDL2 to SDL3, including build files, includes and renamed APIs, then checks the port for changes that compile cleanly but break at runtime. Use when migrating or upgrading code from SDL2 to SDL3, or when ported SDL3 code misbehaves.
license: Zlib
---

# sdl3-porter

Port SDL2 code to SDL3, then sweep the port for traps: code that compiles cleanly against SDL3 but now means something else.

Work from SDL's documentation, never from memory. Most SDL code you have seen is SDL2, and SDL3 changed return values, ownership and defaults without changing how the code looks.

- SDL's migration guide, pinned to SDL 3.4.16: `${CLAUDE_SKILL_DIR}/scripts/sdl/docs/README-migration.md`. It has one `## SDL_<header>.h` section per header. Search it for a symbol before changing code that uses it.
- Exact SDL3 signatures: SDL 3.4.16's public headers, pinned beside the guide in `${CLAUDE_SKILL_DIR}/scripts/sdl/include/SDL3/`. Each function's comment says which version added it (`\since`), so check that against the oldest SDL3 the project supports. Don't search the filesystem for other SDL headers: a whole-disk search can take minutes.

**Scope:** core SDL3, meaning init, events, video and OpenGL, the renderer, input, audio, timers and the filesystem. SDL_image, SDL_ttf, SDL_mixer and SDL_net are out of scope. Port their includes and nothing else, and list them in the report.

## Workflow

Copy this checklist into your response and tick each item as you finish it:

```
SDL3 port:
- [ ] 1. Survey the project
- [ ] 2. Port, or recommend sdl2-compat
- [ ] 3. Port the build
- [ ] 4. Run SDL's rename scripts
- [ ] 5. Build and fix until it compiles
- [ ] 6. Sweep for traps
- [ ] 7. Build and run
- [ ] 8. Report
```

### 1. Survey the project

- How does the build get SDL2? Look for `find_package(SDL2`, `SDL2::`, `sdl2-config`, `pkg-config` with `sdl2`, a vendored copy (for example `external/SDL2/`), FetchContent, Makefiles, Visual Studio projects or meson files.
- Which paths hold the project's own source? Which hold vendored SDL, other third-party code or build output? The rename scripts must never touch the second group.
- Which SDL subsystems does the code use? Check the `SDL_Init` flags and the API prefixes. This decides which trap files to read in step 6.
- Are SDL_image, SDL_ttf, SDL_mixer or SDL_net used? They are out of scope.
- Run `git status`. The port must be reviewable as a diff. If there are uncommitted changes, ask the user before going on. If the project isn't a git repository, say so and suggest a backup first.

### 2. Port, or recommend sdl2-compat

If the user asked to port the code, or wants SDL3's APIs, port it.

If they only want an existing SDL2 program to run on SDL3, recommend sdl2-compat instead and ask before porting. See [build.md: sdl2-compat](references/build.md#sdl2-compat-running-without-porting).

### 3. Port the build

Follow [references/build.md](references/build.md). The essentials:

- Keep the project's way of getting SDL. A system package stays a package, and a vendored copy stays vendored.
- `SDL2::SDL2` becomes `SDL3::SDL3`, and `find_package(SDL2 ...)` becomes `find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)`.
- Remove `SDL2main` from the build. Add `#include <SDL3/SDL_main.h>` to the one file that defines `main`.

### 4. Run SDL's rename scripts

SDL's own scripts do the mechanical renames: headers, then symbols, then macros. They edit files in place. Pass only the project's own source paths, never the repository root, `.git`, build output or a vendored SDL:

```sh
python3 "${CLAUDE_SKILL_DIR}/scripts/sdl/build-scripts/rename_headers.py" src include
python3 "${CLAUDE_SKILL_DIR}/scripts/sdl/build-scripts/rename_symbols.py" --all-symbols src include
python3 "${CLAUDE_SKILL_DIR}/scripts/sdl/build-scripts/rename_macros.py" src include
```

- Replace `src include` with the project's source paths.
- Use whichever of `python3`, `python` or `py -3` runs Python 3.
- Without Python, rename by hand from the "renamed" lists in the migration guide.
- Then read `git diff`. Resolve every `FIXME` comment that `rename_macros.py` added.

The scripts only rename. They don't fix changed return values, ownership or defaults. That's step 6.

### 5. Build and fix until it compiles

Build with the project's own build system. For each error:

1. Search the migration guide for the symbol, under its header's section.
2. Check the bundled SDL3 header for the exact signature.
3. Fix the code, then rebuild.

Repeat until it builds with no warnings that the SDL2 build didn't have.

If SDL3 isn't installed, compile without linking instead. Compile each changed source file against the bundled headers, with the project's own include paths and defines: `cc -fsyntax-only -I"${CLAUDE_SKILL_DIR}/scripts/sdl/include"`, `c++` for C++, or `cl /Zs` with MSVC. The bundle leaves out `SDL_opengl.h` and SDL's other Khronos headers, so skip any file that includes them. Fix every error as above, and say in the report that the port was only syntax-checked.

Never add a cast just to make a type error go away. A cast that hides a changed type is itself a trap. For example, don't cast an `SDL_Rect *` to `SDL_FRect *`. Convert the rectangle instead.

Some strings that SDL2 handed over for you to free are now `const` in SDL3, because SDL owns them. When `SDL_free` or a `char *` rejects one, delete the `SDL_free` and keep the pointer `const`. Casting `const` away compiles, then frees memory SDL still owns. Two examples:

- `event.drop.data`, which was `event.drop.file` in SDL2. SDL manages event memory (migration guide, `SDL_events.h`).
- `SDL_GetBasePath()`. SDL caches the result (`SDL_filesystem.h`).

`SDL_GetPrefPath()` still returns a `char *` that you free.

### 6. Sweep for traps

Code that compiles can still be wrong. For each subsystem the project uses, read its file and check every trap and guidance section in it against all ported files:

| Subsystem | File |
|---|---|
| Init and error handling (always read) | [references/init.md](references/init.md) |
| Events | [references/events.md](references/events.md) |
| Keyboard, text input, joysticks and gamepads | [references/input.md](references/input.md) |
| 2D renderer | [references/render.md](references/render.md) |
| Audio | [references/audio.md](references/audio.md) |
| Hints, set in code, the environment or config files | [references/hints.md](references/hints.md) |

For each trap:

1. Run its "How to find it" search.
2. Read every hit in context.
3. Fix the ones that match, and note `file:line` and the trap id.

### 7. Build and run

Rebuild. If the project's own build can't run on this machine, you may build with throwaway files outside the project, but don't add a build system to the project. Run the project's tests if it has any. If the program can run without a display (SDL's dummy drivers: `SDL_VIDEO_DRIVER=dummy SDL_AUDIO_DRIVER=dummy`, or a test mode or frame limit the program offers), run it and check it exits cleanly.

Don't call the port finished until it builds and, where possible, runs. If you couldn't build or run it, say so plainly.

### 8. Report

Use this format, one line per trap, with no emoji:

```
2 traps found, 2 fixed

src/main.c:14     bool-returns   SDL_Init returns true on success, so `< 0` never caught a failure.
src/net.c:88      bool-returns   SDL_SetHint returns bool, so `!= 0` treated success as failure.

Also changed: src/input.c:31 SDL_GetKeyboardState now returns const bool *.
Build: find_package(SDL3), SDL2main removed, SDL_main.h added to src/main.c.
Not checked: couldn't build here (SDL3 isn't installed).
Needs a human: audio by ear, a real gamepad, SDL_mixer (out of scope).
```

Every trap line names an id from a trap file, one line for each place you fixed it. Nothing else is a trap. Put renamed or removed APIs, changed signatures and every other change under `Also changed:`, one per line, even when the port needed them, and leave them out of `N traps found`.

If no traps were found, say `0 traps found` and list the trap files you checked.
