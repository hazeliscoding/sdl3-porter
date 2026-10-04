---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_Set(Default)?TextureScaleMode\s*\([^;]*SDL_SCALEMODE_(NEAREST|PIXELART)'
match: contains
---
