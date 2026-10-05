# Security

## What the skill does on your machine

sdl3-porter is instructions and reference files for a coding agent, plus SDL's own rename scripts and headers. When an agent uses it, the agent:

- edits your project's source and build files;
- runs SDL's three rename scripts, which ship with the skill, on the source paths it picks;
- runs your project's build and, where it can, the program it built.

The skill downloads nothing itself. Your build might: a CMake project that fetches SDL3 with `FetchContent` downloads it when it configures.

The files in `skills/sdl3-porter/scripts/sdl/` are unmodified copies from SDL's `release-3.4.16` tag, and CI checks them byte for byte against the release archive.

Give the agent the same permissions you'd give it for any other change to your code, and review its diff before you commit it.

## Reporting a vulnerability

Report it privately, through **Report a vulnerability** on the repository's Security tab, rather than in a public issue. Fixes go into the latest release.
