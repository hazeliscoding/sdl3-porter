---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_PauseAudio(Stream)?Device\s*\('
match: not_contains
---
