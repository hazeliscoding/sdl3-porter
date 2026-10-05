---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_CreateSurfacePalette\s*\(|SDL_SetSurfacePalette\s*\('
match: contains
---
