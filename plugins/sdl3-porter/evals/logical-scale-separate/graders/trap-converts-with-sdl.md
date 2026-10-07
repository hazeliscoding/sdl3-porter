---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_RenderCoordinatesFromWindow\s*\(|SDL_ConvertEventToRenderCoordinates\s*\(|SDL_GetRenderLogicalPresentationRect\s*\('
match: contains
---
