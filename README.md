# sdl3-porter

**Port SDL2 code to SDL3 without the bugs that still compile.** sdl3-porter is an agent skill that ports C and C++ code from SDL2 to SDL3, then checks the port for the changes that build cleanly but break at runtime.

SDL's rename scripts handle most of a port, and the compiler catches most of the rest. Neither catches code that still compiles but now means something else. SDL3 functions return `true` on success, so a leftover `if (SDL_Init(...) != 0)` exits on every launch. A device opened with `SDL_OpenAudioDeviceStream` starts paused, so the ported game plays no sound. Textures now filter linearly by default, so pixel art blurs. Coding agents make the same mistakes, because most of the SDL code they learned from is SDL2.

> **Status:** planning. There is nothing to install yet. See [ROADMAP.md](ROADMAP.md).

## Before and after

This port compiles against SDL3 without a warning:

```c
if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {  // SDL3 returns true on success
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;                                          // so this runs on every launch
}

SDL_AudioStream *music = SDL_OpenAudioDeviceStream(
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
SDL_PutAudioStreamData(music, samples, size);          // the device is paused: silence
```

The planned report after sdl3-porter finishes the port:

```text
3 traps found, 3 fixed

src/main.c:14     bool-returns         SDL_Init returns true on success, so `!= 0` exited on every launch.
src/audio.c:31    audio-stream-paused  SDL_OpenAudioDeviceStream starts paused. Added SDL_ResumeAudioStreamDevice.
src/sprites.c:9   linear-by-default    SDL_HINT_RENDER_SCALE_QUALITY no longer exists. Set SDL_SCALEMODE_NEAREST per texture.

Needs a human: check the window on a high-DPI display.
```

## What it does

- **Decides whether to port.** If you only need your SDL2 game to run on SDL3, it recommends [sdl2-compat](https://github.com/libsdl-org/sdl2-compat) and asks before porting.
- **Runs SDL's own rename scripts.** Pinned copies ship with the skill, so every port starts from the same mechanical renames.
- **Fixes the build** one subsystem at a time: CMake, includes, `SDL_main.h`, then each compile error.
- **Sweeps for traps** in each subsystem you use, cites `file:line` for every one it finds, fixes it and tells you what still needs a human.

v0.1 covers core SDL3: init, events, video and OpenGL, the renderer, input, audio, timers and the filesystem. SDL_image, SDL_ttf and SDL_mixer come later.

## Every trap is proven

- **Three programs per trap.** The SDL2 original, the naive port that compiles and fails, and the correct port. CI builds both ports against SDL 3.4.16 and runs them headless on Windows, Linux and macOS (OpenGL traps on Linux). The naive port must fail and the correct one must pass, or the trap doesn't ship.
- **Measured against no skill.** Evals run Claude Code on each original with and without sdl3-porter, and the README will publish both scores.
- **Sourced.** Each trap links the section of SDL's [migration guide](https://wiki.libsdl.org/SDL3/README-migration) it rests on.

## Contributing

Each trap will be one reference entry, three small fixture programs and one eval case. A contributor guide arrives with v0.1.

## License

[zlib](LICENSE), like SDL. sdl3-porter is an independent project and is not affiliated with the SDL project.
