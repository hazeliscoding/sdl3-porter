# Dogfooding

What sdl3-porter caught and missed on real SDL2 code. Every miss becomes a trap, a reference fix or a Later item in `ROADMAP.md`.

## jage (2026-10-05)

[jage](https://github.com/hazeliscoding/jage) is a small C++17 engine on SDL2 and OpenGL 4.1, built with premake and a vendored SDL2. About 1,150 of its lines touch SDL: the window and GL context, events, the keyboard, the mouse and game controllers.

**Run:** a fresh headless Claude Code session (Sonnet 5, Claude Code 2.1.289) with the plugin loaded from this repo, on a local scratch branch. It took 124 turns, 13 minutes and $4.32. SDL3 isn't installed, and jage's last commit only builds with workarounds, so nothing was built.

**Caught:**

- `bool-returns` in `engine.cpp`: `SDL_Init(...) < 0` became `!SDL_Init(...)`.
- `SDL_INIT_EVERYTHING`, which SDL3 removed, became `SDL_INIT_VIDEO | SDL_INIT_GAMEPAD`, so controllers still connect.
- Gamepad events' `which` is an instance ID for both adding and removing a pad. That also fixes jage's SDL2 bug of matching removals against a device index.
- Compile-level changes, read from the bundled headers: the keyboard state is `const bool *`, the mouse position is in floats, `SDL_GL_GetProcAddress` returns a function pointer, `SDL_GetVersion` replaces `SDL_VERSION`, and `SDL_main.h` is included.
- It left jage's GL attributes, which are set after the window exists, alone and reported them as a bug that predates the port.

**Missed:**

- jage's `window.h` forward-declares `using SDL_GLContext = void*;`, which conflicts with SDL3's `typedef struct SDL_GLContextState *SDL_GLContext`, so the port doesn't compile. Compiling the changed files against the bundled headers without linking finds it in one pass. **Reference fix:** `SKILL.md` step 5 now says to do that when SDL3 isn't installed.
- The report gave renamed APIs made-up trap ids (`api-signature`, `opaque-type-tag`) and counted 6 traps, where the trap files have 1. **Reference fix:** `SKILL.md` step 8 now counts only ids from the trap files and lists other changes under `Also changed:`.

**Also seen:**

- `rename_headers.py` left jage's lowercase `<sdl2/SDL.h>` includes alone. The agent found them in `git diff` and fixed them by hand.
- The agent replaced the vendored SDL2 with the skill's SDL3 headers, without libraries, and said that SDL3's `.lib` and `.dll` still need adding.
