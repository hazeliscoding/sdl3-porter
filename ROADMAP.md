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
- **`vsync-flag-dropped` checks the renderer's vsync setting, not timing (2026-10-04).** SDL2's software renderer reports vsync but doesn't wait on a window with a native framebuffer, like the dummy driver's, so a timed loop fails on the SDL2 original. The fixtures ask the renderer instead: `SDL_GetRendererInfo` in SDL2 and `SDL_GetRenderVSync` in SDL3. SDL3's software renderer does wait when vsync is on, so the fixed port's loop is paced either way.
- **`text-input-off` checks whether text input is on (2026-10-04).** The dummy and offscreen drivers produce no keyboard input, and SDL3 only gates real keyboard text, not pushed events, so no headless fixture can type. SDL3's keyboard code drops text and editing events unless `SDL_TextInputActive` is true for the focused window, so the fixtures ask that instead: `SDL_IsTextInputActive` in SDL2 and `SDL_TextInputActive` in SDL3.
- **`gl-attributes-after-window` is dropped (2026-10-04).** It isn't an SDL3 change. `SDL_GL_SetAttribute` is documented the same way in SDL 2.32 and 3.4 ("attributes should be set before creating an OpenGL window"), and the migration guide says nothing about when attributes apply, so an SDL2 original that sets them after the window would fail the same check. Without a source, it can't be a trap. M2 has no OpenGL trap.
- **`rect-cast-to-frect` is guidance, not a trap (2026-10-04).** SDL3's render functions take `SDL_FRect`, and the unchanged SDL2 call that passes an `SDL_Rect *` doesn't compile cleanly. MSVC warns (C4133), GCC and Clang warn about incompatible pointer types, GCC 14 and later reject it, and so does C++. Only a cast gets it through, and a naive fixture may not silence a warning, as with `drop-data-freed`. `SKILL.md` step 5 already says to convert the rectangle instead of casting it.

### Added during M3 (2026-10-04)

- **Trap cases port realistic samples, not the fixture originals.** A fixture's SDL2 original exists to test its trap, and its checks and messages point straight at it, so porting one measures how well a model ports a test, with the skill or without it. Each trap's case scaffolds `samples/<id>/` instead: a small, plausible SDL2 program with the trap in it, a neutral name and no self-check. CI builds every sample against SDL 2.32 with warnings as errors, bounce included.
- **`port-sample` is the whole-program case.** Bounce already holds seven of the eight traps, all but `gamepad-index-vs-id`, and three guidance items. Its event-driven gamepad code ports correctly. The new graders are named `trap-<id>` and `guidance-<item>`, and the include and CMake graders from M1 stay.
- **Trap graders must be able to fail.** Every `trap-*` and `guidance-*` grader must fail on a port that only ran SDL's rename scripts, which still has every trap. The `eval-graders` test checks it, with a control for a grader that passes anyway.
- **Case scores leave out `skill-fired`.** In a with-without run, `claude plugin eval` reports the `tool_used: Skill` grader separately instead of scoring it in either arm, so a case's score measures only the port.
- **The smoke run passed, and showed a slow header search (2026-10-04).** `text-input-off` scored 1.0 with the skill and 1.0 without it, for $0.69, with Sonnet 5 and Claude Code 2.1.289. The run with the skill took 13 minutes for 33 turns. A traced rerun took 3.5 minutes, 87 seconds of it in a `find /` for SDL3 headers that the sandbox doesn't have. `SKILL.md` now says to find headers through the build and to fall back on the migration guide. Trap cases now get `port-sample`'s 20 minutes, and the `Evals` job 3 hours.
- **The skill bundles SDL 3.4.16's public headers.** The first full eval run scored every case 1.00 with the skill except `bool-returns`, at 0.67: one of its runs timed out after 1200 seconds. A traced rerun showed the agent still searching the whole disk for SDL3 headers, despite the new line in `SKILL.md`, and one search outlasted the Bash tool's 2-minute limit. So the agent now gets exact signatures from the skill: 66 headers in `scripts/sdl/include/SDL3/`, 2.3 MB, leaving out the test library and the Khronos OpenGL and EGL headers. The byte-identity check covers them. `SKILL.md` says to check each function's `\since` against the oldest SDL3 the project supports. Every case runs again so the README table reflects the skill as shipped.
- **M3 results (2026-10-05).** With the headers bundled, every case scored 1.00 with the skill over 3 runs, and the skill fired in all 27 runs, with Sonnet 5 and Claude Code 2.1.289, for $24.36. Without the skill, Sonnet 5 scored 0.00 on `linear-by-default`, 0.78 on `hint-string-ignored` and 0.76 on the whole program. In the whole program it missed `text-input-off` and `linear-by-default` in every run, although it fixed `text-input-off` in that trap's own sample. The other six cases scored 1.00 either way. The slowest run with the skill took 342 seconds, down from 1200. M3's runs cost $50.80 in all, counting the smoke and traced runs and the first full run. The README table comes from the nine dispatches' `--json` results, which use the `aggregate-result.json` schema.

### Added during M4 (2026-10-05)

- **jage was ported on a local scratch branch.** jage's own roadmap moves it to SDL3 in its M1, together with CMake, and its last commit doesn't build without workarounds yet. So a fresh headless Claude Code session with the plugin ported jage on a local `sdl3-dogfood` branch, without building it, and the port was then compiled against SDL3's headers without linking. jage's `main` and milestones are unchanged, and the branch isn't pushed. `docs/dogfooding.md` has what the skill caught and missed.
- **The public programs are gbemu and AKrikler/chip8, in local clones.** Each started from the commit before its maintainer's own SDL3 port, which served as the answer key. pengupop was dropped, because its original is SDL 1.2. Only MSVC was available, so chip8 was built and run, and gbemu, whose build needs GCC or Clang, was compiled without linking. Nothing was forked or pushed.
- **`blend-by-default` joins the traps.** The skill's gbemu port missed it, and the maintainer's own port fixed it: SDL3 blends textures with an alpha format by default, so a frame buffer of `0x00RRGGBB` pixels draws transparent. Its fixtures prove it like the other traps. Its eval case waits for the full eval run before the next release, which also measures the `SKILL.md` changes made in M4.
- **Every dogfood report counted changes without a trap id as traps.** jage's, gbemu's and chip8's reports all did, the last two after a first rule against it. `SKILL.md` step 8 now requires a trap id on every trap line.
- **Evals still don't build the port.** This answers M1's note that M3 would revisit it. A trap compiles cleanly by definition, so a build wouldn't change any trap score, and graders can't see whether a build succeeded. `SKILL.md` step 5's syntax check against the bundled headers already catches compile errors when SDL3 isn't installed. Installing SDL3 on the eval runner moves to Later.

### Added during M5 (2026-10-05)

- **The pre-release eval run passed.** All 10 cases scored 1.00 with the skill over 3 runs, and the skill fired in all 30 runs, with Sonnet 5 and Claude Code 2.1.289, for $26.58. `blend-by-default` scored 0.00 without the skill. The median run with the skill took 202 seconds, but three took 746, 808 and 1130 seconds with ordinary turn counts, so something in them waited, close to the 1200-second timeout. Those runs kept no transcripts, so the cause is still open.
- **The quick start works from an empty Claude Code config.** With `CLAUDE_CONFIG_DIR` pointing at a new folder, the README's two commands added the marketplace and installed and enabled the plugin at the latest commit. That checks the commands, not a clean machine, so the quick start item stays open for one.
- **The slow runs were searches for an installed SDL3.** A traced rerun of the three slow cases, 9 runs with the skill for $5.15, scored 1.00 every time. Three of the runs spent 88 to 118 seconds in a `find` for SDL3's library or its CMake and `sdl3-config` files, to decide whether they could build, and one searched only `/usr` and still took 96 seconds. The rule against searching for headers didn't cover that. `SKILL.md` now says never to look for SDL outside the project with `find` or `locate`, and to ask the build whether SDL3 is installed: configure the project, or run `pkg-config --exists sdl3`.
- **The broader rule holds.** The same 9 runs against the new `SKILL.md`, for $5.11, all scored 1.00, and none ran `find` or `locate` outside the project. No command took longer than 15 seconds, and the slowest run took 202 seconds. Every run compiled its port against the bundled headers with `cc -fsyntax-only`, and most checked for SDL3 with `pkg-config`.
- **The first clean-machine attempt installed into the Windows setup instead.** On a fresh WSL Ubuntu 24.04, the README's two commands added the marketplace and installed the plugin, at commit `73efb77`. But no Linux `claude` existed yet, and WSL appends the Windows `PATH`, so the `claude` in that terminal was the owner's Windows install, with its own config and plugins. The commands work in a real setup, then, but not yet in a clean one, so the quick start item stays open until a Linux-native Claude Code installs the plugin from a new terminal. Ubuntu 24.04 doesn't package SDL3, so the test distro builds SDL 3.4.16 from source. Under WSLg, the SDL2 bounce sample drew 300 frames in 4,835 ms, so vsync works there.
- **The clean-machine check passed.** On the same fresh WSL Ubuntu 24.04, a Linux-native Claude Code 2.1.289, logged in for the first time, installed the plugin with the README's two commands, at `55d487a`. A fresh session on the owner's default model, Opus 5.5, ported the bounce sample in 16 turns for $0.69, and its report listed all 7 of the sample's traps by id. Another ported AKrikler/chip8 in 25 turns for $1.13 and listed all 4 of its traps. Both ports built against SDL 3.4.16 without warnings and ran on WSLg. The bounce port drew 300 frames in 5,097 ms, against 4,835 ms for the SDL2 original, so vsync still paces it. The owner saw a crisp sprite and their typing in the title, heard clean beeps, and played chip8's Pong with sharp pixels.
- **Versioning starts at 0.1.0.** Claude Code compares a plugin's `version` to decide whether an install has an update, so from now on users only get changes when `plugin.json`'s version goes up. Every release bumps it, tags `vX.Y.Z` and publishes a GitHub release, and changes between releases wait on `main` until the next one.

### Road to 1.0 (2026-10-05)

- **1.0 means porting, done well.** It's a complete, stable SDL2-to-SDL3 porter with evidence behind it. Writing new SDL3 correctly (main callbacks, properties, the GPU API) is the 2.x track: it needs evals that judge new code rather than catch known traps, and it can reuse every trap card as a list of things not to do.
- **What 1.0 promises.** The plugin and skill names, the trap ids, the report format and the oldest supported SDL3 stay fixed across 1.x. The coverage claim lists its known gaps. The supported SDL range is tested in CI. The evidence covers more than one model and ports larger than 1,000 lines.
- **A sweep of the migration guide found the rest of the core traps.** It turned up 7 strong candidates that compile cleanly, can be proven headless and look common, 6 provable but niche ones, and about 9 that can only be guidance. It also found that the guide's YUV default (`SDL_COLORSPACE_JPEG`) contradicts the 3.4.16 header (`SDL_COLORSPACE_BT601_LIMITED`, which matches SDL2), so nothing changes in practice.
- **Surfaces and `SDL_IOStream` join the core scope.** Two of the strong candidates live there, and both are core SDL3.
- **Each milestone on the way ends in a minor release** (0.2.0 after M6, and so on), so users get new traps without waiting for 1.0.

### Added during M7 (2026-10-05)

- **CI and evals run on `ubuntu-24.04`, not `ubuntu-latest`.** GitHub moves `ubuntu-latest` to Ubuntu 26 from 2026-10-19. The pin keeps the image that the apt packages and the eval sandbox setup, including the AppArmor change bubblewrap needs, were written for. Moving to Ubuntu 26 is a separate change, with its own check that the packages still install.

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

- [x] Trap cards in `references/<subsystem>.md`. Each has: what compiles, what breaks, how to find it, the fix and the source.
- [x] Fixtures for each trap: `sdl2/` (builds against SDL 2.32), `naive/` and `fixed/`.
- [x] The traps:

| Area | Trap | What breaks |
|---|---|---|
| Init | `bool-returns` | A leftover `SDL_Init(...) != 0` treats success as failure and exits on every launch |
| Input | `text-input-off` | Text input is no longer on by default, so no text events arrive |
| Input | `gamepad-index-vs-id` | Functions that took a device index now take an instance ID, so an index loop opens the wrong pad or none |
| Render | `vsync-flag-dropped` | Dropping the removed `SDL_RENDERER_PRESENTVSYNC` flag leaves the loop unthrottled |
| Render | `linear-by-default` | The scale-quality hint is gone and textures default to linear, so pixel art blurs |
| Render | `blend-by-default` | Textures with an alpha format now blend, so pixels written with alpha 0 vanish (added in M4) |
| Audio | `audio-stream-paused` | A device opened with `SDL_OpenAudioDeviceStream` starts paused, so nothing plays |
| Audio | `mix-volume-float` | An SDL2 volume (0–128) passed as `SDL_MixAudio`'s 0–1 float turns 16-bit audio into wrapped-around noise |
| Hints | `hint-string-ignored` | Hints written as renamed or removed string literals are silently ignored |

- [x] Consistency check: every trap card has a fixture folder, and every fixture folder has a trap card.

**Done when:** every naive fixture fails and every fixed fixture passes, the consistency check passes and fails on a card without fixtures, and any trap that couldn't be reproduced headless has moved to guidance with a decision recorded here.

## M3: Evals

- [x] One eval case per trap: scaffold its sample from `samples/<id>/`, ask for a port, and grade that the skill fired, the naive pattern is gone and the fix is present.
- [x] A whole-program case: the sample with several traps at once.
- [x] Run every case 3 times with and without the skill in the `Evals` workflow.
- [x] A README table of with-skill and without-skill scores per trap, with the model and the date.

**Done when:** every case scores at least 0.9 with the skill across 3 runs, and the README table is generated from `aggregate-result.json`.

## M4: Dogfood

- [x] Port jage to SDL3 with the skill.
- [x] Port 2 public SDL2 programs in scratch forks. No upstream PRs.
- [x] `docs/dogfooding.md`: what it caught and what it missed. Every miss becomes a trap, a reference fix or a Later item.

**Done when:** the log has at least 3 entries and every miss has been turned into one of those three.

## M5: v0.1.0

- [x] Run every eval case 3 times with and without the skill, and regenerate the README table.
- [x] README quick start: install with `/plugin marketplace add hazeliscoding/sdl3-porter`, checked on a clean machine.
- [x] Document what's supported: SDL 3.2 and later, C and C++, Claude Code. Other harnesses are untested.
- [x] `CONTRIBUTING.md`: how to add a trap as a card, three fixtures and an eval case.
- [x] `SECURITY.md`: the skill edits your code and runs the bundled Python scripts and your build. It downloads nothing at runtime.
- [x] A "missed trap" issue template that asks for the SDL version, a minimal SDL2 snippet and what went wrong.
- [x] Set `plugin.json` to 0.1.0, tag it and publish a GitHub release.

**Done when:** on a clean machine, someone can install from the README, port the sample and build and run it, CI is green, and there are no known critical bugs.

## M6: Coverage

- [ ] Widen the scope to surfaces and `SDL_IOStream` in the README, `SKILL.md` and this file.
- [ ] Add the 7 strong traps, each with a card, fixtures, a sample and an eval case:

| Area | Trap | What breaks |
|---|---|---|
| Input | `mouse-logical-coords` | Mouse events are no longer converted to the logical size, so clicks land in the wrong place |
| Video | `display-index-vs-id` | Display functions take an ID, not an index, so display 0 is invalid |
| I/O | `rwread-count-vs-bytes` | `SDL_ReadIO` returns bytes, not a count of items, so a `!= 1` check fails every read |
| Surfaces | `indexed-surface-no-palette` | Indexed surfaces no longer get a palette, so palette calls and blits fail |
| Audio | `audio-init-implicit` | Opening audio no longer initializes the audio subsystem, so the device doesn't open |
| Render | `vertex-color-float` | `SDL_Vertex` colors are floats from 0 to 1, so 0–255 values draw white |
| Video | `window-mode-null` | `SDL_GetWindowFullscreenMode` returns NULL for a windowed window, which crashes on use |

- [ ] Write guidance for the changes no fixture can prove: Nintendo face buttons, exclusive fullscreen, batching with direct OpenGL, mouse wheel values, high DPI, asynchronous window operations, Apple bundle paths, gamepad rumble and `SDL_RegisterEvents`.
- [ ] Keep the 6 niche candidates as candidates: `event-timestamp-ns`, `target-state-persists`, `blit-dstrect-unchanged`, `render-output-size-logical`, `logical-scale-separate` and `resized-on-set-size`. Each becomes a trap only when a port or an issue shows it in real code.
- [ ] Release 0.2.0.

**Done when:** every strong candidate is either a trap whose fixtures pass in CI or guidance with a decision recorded here, and 0.2.0 is released.

## M7: Support and stability

- [ ] Run the fixtures in CI against SDL 3.2.0 as well as the pinned 3.4 release, because the README promises 3.2 and later. A trap that behaves differently on 3.2 says so on its card.
- [x] Pin the CI and `Evals` runners before GitHub moves `ubuntu-latest` to Ubuntu 26 on 2026-10-19, or check that their packages still install there.
- [ ] Write the stability promise in the README: what stays fixed across 1.x and what only a major version may change.
- [ ] Write down how the pinned SDL moves: to the latest 3.x release before each minor release, with the fixtures and evals run again.
- [ ] Release 0.3.0.

**Done when:** CI is green against both SDL versions, the stability promise is in the README, and 0.3.0 is released.

## M8: Evidence

- [ ] Run every eval case 5 times with and without the skill, on Sonnet 5 and on Haiku 4.5, and show both models in the README table.
- [ ] Port two public SDL2 projects of 10,000 lines or more in local clones, against their maintainers' own SDL3 ports where they exist, and log them in `docs/dogfooding.md`.
- [ ] Turn every miss into a trap, a reference fix or a Later item, as in M4.
- [ ] Release 0.4.0.

**Done when:** every case scores at least 0.9 with the skill on both models, the log has the two larger ports with every miss resolved, and 0.4.0 is released.

## M9: v1.0.0

- [ ] List the plugin in plugin directories.
- [ ] Publish the write-up: how the skill was built and tested, the eval results with and without it, the header search that hung the evals, and how its ports compare with maintainers' own SDL3 migrations.
- [ ] Set `plugin.json` to 1.0.0, tag it and publish a GitHub release.

**Done when:** 1.0.0 is released, CI is green, and every open "missed trap" issue has a decision.

## Later

- A deterministic trap checker built from the trap cards' patterns, runnable in CI without a model, and a hook that runs it on every file the agent edits.
- SDL_image 3, SDL_ttf 3 and SDL_mixer 3.
- Writing new SDL3: main callbacks, properties and the GPU API. This is the 2.x track.
- A visual high-DPI check, beyond M6's guidance.
- Evals for Codex and other harnesses.
- A catalog marketplace repo that lists this and future skills.
- Build the port inside evals: install SDL3 on the eval runner so the agent can build its port and run it headless.

## Not planned

- SDL 1.2 to SDL3.
- Language bindings (SDL3-CS, Rust, Zig).
- Engines built on SDL.
- Keeping SDL2 code working. sdl2-compat does that.
- A hosted service, accounts or telemetry.

## How we'll know it works

Evidence comes from the eval scores with and without the skill, and from the dogfooding log (n=1, plus scratch ports). After release it also comes from public signals: installs, "missed trap" issues, PRs that add traps and SDL projects that link to it.
