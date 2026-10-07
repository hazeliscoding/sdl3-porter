# Build systems

How to move a project's build from SDL2 to SDL3. Facts come from SDL's docs at `release-3.4.18` and sdl2-compat's README at `release-2.32.72`. Where SDL's docs are silent, this file says so. Don't fill those gaps from memory.

## Contents

- [Keep the project's approach](#keep-the-projects-approach)
- [CMake](#cmake)
- [Vendored SDL](#vendored-sdl)
- [pkg-config, sdl2-config and Makefiles](#pkg-config-sdl2-config-and-makefiles)
- [Includes](#includes)
- [The main entry point](#the-main-entry-point)
- [Windows: the SDL3 DLL](#windows-the-sdl3-dll)
- [Linux](#linux)
- [Emscripten](#emscripten)
- [Package managers](#package-managers)
- [sdl2-compat: running without porting](#sdl2-compat-running-without-porting)

## Keep the project's approach

Port the build the way the project already gets SDL2. A system package stays a system package, a vendored copy stays vendored, and FetchContent stays FetchContent. Changing how SDL is obtained is a separate decision for the user.

## CMake

| SDL2 | SDL3 |
|---|---|
| `find_package(SDL2 REQUIRED)` | `find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)` |
| `SDL2::SDL2` | `SDL3::SDL3` |
| `SDL2::SDL2-static` | `SDL3::SDL3-static` (component `SDL3-static`; SDL built with `SDL_STATIC=ON`) |
| `SDL2::SDL2main` | Delete it. There is no SDL3main library. See [The main entry point](#the-main-entry-point). |
| `${SDL2_INCLUDE_DIRS}` | No equivalent. Link `SDL3::SDL3`, or `SDL3::Headers` for headers only. |
| `${SDL2_LIBRARIES}` | `SDL3_LIBRARIES` still exists and equals `SDL3::SDL3`. Prefer the target. |
| `SDL2::SDL2test` | `SDL3::SDL3_test` |

- Only `SDL3::SDL3` is guaranteed to exist. It aliases the shared library if one was built, otherwise the static one. `SDL3::SDL3-shared` and `SDL3::SDL3-static` may not exist.
- SDL3's config files no longer define `SDL3_INCLUDE_DIR(S)`, `SDL3_PREFIX`, `SDL3_BINDIR` or `SDL3_LIBDIR`. Replace uses of the SDL2 equivalents with the targets or generator expressions.
- The minimum SDL3 version is 3.2.0, the first stable release. If the project pins a version, write `find_package(SDL3 3.2.0 REQUIRED CONFIG COMPONENTS SDL3)`.

Source: `docs/README-cmake.md` ("A system SDL library"), `docs/README-migration.md` (intro), `cmake/SDL3Config.cmake.in`.

## Vendored SDL

SDL documents `add_subdirectory`:

```cmake
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
target_link_libraries(mygame PRIVATE SDL3::SDL3)
```

FetchContent provides the same targets. SDL's docs have no FetchContent example; this one pins a release by hash:

```cmake
include(FetchContent)
FetchContent_Declare(
  SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.tar.gz
  URL_HASH SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
)
FetchContent_MakeAvailable(SDL3)
```

- A vendored SDL builds a shared library by default. Set `SDL_SHARED`/`SDL_STATIC` (or `BUILD_SHARED_LIBS`) before `add_subdirectory` or `FetchContent_MakeAvailable` to match what the project did with SDL2.
- `SDL_TEST_LIBRARY` defaults to ON. `SDL_TESTS` defaults to ON only when SDL is the top-level project, and `SDL_EXAMPLES` defaults to OFF.
- A vendored SDL2 copy (for example `external/SDL2/`) is replaced, not edited. Never run the rename scripts on it.

Source: `docs/README-cmake.md` ("Using a vendored SDL", "SDL-specific CMake options", "Shared or static").

## pkg-config, sdl2-config and Makefiles

| SDL2 | SDL3 |
|---|---|
| `pkg-config --cflags --libs sdl2` | `pkg-config --cflags --libs sdl3` |
| `sdl2-config --cflags --libs` | `pkg-config --cflags --libs sdl3`. SDL 3.4.18 installs no `sdl3-config` script. |
| Autotools `PKG_CHECK_MODULES([SDL2], [sdl2])` | `PKG_CHECK_MODULES([SDL3], [sdl3])` |
| `-lSDL2main -lSDL2` | `-lSDL3`, with no main library |

Source: `docs/README-migration.md` (intro), `cmake/sdl3.pc.in`. The absence of `sdl3-config` comes from the 3.4.18 `CMakeLists.txt`, not from a doc.

## Includes

- SDL3 headers are included as `#include <SDL3/SDL.h>`. `rename_headers.py` rewrites `"SDL.h"`, `<SDL.h>` and `<SDL2/SDL.h>`, and the satellite libraries (`<SDL3_image/SDL_image.h>` and so on).
- If the build added SDL's include directory itself (for example `include_directories(${SDL2_INCLUDE_DIRS})` so that `"SDL.h"` resolved), remove it. Linking `SDL3::SDL3` provides the right include path for `<SDL3/...>`.

Source: `docs/README-migration.md` (intro).

## The main entry point

- The SDLmain library is gone. Remove `SDL2main` from the build.
- The file that defines `main` includes `<SDL3/SDL_main.h>`. Include it from exactly one file. `SDL.h` does not include it, and it `#define`s `main`.
- Keep the signature `int main(int argc, char *argv[])`.
- `SDL_MAIN_HANDLED`: keep it only if the project provides its own platform entry point (for example its own `WinMain`). Such code must call `SDL_SetMainReady()` or use `SDL_RunApp()`.
- `SDL_MAIN_NOIMPL` includes the header without generating the entry point code.
- Main callbacks (`SDL_MAIN_USE_CALLBACKS` with `SDL_AppInit`, `SDL_AppIterate`, `SDL_AppEvent` and `SDL_AppQuit`) replace `main` entirely. Don't convert a project to them during a port unless the user asks.

Source: `docs/README-main-functions.md`, `docs/README-migration.md` ("SDL_main.h"), `include/SDL3/SDL_main.h`.

## Windows: the SDL3 DLL

With a shared SDL3, the executable needs `SDL3.dll` next to it. SDL documents a post-build copy:

```cmake
if(WIN32)
  add_custom_command(
    TARGET mygame POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:mygame>
    VERBATIM
  )
endif()
```

- This only works when `SDL3::SDL3-shared` exists. A static build needs no copy.
- SDL also suggests setting absolute `CMAKE_RUNTIME_OUTPUT_DIRECTORY` and `CMAKE_LIBRARY_OUTPUT_DIRECTORY` so the DLL and the executable land together.
- `$<TARGET_RUNTIME_DLLS:mygame>` (CMake 3.21+) also works, but SDL's docs don't mention it.
- If the SDL2 build copied `SDL2.dll`, replace that step, and delete any checked-in `SDL2.dll`.
- SDL's docs don't say what choosing the console or `WIN32` subsystem changes. `SDL_main.h` provides both `main` and `WinMain`, so keep whichever the project used.

Source: `docs/README-cmake.md` ("How do I copy a SDL3 dynamic library to another location?"), `docs/README-windows.md`.

## Linux

- Building SDL3 from source needs the development packages for at least one windowing backend (X11 or Wayland). With X11, SDL also requires Xcursor and the other X11 extension libraries. The full Ubuntu and Fedora lists are in SDL's `docs/README-linux.md` ("Build Dependencies").
- `-DSDL_UNIX_CONSOLE_BUILD=ON` builds without X11 or Wayland, for programs that never open a window.

## Emscripten

- SDL's docs don't document an SDL3 replacement for `-sUSE_SDL=2`. What they document: build SDL3 with `emcmake cmake` and link `libSDL3.a`.
- A blocking `while` main loop doesn't work on Emscripten. Use `emscripten_set_main_loop` or main callbacks.
- Flag an Emscripten build for the user instead of guessing.

Source: `docs/README-emscripten.md`.

## Package managers

SDL's docs don't cover vcpkg, Conan, Homebrew or apt (MSYS2 is the exception: `mingw-w64-ucrt-x86_64-sdl3`). Check the package manager's own index for an SDL3 package and confirm its version is 3.2.0 or later. Don't guess package names.

## sdl2-compat: running without porting

sdl2-compat "provides a binary and source compatible API for programs written against SDL2, but it uses SDL3 behind the scenes."

- **At runtime:** it replaces `SDL2.dll` or `libSDL2` for an existing binary. It needs SDL3 available at runtime. `SDL_DYNAMIC_API=<path to sdl2-compat>` works even when SDL2 is statically linked.
- **At build time:** it ships SDL2-compatible headers, CMake targets (`SDL2::SDL2`, `SDL2::SDL2main`), `sdl2.pc` and `sdl2-config`, so the project builds unchanged. Static builds are supported only on Linux.
- **Limits:** apps that call the window system directly may break under a different video driver, for example X11 code under Wayland. The documented workaround is `SDL_VIDEODRIVER=x11`.
- **SDL's advice:** "If you are writing new code, please target SDL3 directly and do not use this layer."

Recommend sdl2-compat when the user wants the SDL2 program to run on SDL3 without changing code. Port when they want SDL3's APIs, or the code will keep being developed.

Source: sdl2-compat `README.md` and `HOW_TO_TEST_GAMES.md` at `release-2.32.72`.
