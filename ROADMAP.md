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
- **Fixtures compile with warnings as errors** (`-Wall -Wextra -Werror`, `/W4 /WX`). This enforces "compiles cleanly". If any compiler warns about a naive port, the compiler already catches it, so it becomes reference guidance instead of a trap.
- **SDL builds from a hash-checked source archive as a static library,** so the build is the same locally and in CI, and Windows needs no DLL copying.
- **Headless CI:** the offscreen video driver, dummy or disk audio, the virtual joystick API and the software renderer with `SDL_RenderReadPixels`. OpenGL traps run on Linux with Mesa. Every other trap runs on Windows, Linux and macOS.
- **Claude Code first.** The repo is a Claude Code plugin with its own marketplace file. The SKILL.md also works as a plain Agent Skill, and the README calls that untested until an eval covers another harness.
- **Name:** `sdl3-porter`, for the repo, the plugin and the skill (`/sdl3-porter`).
- **License:** zlib, matching SDL and the bundled scripts.
- **The logo is option A, "Chevron tile":** `›3` in a rounded orange tile, beside the `sdl3-porter` wordmark, both in JetBrains Mono ExtraBold. The accent is `#c2410c` on light backgrounds and `#fb923c` on dark ones.

### Added during M0 (2026-09-27)

- **Fixtures default to the dummy video and audio drivers.** A trap that needs rendering or OpenGL sets the offscreen driver on its own test.
- **The plugin has no version until 0.1.0.** Installs follow the latest commit until then. `claude plugin validate` warns about it, which is expected.
- **Installed as a plugin, the skill's name is `sdl3-porter:sdl3-porter`.** Eval graders match it with `(?:[\w-]+:)?sdl3-porter`.
- **No CI cache for now.** SDL builds in about 90 seconds per platform. Add a cache only if CI gets slow.
- **Linux CI installs SDL's X11 and Mesa packages.** SDL refuses to configure with X11 but without Xcursor and its other X11 dependencies.

### Added during M1 (2026-09-27)

- **SDL's migration guide ships with the skill.** `docs/README-migration.md` from `release-3.4.16` sits beside the rename scripts, so the agent works from pinned docs, not memory. The byte-identity check covers it.
- **The `bool-returns` trap card is written in M1** (`references/init.md`), because the trap sweep needs at least one card. M2 adds the rest.
- **A smoke test pins what the rename scripts do.** It runs them from the skill's layout on the bounce sample, and checks that they rename the APIs and leave the `bool-returns` and `mix-volume-float` traps in place.
- **Controls pass only on their expected failure.** The tampered-bundle and unrenamed-sample controls match their specific failure message, not any failure. Naive fixtures stay `WILL_FAIL` until M2 adds a runner that also handles crashes.
- **Configuring the harness requires Python 3**, for the rename smoke test.
- **Evals run in GitHub Actions on Ubuntu, by hand.** Claude Code can't sandbox a shell tool on Windows, so it refuses evals that grant Bash there. The `Evals` workflow runs only on `workflow_dispatch`, with Sonnet 5, a $20 cost ceiling and a token from a repository secret.
- **`port-sample` doesn't build the port.** SDL3 isn't installed in the eval sandbox, so the prompt says so and the graders check files. M3 revisits building inside evals.
- **Eval file graders take one file each.** Globs in `{source: file, path}` aren't supported.
- **`port-sample` passed 3 of 3 runs** on 2026-09-27, with Sonnet 5 and Claude Code 2.1.283: all 8 graders in every run, 50–53 of the 60 allowed turns, about 6 minutes and $1.20–1.50 per run, $3.95 in total. Raise `max_turns` if a later run hits the limit.
- **Transcripts are opt-in.** The `Evals` workflow uploads each run's `trace.jsonl` only when started with `traces=true`, because artifacts on a public repo are readable by anyone signed in. They expire after 7 days.

### Added during M2 (2026-09-29)

- **SDL2 originals build against SDL 2.32.10 and must pass.** It's the latest 2.32 release, fetched from a hash-checked archive into the same build as SDL3. The two share option names, so both build static with the C library. Each original runs headless with SDL2's `SDL_VIDEODRIVER` and `SDL_AUDIODRIVER` variables. A passing original proves the fixture's check holds on SDL2, so the naive failure comes from the port.
- **Naive fixtures run through `cmake/run-naive.cmake` instead of `WILL_FAIL`.** CTest can't count a crash as an expected failure. The test passes only when the naive port exits non-zero after printing a line that starts with `<id>: `, or crashes. A port that passes, hangs or fails without that line fails the test. Controls cover a passing program, a failure without the line and a crash.
- **A trap card is a `## <id>` section of `references/*.md` with a `**What compiles:**` part.** The `trap-cards` check matches those ids against the `fixtures/` folders, with controls for a card without fixtures and for fixtures without a card.
- **`drop-data-freed` and `base-path-freed` are guidance, not traps (2026-09-30).** SDL3 makes `SDL_DropEvent.data` and `SDL_GetBasePath()` `const char *`, so no naive port that frees them compiles cleanly. In C, GCC, Clang and MSVC warn about the dropped `const` even without warning flags, and in C++ it's an error. Only a cast gets it through, and a naive fixture may not silence a warning. `SKILL.md` step 5 now says to drop the `SDL_free` instead of casting.
- **Fixture setup failures print `setup:`, not the trap id (2026-09-30).** Only the trap's own check starts its line with `<id>: `, so the naive runner can't mistake a failed `SDL_Init` or device open for the trap.
- **`mix-volume-float` qualifies through integer literals, macros and 8- and 16-bit integer variables, not `int` variables (2026-10-01).** SDL3 removed `SDL_MIX_MAXVOLUME`, so code that still uses it doesn't compile, and MSVC `/W4` warns (C4244) when a non-constant `int` expression is passed as the float volume. Literals, macros, including a re-defined `SDL_MIX_MAXVOLUME`, and plain 8- and 16-bit integer variables compile cleanly on MSVC, GCC and Clang, as C and as C++. Several public SDL3 ports re-define the macro, and the bounce sample uses it, so the naive fixture does that. The card shows the literal form too, and its search still lists `int` variables, because GCC and Clang build them silently. SDL3 doesn't clamp the volume, so 16-bit samples wrap rather than clip.
- **`text-event-pointer-kept` is guidance, not a trap (2026-10-02).** SDL3 frees text-event memory at the next event pump, but only its platform keyboard backends create that memory, through the internal `SDL_SendKeyboardText`. An event pushed with `SDL_PushEvent` keeps the app's own pointer, which SDL never frees, and the dummy and offscreen drivers produce no keyboard input. So no headless fixture can make the naive port fail. `references/events.md` covers it, together with drop-event data, which has the same lifetime.
- **Software-rendering traps run on the dummy video driver (2026-10-04).** The dummy drivers of SDL2 and SDL3 both give windows a framebuffer, so the software renderer and `SDL_RenderReadPixels` work without the offscreen driver. Only OpenGL traps need offscreen.

## M0: Placeholder (as soon as possible)

- [x] Add `LICENSE` (zlib), `.gitignore` and `.gitattributes`.
- [x] Write `README.md`, `ROADMAP.md`, `AGENTS.md` and `CLAUDE.md`.
- [x] Brand: pick a logo option from a claude.ai design project, export `mark.svg` and `lockup.svg` with `-dark` variants to `docs/brand/`, and add the `<picture>` header to the README. Convert the text to paths.
- [x] Plugin skeleton: `.claude-plugin/plugin.json`, `.claude-plugin/marketplace.json` and a placeholder `skills/sdl3-porter/SKILL.md`.
- [x] Fixture harness: a top-level `CMakeLists.txt` that fetches SDL 3.4.16 and registers each `fixtures/<id>/naive` and `fixtures/<id>/fixed` program as a CTest test. Naive tests are marked `WILL_FAIL`, so a naive fixture that passes breaks the build.
- [x] First trap, `bool-returns`: an `SDL_Init(...) != 0` check that exits on success.
- [x] CI: configure, build and `ctest` on Linux, Windows and macOS, with the dummy video and audio drivers, plus `claude plugin validate`.

**Done when:** CI is green on all three platforms, with `bool-returns` naive failing and fixed passing, and the plugin installs from its local marketplace and `/sdl3-porter` loads in Claude Code.

## M1: Porting workflow

- [x] `SKILL.md` workflow, under 500 lines: survey, port or sdl2-compat, build system, renames, compile loop, trap sweep, report. Details go in `references/`.
- [x] `references/build.md`: `find_package(SDL3)`, FetchContent, pkg-config, include paths, `SDL_main.h`, and copying the SDL3 DLL on Windows.
- [x] Bundle `scripts/sdl/build-scripts/` (the three rename scripts), `scripts/sdl/include/SDL3/SDL_oldnames.h` and `scripts/sdl/docs/README-migration.md` from `release-3.4.16`, with SDL's license text and a `SOURCE` file naming the tag.
- [x] CI check: the bundled files are byte-identical to the tag.
- [x] A sample SDL2 program that uses video, the renderer, input and audio, for the whole-port eval.
- [x] Eval `port-sample`: the skill fires, no SDL2 include remains, `SDL3/SDL.h` and `SDL3/SDL_main.h` are included, and the CMake file finds SDL3.
- [x] `Evals` workflow: a manual GitHub Actions job on Ubuntu that runs `claude plugin eval`.
- [x] Add a `CLAUDE_CODE_OAUTH_TOKEN` or `ANTHROPIC_API_KEY` repository secret (owner), then run `port-sample` 3 times.

**Done when:** the byte-identity check passes and fails on a tampered copy, and `port-sample` passes 3 of 3 runs.

## M2: Traps

- [ ] Trap cards in `references/<subsystem>.md`. Each has: what compiles, what breaks, how to find it, the fix and the source.
- [ ] Fixtures for each trap: `sdl2/` (builds against SDL 2.32), `naive/` and `fixed/`.
- [ ] The traps:

| Area | Trap | What breaks |
|---|---|---|
| Init | `bool-returns` | A leftover `SDL_Init(...) != 0` treats success as failure and exits on every launch |
| Input | `text-input-off` | Text input is no longer on by default, so no text events arrive |
| Input | `gamepad-index-vs-id` | `which` is now an instance ID, not an index, so the wrong pad or none opens |
| Render | `vsync-flag-dropped` | Dropping the removed `SDL_RENDERER_PRESENTVSYNC` flag leaves the loop unthrottled |
| Render | `linear-by-default` | The scale-quality hint is gone and textures default to linear, so pixel art blurs |
| Render | `rect-cast-to-frect` | Casting `SDL_Rect*` to `SDL_FRect*` draws garbage geometry |
| OpenGL | `gl-attributes-after-window` | GL attributes set after the window exists don't set its depth, stencil or MSAA |
| Audio | `audio-stream-paused` | A device opened with `SDL_OpenAudioDeviceStream` starts paused, so nothing plays |
| Audio | `mix-volume-float` | An SDL2 volume (0–128) passed as `SDL_MixAudio`'s 0–1 float turns 16-bit audio into wrapped-around noise |
| Hints | `hint-string-ignored` | Hints written as renamed or removed string literals are silently ignored |

Done: `bool-returns`, `audio-stream-paused`, `mix-volume-float`, `linear-by-default`.

- [x] Consistency check: every trap card has a fixture folder, and every fixture folder has a trap card.

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
