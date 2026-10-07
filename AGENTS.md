# AGENTS.md

These are the working rules for agents in this repo. sdl3-porter is an agent skill that ports C and C++ code from SDL2 to SDL3 and catches the changes that compile cleanly but break at runtime (zlib). It ships as a Claude Code plugin whose skill also works as a plain Agent Skill.

## Sources of truth

- `README.md`: the pitch and the "every trap is proven" promises.
- `ROADMAP.md`: decisions already made, the milestones, and what is out of scope. Check it before proposing features. Respect those decisions unless the owner reopens them. Record new or changed decisions there, with the date.
- Work from the next unchecked item in `ROADMAP.md`. Don't build past the current milestone without asking.
- SDL facts come from SDL itself: `docs/README-migration.md`, the headers and the wiki at the pinned tag. Never from memory, and never from SDL2-era tutorials.

## Commands

```sh
cmake -S . -B build                                   # fetches and builds SDL 3.4.18 and 2.32.10 the first time
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure   # originals and fixed ports pass, naive ports fail with their trap
cmake -S . -B build-sdl320 "-DSDL3_VERSION=3.2.0"       # the same fixtures against the oldest supported SDL3, as CI runs them
claude plugin validate .                              # the marketplace manifest
claude plugin validate plugins/sdl3-porter            # the plugin manifest
gh workflow run evals.yml -f case=port-sample -f runs=3 -f model=claude-sonnet-5   # evals run in GitHub Actions, never locally on Windows
```

Configuring needs Python 3, for the rename smoke test.

Keep commands cross-platform (`cmake`, `ctest`), because the owner develops on Windows. On Windows, the CMake bundled with Visual Studio works. Avoid bash-only scripts.

## Every trap is proven (hard rules)

The skill is only worth trusting if these hold.

- **No trap without fixtures.** A trap card needs `fixtures/<id>/sdl2/`, `naive/` and `fixed/`. The naive fixture is the positive control: it must fail in CI, and `cmake/run-naive.cmake` passes its test only on that failure. The SDL2 original must pass against SDL 2.32. A trap without a failing naive fixture isn't done.
- **Compiles cleanly, then breaks.** A trap's naive fixture must compile against SDL3 with warnings as errors (`-Wall -Wextra -Werror`, `/W4 /WX`). If it doesn't compile, or any compiler warns about it, the compiler already catches it, so it goes in a reference file, not a trap card. Never silence a warning in a naive fixture to make it qualify.
- **Cite the source.** Every trap card links the README-migration section or SDL wiki page it rests on. If you can't find one, don't add the trap.
- **Don't edit bundled SDL files.** Everything under `plugins/sdl3-porter/skills/sdl3-porter/scripts/sdl/` is byte-identical to the SDL tag named in its `SOURCE` file. To update, copy fresh files from a new tag, update `SOURCE` and record a decision.
- **Headless and offline.** Fixtures run without a display, audio hardware, gamepads or network: the offscreen video driver, dummy or disk audio, the virtual joystick API and the software renderer. A fixture that needs real hardware isn't a fixture.
- **The skill never calls a port done without building it.** When the project has tests or a runnable target, the skill runs them too.

## Skill and trap conventions

- The plugin lives in `plugins/sdl3-porter/`: its manifest, `skills/`, `evals/` and icon. Keep everything else outside it, including CI, docs, fixtures, samples and tools. Anthropic's plugin directory scans the whole plugin folder, and installs download it.

- `SKILL.md` holds the workflow and stays under 500 lines. Detail goes in `references/`, one file per subsystem, so the agent loads only what the project uses.
- A trap card has **What compiles**, **What breaks**, **How to find it**, **Fix** and **Source** sections.
- **How to find it** holds a code block with one Python regular expression per line. `scripts/find_traps.py` runs every section's block at once, and the `trap-searches` test checks that each trap card's search finds its own naive fixture.
- Trap ids are kebab-case and stable. Renaming one is a breaking change that needs a decision in `ROADMAP.md`.
- Fixtures are small C programs that test one behavior each. A failing fixture exits non-zero and prints one line that says what went wrong. Only the trap's own check starts that line with the trap id, for example `bool-returns: SDL_Init reported failure`. Setup failures start with `setup:`, so the naive runner, which requires the trap's line, can tell them apart. Only a crash may replace the trap's line.
- Pin versions: SDL 3.4.18 for ports, SDL 2.32.10 (the latest 2.32 release) for originals. Before each minor release, move the SDL3 pin to the latest SDL 3 release, as `ROADMAP.md` describes. Any other bump needs a decision there.
- The skill's own output is plain: one line per trap with `file:line`, the trap id and what changed, then what still needs a human. No emoji.

## Evals

- Evals use `claude plugin eval`. Don't build a separate eval framework.
- Installed as a plugin, the skill is `sdl3-porter:sdl3-porter`. Match it in `tool_used` graders with `(?:[\w-]+:)?sdl3-porter`.
- Evals run in the manual `Evals` GitHub Actions workflow. Claude Code refuses evals that grant Bash on Windows, because it can't sandbox the shell there.
- Prefer regex graders over files, one file per grader (`{source: file, path}` takes no globs). Use `llm` graders only for short output with a concrete pass/fail rubric.
- Case scaffolds are bash scripts that copy from `samples/` or `fixtures/` relative to their own location, then commit the copy, so the skill starts from a clean git tree.
- A trap's case ports `samples/<id>/`, a plausible SDL2 program with a neutral name and no self-check. Never port the fixture's SDL2 original: its checks point straight at the trap.
- Name trap graders `trap-*` and guidance graders `guidance-*`. Each must fail on a port that only ran SDL's rename scripts, and the `eval-graders` test checks that. Write patterns that JavaScript and Python's `re` read the same way, because the check runs them in Python.
- Never commit eval results, and never lower a threshold to make a case pass.

## Brand

- The logo is option A, "Chevron tile": `›3` in a rounded tile, beside the `sdl3-porter` wordmark. Keep it that simple. Don't add effects, gradients or a second accent.
- The assets are in `docs/brand/`: `mark.svg` and `lockup.svg` for light backgrounds, and `-dark` files for dark backgrounds. Use the SVGs, and don't re-typeset the wordmark with a web font.
- The type is JetBrains Mono ExtraBold, converted to vector paths, with -0.03em letter spacing on the wordmark and -0.04em on the tile.
- The accent is `#c2410c` on light and `#fb923c` on dark. The tile's glyphs are `#ffffff` on light and `#0d1117` on dark. Ink is `#1f2328` on light and `#e6edf3` on dark, matching GitHub's text colors.
- Write the name in lowercase, `sdl3-porter`, everywhere.

## Working style

- **Releases:** installed plugins only update when `version` in `plugins/sdl3-porter/.claude-plugin/plugin.json` changes. A release bumps it, tags `vX.Y.Z` and publishes a GitHub release. Don't bump it outside a release.
- **Commits:** [Conventional Commits](https://www.conventionalcommits.org/) (`feat:`, `fix:`, `docs:`, `chore:`, `test:`, `ci:`, `build:`, `refactor:`). Keep each commit atomic, and use a scope when it adds clarity (`feat(skill): …`, `test(fixtures): …`).
- **No AI attribution** in commits or PRs. That means no `Co-Authored-By` trailers, no "Generated with" lines and no session links.
- **`AGENTS.md`, `CLAUDE.md` and `.claude-plugin/` are committed.** `.gitignore` un-ignores them, overriding the global gitignore. Keep them free of secrets and private paths.
- **Checks:** automate acceptance checks instead of handing manual steps to the owner. Give every check that tests for an absence a positive control, meaning a case that proves the check can fail.
- **Validation:** evidence comes from the evals, the dogfooding log and public async signals (issues, PRs, installs). Don't plan interviews, recruiting or outreach.
- **Docs:** short and concise. Prefer editing `ROADMAP.md` over creating new planning documents. Repo files never reference the owner's private notes.
- **Code comments:** explain why, not what. In fixtures, one comment at the trap line saying what SDL3 changed is enough. Don't leave commented-out code.
