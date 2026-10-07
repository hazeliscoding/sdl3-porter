---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_Init\s*\([^;]*\)\s*(<|<=|>|>=|==|!=)\s*-?[0-9]'
match: not_contains
---
