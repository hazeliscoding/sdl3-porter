---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_WriteIO\s*\([^;]*\)\s*!=\s*\(size_t\)\s*score_count'
match: not_contains
---
