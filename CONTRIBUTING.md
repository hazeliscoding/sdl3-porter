# Contributing

The most useful contribution is a trap: SDL2 code that still compiles against SDL3 but breaks at runtime. Found one in a real port? Open a "missed trap" issue, or add it yourself as described below.

## What counts as a trap

A trap has to pass all three tests:

- **It compiles cleanly.** The naive SDL3 port builds with `-Wall -Wextra -Werror` on GCC and Clang and `/W4 /WX` on MSVC. If any compiler warns about it, the compiler already catches it.
- **It breaks headless.** A small program shows the failure without a display, audio hardware, gamepads or network: SDL's dummy or offscreen video driver, the dummy audio driver, the virtual joystick API and the software renderer.
- **SDL documents it.** The card cites the section of SDL's migration guide or the SDL wiki page it rests on.

A change that fails one of these can still go in a reference file as guidance, like `references/events.md`.

## Adding a trap

Pick a kebab-case id, such as `audio-stream-paused`. Ids are stable once added. A trap is four pieces, and CI checks that they match:

1. **A card:** a `## <id>` section in `skills/sdl3-porter/references/<subsystem>.md`, with **What compiles**, **What breaks**, **How to find it**, **Fix** and **Source** parts. Add the file to the table in `SKILL.md` step 6 if it's new.
2. **Three fixtures** in `fixtures/<id>/`: `sdl2/main.c`, the original against SDL 2.32; `naive/main.c`, the port that keeps the trap; and `fixed/main.c`, the correct port. Each is a small C program that tests one behavior.
   - A failing check exits non-zero and prints one line that starts with the trap id, such as `bool-returns: SDL_Init reported failure`. Failures in setup, such as a window that doesn't open, start with `setup:` instead.
   - In the naive fixture, one comment at the trap line saying what SDL3 changed is enough.
   - The original and the fixed port must pass, and the naive port must fail with its trap's line or crash.
3. **A sample:** `samples/<id>/CMakeLists.txt` and `samples/<id>/src/main.c`, a plausible SDL2 program that contains the trap. Give it a neutral name and no self-check, so it doesn't point the model at the trap.
4. **An eval case** in `evals/<id>/`: `case.yaml`, a `scaffold.sh` that copies the sample, a `skill-fired.md` grader, and one or more regex graders named `trap-*.md` that check the fix. Each trap grader must fail on a port that only ran SDL's rename scripts.

Copy an existing trap, such as `blend-by-default`, as a starting point.

## Checks

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
claude plugin validate .
```

Configuring needs Python 3, and the first run downloads and builds SDL 3.4.16 and SDL 2.32.10. CI runs the same checks on Windows, Linux and macOS. Evals cost model usage, so the maintainer runs them by hand before releases.

## Commits

Use [Conventional Commits](https://www.conventionalcommits.org/), such as `feat(skill): add the blend-by-default trap`, and keep each commit to one change.
