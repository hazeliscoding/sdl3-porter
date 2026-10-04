---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_free\s*\([^;]*drop\.'
match: not_contains
---
