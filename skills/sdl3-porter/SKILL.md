---
name: sdl3-porter
description: Ports C and C++ projects from SDL2 to SDL3, including build files, includes and renamed APIs, then checks the port for changes that compile cleanly but break at runtime. Use when migrating or upgrading code from SDL2 to SDL3, or when ported SDL3 code misbehaves.
license: Zlib
---

# sdl3-porter

Port SDL2 code to SDL3, then sweep the port for traps: code that compiles cleanly against SDL3 but now means something else.

Work from SDL's documentation, never from memory. Most SDL code you have seen is SDL2, and SDL3 changed return values, ownership and defaults without changing how the code looks.

- SDL's migration guide, pinned to SDL 3.4.18: `${CLAUDE_SKILL_DIR}/scripts/sdl/docs/README-migration.md`. It has one `## SDL_<header>.h` section per header. Search it for a symbol before changing code that uses it.
- Exact SDL3 signatures: SDL 3.4.18's public headers, pinned beside the guide in `${CLAUDE_SKILL_DIR}/scripts/sdl/include/SDL3/`. Each function's comment says which version added it (`\since`), so check that against the oldest SDL3 the project supports. Never look for SDL outside the project with `find` or `locate`, whether for headers, libraries or CMake and pkg-config files. Those searches take minutes.

**Scope:** core SDL3, meaning init, events, video and OpenGL, the renderer, surfaces, input, audio, timers, the filesystem and file I/O (`SDL_IOStream`). SDL_image, SDL_ttf, SDL_mixer and SDL_net are out of scope. Port their includes and nothing else, and list them in the report.

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

Build with the project's own build system. For each round of errors:

1. Search the migration guide for the symbols, under their headers' sections.
2. Check the bundled SDL3 headers for the exact signatures.
3. Fix the code, then rebuild.

Repeat until it builds with no warnings that the SDL2 build didn't have.

Work in batches, because each search and edit costs a turn. Look up every symbol from one round of errors in a single search, for example `grep -n -E 'SDL_CreateWindow\(|SDL_RenderTexture\(|SDL_GetTicks\('` over the headers, and the same over the guide. Make all of a file's changes in one edit where they sit close together. For a small file, writing the whole ported file at once is fine.

To find out whether SDL3 is installed, ask the build: configure the project, or run `pkg-config --exists sdl3`. If SDL3 isn't installed, compile without linking instead. Compile each changed source file against the bundled headers, with the project's own include paths and defines: `cc -fsyntax-only -I"${CLAUDE_SKILL_DIR}/scripts/sdl/include"`, `c++` for C++, or `cl /Zs` with MSVC. The bundle leaves out `SDL_opengl.h` and SDL's other Khronos headers, so skip any file that includes them. Fix every error as above, and say in the report that the port was only syntax-checked.

Never add a cast just to make a type error go away. A cast that hides a changed type is itself a trap. For example, don't cast an `SDL_Rect *` to `SDL_FRect *`. Convert the rectangle instead.

Some strings that SDL2 handed over for you to free are now `const` in SDL3, because SDL owns them. When `SDL_free` or a `char *` rejects one, delete the `SDL_free` and keep the pointer `const`. Casting `const` away compiles, then frees memory SDL still owns. Two examples:

- `event.drop.data`, which was `event.drop.file` in SDL2. SDL manages event memory (migration guide, `SDL_events.h`).
- `SDL_GetBasePath()`. SDL caches the result (`SDL_filesystem.h`).

`SDL_GetPrefPath()` still returns a `char *` that you free.

### 6. Sweep for traps

Code that compiles can still be wrong. Every trap and guidance section in the files below has a "How to find it" search. Run them all at once over the project's own sources:

```sh
python3 "${CLAUDE_SKILL_DIR}/scripts/find_traps.py" src include
```

It prints the hits for each section, including hits in the SDL2 version git has committed, marked `(HEAD)`, and lists the sections with no hits. Under a hint it adds what SDL3 did with it, when SDL3 renamed or removed it. It reads only C, C++ and Objective-C sources, so hints set in config files or the environment still need `hints.md`'s own check. Without Python, run each section's search yourself.

| Subsystem | File |
|---|---|
| Init and error handling | [references/init.md](references/init.md) |
| Events | [references/events.md](references/events.md) |
| Windows, displays and display modes | [references/video.md](references/video.md) |
| Mouse, keyboard, text input, joysticks and gamepads | [references/input.md](references/input.md) |
| 2D renderer | [references/render.md](references/render.md) |
| Surfaces and palettes | [references/surfaces.md](references/surfaces.md) |
| File I/O (`SDL_RWops`, now `SDL_IOStream`) | [references/io.md](references/io.md) |
| Audio | [references/audio.md](references/audio.md) |
| Hints, set in code, the environment or config files | [references/hints.md](references/hints.md) |

Then work through the sections with hits, one at a time:

1. Read the section in its file.
2. Read every hit in context, and decide.
3. Write the verdict down before moving on: fixed, with `file:line` and the trap id; no change needed, with the reason; or needs a human. Guidance sections have no id: put their fixes under `Also changed:`, and the choices they leave to the owner under `Needs a human:`.

A section with no hits needs no verdict. A hit you can't explain away is a fix. For example, code that handles text events but never calls `SDL_StartTextInput` needs the call, whatever else it does with text.

### 7. Build and run

Rebuild. If the project's own build can't run on this machine, you may build with throwaway files outside the project, but don't add a build system to the project. Run the project's tests if it has any. If the program can run without a display (SDL's dummy drivers: `SDL_VIDEO_DRIVER=dummy SDL_AUDIO_DRIVER=dummy`, or a test mode or frame limit the program offers), run it and check it exits cleanly.

The build only compiles this platform's code. Read every SDL call inside `#ifdef` blocks for other platforms (`_WIN32`, `__APPLE__`, `__ANDROID__`, `__EMSCRIPTEN__` and the like) against the bundled headers, port it by hand, and say under `Not checked:` that those blocks weren't compiled.

Don't call the port finished until it builds and, where possible, runs. If you couldn't build or run it, say so plainly. When you say a program ran, give its own exit status: after `program | head`, `$?` is `head`'s.

### 8. Report

Use this format, one line per trap, with no emoji:

```
2 traps found, 2 fixed

src/main.c:14     bool-returns   SDL_Init returns true on success, so `!= 0` exited on every launch.
src/net.c:88      bool-returns   SDL_SetHint returns bool, so `!= 0` treated success as failure.

Also changed: src/input.c:31 SDL_GetKeyboardState now returns const bool *.
Checked, no change needed: blend-by-default (src/sprite.c:12 writes only opaque pixels).
Build: find_package(SDL3), SDL2main removed, SDL_main.h added to src/main.c.
Not checked: couldn't build here (SDL3 isn't installed).
Needs a human: audio by ear, a real gamepad, SDL_mixer (out of scope).
```

`N traps found` is the number of trap lines. Every trap line names an id from a trap file, one line for each place you fixed it, and no place twice. Nothing else is a trap. Code that wouldn't compile against SDL3, such as a removed or renamed function or a changed signature, goes under `Also changed:`, one per line, even when the port needed it. An SDL2-style check that survived such a rewrite, such as an old error test on a call you had to change, is still a trap. Take line numbers from the ported files after your last edit, for example with `grep -n`, not from the SDL2 originals.

List every trap and guidance section whose search matched code that needed no change under `Checked, no change needed:`, each with its reason.

If no traps were found, say `0 traps found` and list the trap files you checked.
