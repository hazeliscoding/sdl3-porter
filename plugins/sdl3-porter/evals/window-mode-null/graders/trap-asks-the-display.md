---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_GetCurrentDisplayMode\s*\(|SDL_GetDesktopDisplayMode\s*\('
match: contains
---
