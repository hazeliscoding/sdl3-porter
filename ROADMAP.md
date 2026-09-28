# Roadmap

sdl3-porter is an agent skill that ports C and C++ code from SDL2 to SDL3 and catches the changes that compile cleanly but break at runtime. This file tracks what gets built, in what order, and the decisions already made.

## Decisions (2026-09-27)

- **Why SDL3.** No public SDL3 skill existed on 2026-09-27. Projects were writing their own inside their repos ([AltirraSDL](https://github.com/ilmenit/AltirraSDL), [ja2-stracciatella](https://github.com/ja2-stracciatella/ja2-stracciatella)). SDL3 has been stable since January 2025, and most of the SDL code agents learned from is SDL2.
- **One job: correct SDL3 code.** This repo holds the skill and, later, tools for the same job, such as a trap checker and a hook. Unrelated skills live in their own repos.
- **v0.1 covers core SDL3:** init, events, video and OpenGL, the renderer, input, audio, timers and the filesystem. SDL_image, SDL_ttf, SDL_mixer and the GPU API come later.
- **A trap is code that compiles against SDL3 but breaks at runtime.** Compile errors are handled by the compile loop and the references, not by trap cards.
- **A trap exists only if it's proven.** Its naive fixture must fail headless in CI and its fixed fixture must pass. Anything that can't be reproduced headless becomes reference guidance instead.
- **Every trap cites its source:** the section of SDL's README-migration or the SDL wiki page it rests on.
- **Verification has two layers.** CI builds the fixtures on every push, with no model involved. `claude plugin eval` runs Claude Code on each SDL2 original with and without the skill, grades the port with regex graders and reports the difference. Evals cost API calls, so they run by hand before releases.
- **Mechanical renames use SDL's own scripts.** Pinned copies of `rename_headers.py`, `rename_symbols.py`, `rename_macros.py` and `SDL_oldnames.h` from `release-3.4.16` ship in `skills/sdl3-porter/scripts/sdl/`, laid out as in the SDL tree so the scripts run unmodified, with SDL's zlib notice. CI checks they match the tag byte for byte. They need Python 3. Without it, the agent renames by hand from the migration guide. The Coccinelle patch is mentioned but not relied on.
- **Port only when it's needed.** When the goal is only to run on SDL3, the skill recommends sdl2-compat and asks before porting.
- **Supported versions:** SDL 3.2.0 and later. Fixtures are pinned to SDL 3.4.16, and SDL2 originals to the latest SDL 2.32 release.
- **Fixtures are small C programs,** one behavior each. The skill ports C and C++.
- **Headless CI:** the offscreen video driver, dummy or disk audio, the virtual joystick API and the software renderer with `SDL_RenderReadPixels`. OpenGL traps run on Linux with Mesa. Every other trap runs on Windows, Linux and macOS.
- **Claude Code first.** The repo is a Claude Code plugin with its own marketplace file. The SKILL.md also works as a plain Agent Skill, and the README calls that untested until an eval covers another harness.
- **Name:** `sdl3-porter`, for the repo, the plugin and the skill (`/sdl3-porter`).
- **License:** zlib, matching SDL and the bundled scripts.
- **The logo is option A, "Chevron tile":** `›3` in a rounded orange tile, beside the `sdl3-porter` wordmark, both in JetBrains Mono ExtraBold. The accent is `#c2410c` on light backgrounds and `#fb923c` on dark ones.

## M0: Placeholder (as soon as possible)

- [x] Add `LICENSE` (zlib), `.gitignore` and `.gitattributes`.
- [x] Write `README.md`, `ROADMAP.md`, `AGENTS.md` and `CLAUDE.md`.
- [x] Brand: pick a logo option from a claude.ai design project, export `mark.svg` and `lockup.svg` with `-dark` variants to `docs/brand/`, and add the `<picture>` header to the README. Convert the text to paths.
- [ ] Plugin skeleton: `.claude-plugin/plugin.json`, `.claude-plugin/marketplace.json` and a placeholder `skills/sdl3-porter/SKILL.md`.
- [ ] Fixture harness: a top-level `CMakeLists.txt` that fetches SDL 3.4.16 and registers each `fixtures/<id>/naive` and `fixtures/<id>/fixed` program as a CTest test. Naive tests are marked `WILL_FAIL`, so a naive fixture that passes breaks the build.
- [ ] First trap, `bool-returns`: an `SDL_Init(...) != 0` check that exits on success.
- [ ] CI: configure, build and `ctest` on Linux, Windows and macOS, with the offscreen video and dummy audio drivers.

**Done when:** CI is green on all three platforms, with `bool-returns` naive failing and fixed passing, and the plugin installs from its local marketplace and `/sdl3-porter` loads in Claude Code.

## M1: Porting workflow

- [ ] `SKILL.md` workflow, under 500 lines: survey, port or sdl2-compat, build system, renames, compile loop, trap sweep, report. Details go in `references/`.
- [ ] `references/build.md`: `find_package(SDL3)`, FetchContent, pkg-config, include paths, `SDL_main.h`, and copying the SDL3 DLL on Windows.
- [ ] Bundle `scripts/sdl/build-scripts/` (the three rename scripts) and `scripts/sdl/include/SDL3/SDL_oldnames.h` from `release-3.4.16`, with SDL's license text and a `SOURCE` file naming the tag.
- [ ] CI check: the bundled files are byte-identical to the tag.
- [ ] A sample SDL2 program that uses video, the renderer, input and audio, for the whole-port eval.
- [ ] Eval `port-sample`: the skill fires, no `SDL2/` include remains, and `SDL3/SDL.h` is included.

**Done when:** the byte-identity check passes and fails on a tampered copy, and `port-sample` passes 3 of 3 runs.

## M2: Traps

- [ ] Trap cards in `references/<subsystem>.md`. Each has: what compiles, what breaks, how to find it, the fix and the source.
- [ ] Fixtures for each trap: `sdl2/` (builds against SDL 2.32), `naive/` and `fixed/`.
- [ ] The traps:

| Area | Trap | What breaks |
|---|---|---|
| Init | `bool-returns` | A leftover `SDL_Init(...) != 0` treats success as failure and exits on every launch |
| Memory | `drop-data-freed` | `SDL_free` on drop-event data is a double free, because SDL owns event memory |
| Memory | `text-event-pointer-kept` | Keeping `event.text.text` after the event leaves a dangling pointer |
| Memory | `base-path-freed` | `SDL_GetBasePath()` now returns memory SDL owns, so freeing it corrupts the heap |
| Input | `text-input-off` | Text input is no longer on by default, so no text events arrive |
| Input | `gamepad-index-vs-id` | `which` is now an instance ID, not an index, so the wrong pad or none opens |
| Render | `vsync-flag-dropped` | Dropping the removed `SDL_RENDERER_PRESENTVSYNC` flag leaves the loop unthrottled |
| Render | `linear-by-default` | The scale-quality hint is gone and textures default to linear, so pixel art blurs |
| Render | `rect-cast-to-frect` | Casting `SDL_Rect*` to `SDL_FRect*` draws garbage geometry |
| OpenGL | `gl-attributes-after-window` | GL attributes set after the window exists don't set its depth, stencil or MSAA |
| Audio | `audio-stream-paused` | A device opened with `SDL_OpenAudioDeviceStream` starts paused, so nothing plays |
| Audio | `mix-volume-float` | `SDL_MIX_MAXVOLUME` (128) passed as a 0–1 float gives 128× gain and clipping |
| Hints | `hint-string-ignored` | Hints written as renamed or removed string literals are silently ignored |

- [ ] Consistency check: every trap card has a fixture folder, and every fixture folder has a trap card.

**Done when:** every naive fixture fails and every fixed fixture passes, the consistency check passes and fails on a card without fixtures, and any trap that couldn't be reproduced headless has moved to guidance with a decision recorded here.

## M3: Evals

- [ ] One eval case per trap: scaffold its `sdl2/` program, ask for a port, and grade that the skill fired, the naive pattern is gone and the fix is present.
- [ ] A whole-program case: the sample with several traps at once.
- [ ] A README table of with-skill and without-skill scores per trap, with the model and the date.

**Done when:** every case scores at least 0.9 with the skill across 3 runs, and the README table is generated from `aggregate-result.json`.

## M4: Dogfood

- [ ] Port jage to SDL3 with the skill.
- [ ] Port 2 public SDL2 programs in scratch forks. No upstream PRs.
- [ ] `docs/dogfooding.md`: what it caught and what it missed. Every miss becomes a trap, a reference fix or a Later item.

**Done when:** the log has at least 3 entries and every miss has been turned into one of those three.

## M5: v0.1.0

- [ ] README quick start: install with `/plugin marketplace add hazeliscoding/sdl3-porter`, checked on a clean machine.
- [ ] Document what's supported: SDL 3.2 and later, C and C++, Claude Code. Other harnesses are untested.
- [ ] `CONTRIBUTING.md`: how to add a trap as a card, three fixtures and an eval case.
- [ ] `SECURITY.md`: the skill edits your code and runs the bundled Python scripts and your build. It downloads nothing at runtime.
- [ ] A "missed trap" issue template that asks for the SDL version, a minimal SDL2 snippet and what went wrong.
- [ ] Set `plugin.json` to 0.1.0, tag it and publish a GitHub release.

**Done when:** on a clean machine, someone can install from the README, port the sample and build and run it, CI is green, and there are no known critical bugs.

## Later

- A deterministic trap checker built from the trap cards' patterns, runnable in CI without a model, and a hook that runs it on every file the agent edits.
- SDL_image 3, SDL_ttf 3 and SDL_mixer 3.
- Writing new SDL3: main callbacks, properties and the GPU API.
- High-DPI guidance, with a visual check.
- Evals for Codex and other harnesses.
- A catalog marketplace repo that lists this and future skills.
- Listings in plugin directories.

## Not planned

- SDL 1.2 to SDL3.
- Language bindings (SDL3-CS, Rust, Zig).
- Engines built on SDL.
- Keeping SDL2 code working. sdl2-compat does that.
- A hosted service, accounts or telemetry.

## How we'll know it works

Evidence comes from the eval scores with and without the skill, and from the dogfooding log (n=1, plus scratch ports). After release it also comes from public signals: installs, "missed trap" issues, PRs that add traps and SDL projects that link to it.
