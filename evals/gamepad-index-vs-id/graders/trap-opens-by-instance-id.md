---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_OpenGamepad\s*\(\s*(\w+\s*\[\s*\w+\s*\]|(\w+_)?(id|ID)|[\w.>-]*which)\s*\)'
match: contains
---
