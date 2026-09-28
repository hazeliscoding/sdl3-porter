---
name: sdl3-porter
description: Ports C and C++ code from SDL2 to SDL3 and checks the port for changes that compile cleanly but break at runtime. Use when migrating, upgrading or porting a project from SDL2 to SDL3, when SDL3 code behaves differently from the SDL2 version it replaced, or when SDL2-era patterns (SDL_Init(...) != 0 checks, SDL_RenderCopy, audio callbacks, SDL_GameController) show up in SDL3 code.
license: Zlib
---

# sdl3-porter

**Status: placeholder.** The porting workflow and the trap checks are not written yet. They arrive in the project's M1 and M2 milestones.

Until then:

1. Tell the user that sdl3-porter is a placeholder and can't yet check a port for runtime traps.
2. Work from SDL's own migration guide, `docs/README-migration.md` in the SDL3 source tree or https://wiki.libsdl.org/SDL3/README-migration, not from memory.
3. Remember the most common silent change: SDL3 functions return `bool`, true on success. An SDL2-style `if (SDL_Init(...) != 0)` check now fails on every successful launch, and `< 0` never catches a failure.
4. Don't tell the user a port is finished until it builds and runs.
