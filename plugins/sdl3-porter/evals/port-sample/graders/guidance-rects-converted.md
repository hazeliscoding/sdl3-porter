---
type: regex
target: {source: file, path: "src/main.c"}
pattern: '^(?![\s\S]*\(\s*(const\s+)?SDL_FRect\s*\*\s*\))[\s\S]*SDL_FRect'
match: contains
---
