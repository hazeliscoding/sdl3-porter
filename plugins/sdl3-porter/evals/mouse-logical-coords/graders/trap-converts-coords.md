---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_ConvertEventToRenderCoordinates\s*\(|SDL_RenderCoordinatesFromWindow\s*\('
match: contains
---
