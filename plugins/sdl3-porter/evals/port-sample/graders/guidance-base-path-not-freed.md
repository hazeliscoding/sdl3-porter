---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_free\s*\([^;]*\bbase\s*\)'
match: not_contains
---
