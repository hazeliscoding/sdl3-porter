---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_SetTextureBlendMode\s*\([^;]*SDL_BLENDMODE_NONE|SDL_PIXELFORMAT_XRGB8888|0x[fF]{2}000000|0x[fF]{2}[uU]?\s*<<\s*24|255[uU]?\s*<<\s*24'
match: contains
---
