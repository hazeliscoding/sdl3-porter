---
type: regex
target: {source: file, path: "src/main.c"}
pattern: '\.color\.[rgba]\s*=\s*(left|right)\.[rgba]\s*;|SDL_FColor\s+\w+\s*=\s*\{\s*([2-9][0-9]|1[0-9][0-9]|2[0-4][0-9])\s*,|^(?=[\s\S]*SDL_Color\s+left\b)[\s\S]*\.color\s*=\s*left\s*;'
match: not_contains
---
