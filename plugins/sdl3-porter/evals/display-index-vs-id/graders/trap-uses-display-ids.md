---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_GetPrimaryDisplay\s*\(|SDL_GetDisplayForWindow\s*\(|\w+\s*\[\s*\w+\s*\]'
match: contains
---
