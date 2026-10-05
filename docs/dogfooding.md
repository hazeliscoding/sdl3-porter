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

## gbemu (2026-10-05)

[gbemu](https://github.com/jgilchrist/gbemu) is a Game Boy emulator in C++17. Its SDL2 frontend is one 204-line file, `platforms/sdl/main.cc`: a window, a renderer with vsync, and a streaming texture the emulator draws into. The port started from `436ca86`, the commit before the maintainer's own SDL3 port (`cdbcfc8`), which served as the answer key.

**Run:** a fresh headless Claude Code session (Sonnet 5, Claude Code 2.1.289) with the plugin loaded from this repo and SDL3 3.4.16 installed. It took 81 turns, 11 minutes and $2.71. gbemu's CMake build only supports GCC and Clang, and the machine only had MSVC, so the agent compiled the SDL frontend without linking (`cl /Zs`) against the installed headers. It compiled cleanly.

**Caught:**

- `vsync-flag-dropped`: added `SDL_SetRenderVSync(renderer, 1)`, as the maintainer did.
- Compile-level changes: uppercase letter key codes (`SDLK_X`), `event.key.key`, `SDL_EVENT_WINDOW_CLOSE_REQUESTED`, and `SDL_CreateWindow` without a position.

**Missed:**

- The emulator writes `(r << 16) | (g << 8) | b` into an `SDL_PIXELFORMAT_ARGB8888` texture, so every pixel's alpha is 0. SDL3 blends textures with an alpha format by default, so the port draws nothing, and the screen stays black. The maintainer's port added `0xFF << 24`. **New trap:** `blend-by-default`, with fixtures, a card in `references/render.md`, a sample and an eval case.
- The report counted 3 traps, one of them a removed event type with no trap id. **Reference fix:** `SKILL.md` step 8 now requires a trap id on every trap line.

**Also seen:** the agent set `SDL_SCALEMODE_NEAREST` on the screen texture. gbemu scales its pixels itself and draws the texture at window size, so this was harmless but not needed.

## chip8 (2026-10-05)

[AKrikler/chip8](https://github.com/AKrikler/chip8) is a CHIP-8 emulator in C99, built with a Makefile. About 215 lines use SDL2: a 64×32 streaming texture scaled up 8 times, a beeper on `SDL_OpenAudio` with a callback, and the keypad. The port started from `b00bb89`, the commit before the maintainer's own SDL3 port (`37bea12`).

**Run:** the same setup as gbemu. It took 67 turns, 7 minutes and $2.27. The agent built the emulator and its assembler with MSVC without warnings and ran the emulator with SDL's dummy drivers.

**Caught:**

- `bool-returns`: both `SDL_Init(...) < 0` checks, in `audio.c` and `video.c`.
- `audio-stream-paused`: moved the callback to `SDL_OpenAudioDeviceStream` and resumed the device.
- `linear-by-default`: set `SDL_SCALEMODE_NEAREST` on the display texture. The maintainer chose `SDL_SCALEMODE_PIXELART`.
- Compile-level changes: `SDL_CreateWindowAndRenderer`'s new title argument, `SDL_EVENT_WINDOW_FOCUS_LOST`, `event.key.key`, and the uppercase letter key codes.

**Missed:**

- The report counted 6 traps, 4 of them API changes with no trap id, and it filed the `audio-stream-paused` fix under one of those. **Reference fix:** `SKILL.md` step 8, as for gbemu.
- To build with MSVC, the agent added a `CMakeLists.txt` and a `.gitignore` entry to a project built with a Makefile. **Reference fix:** `SKILL.md` step 7 now says to build with throwaway files outside the project instead.
